#include "uv_ssl.h"

CLASS_ENTRY_FUNCTION_D(UVSSL){
    EXTENDS_CLASS_WITH_OBJECT_NEW(UVSSL, createUVSSLResource, UVTcp);
    OBJECT_HANDLER(UVSSL).offset = XtOffsetOf(uv_tcp_ext_t, zo) + XtOffsetOf(uv_ssl_ext_t, uv_tcp_ext);
    OBJECT_HANDLER(UVSSL).clone_obj = NULL;
    OBJECT_HANDLER(UVSSL).free_obj = freeUVSSLResource;
    REGISTER_CLASS_CONSTANT_LONG(UVSSL, SSL_METHOD_SSLV2);
    REGISTER_CLASS_CONSTANT_LONG(UVSSL, SSL_METHOD_SSLV3);
    REGISTER_CLASS_CONSTANT_LONG(UVSSL, SSL_METHOD_SSLV23);
    REGISTER_CLASS_CONSTANT_LONG(UVSSL, SSL_METHOD_TLSV1);
    REGISTER_CLASS_CONSTANT_LONG(UVSSL, SSL_METHOD_TLSV1_1);
    REGISTER_CLASS_CONSTANT_LONG(UVSSL, SSL_METHOD_TLSV1_2);

    REGISTER_UV_SSL_X509_CONSTANT();

    SSL_library_init();
}

zend_always_inline int handleHandshakeCallback(uv_ssl_ext_t *resource, int err) {
    zval z_retval;
    zval params[2];
    int retval;
    params[0] = resource->uv_tcp_ext.object;
    ZVAL_FALSE(&z_retval);
    
    if(!Z_ISNULL(resource->sslHandshakeCallback.func)){
        ZVAL_LONG(&params[1], err);
        fci_call_function(&resource->sslHandshakeCallback, &z_retval, 2, params);
        zval_dtor(&params[1]);
    }
    
    retval = zend_is_true(&z_retval);
    zval_dtor(&z_retval);

    if(retval == 0){
        tcp_close_socket(&resource->uv_tcp_ext);
    }
    return retval;
}

static int verify_callback(int preverify_ok, X509_STORE_CTX *ctx) {
    zval rv;
    X509_STORE_CTX_get_current_cert(ctx);
    int err = X509_STORE_CTX_get_error(ctx);
    int depth = X509_STORE_CTX_get_error_depth(ctx);
    SSL *ssl;
    uv_ssl_ext_t *resource;
    ssl = X509_STORE_CTX_get_ex_data(ctx, SSL_get_ex_data_X509_STORE_CTX_idx());
    resource = SSL_get_ex_data(ssl, 0);

    if(err == X509_V_OK){
        return 1;
    }

    if(depth > OPENSSL_DEFAULT_STREAM_VERIFY_DEPTH){
        err = X509_V_ERR_CERT_CHAIN_TOO_LONG;
    }
    return handleHandshakeCallback(resource, err);
}

static void alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
    buf->base = emalloc(suggested_size);
    buf->len = suggested_size;
}

zend_always_inline int handleHandshake(uv_ssl_ext_t *resource, ssize_t nread, const uv_buf_t* buf) {
    int ret, err;
    zval rv;
    X509 *peer_cert;
    
    if(SSL_is_init_finished(resource->ssl)) {
        resource->uv_tcp_ext.flag |= UV_TCP_WRITE_CALLBACK_ENABLE;
        return 1;
    }

    ret = SSL_do_handshake(resource->ssl);
    write_bio_to_socket(resource);
    
    if(ret != 1){
        err = SSL_get_error(resource->ssl, ret);
        switch(err){
            case SSL_ERROR_WANT_READ:
                return 0;
            case SSL_ERROR_WANT_WRITE:
                write_bio_to_socket(resource);
                return 0;
            default:
                tcp_close_socket(&resource->uv_tcp_ext);
                return 0;
        }
    }

    if(resource->clientMode){
        if(peer_cert = SSL_get_peer_certificate(resource->ssl)) {
            if(!matches_common_name(peer_cert, resource->sniConnectHostname) && !matches_san_list(peer_cert, resource->sniConnectHostname)) {
                if(!handleHandshakeCallback(resource, X509_V_ERR_SUBJECT_ISSUER_MISMATCH)) {
                    X509_free(peer_cert);
                    return 0;
                }
            }
            X509_free(peer_cert);
        }
    }
    return handleHandshakeCallback(resource, X509_V_OK);
}

static void read_cb(uv_tcp_ext_t *tcp_resource, ssize_t nread, const uv_buf_t* buf) {
    zval params[2];
    uv_ssl_ext_t *resource = FETCH_RESOURCE_FROM_EXTEND(tcp_resource, uv_tcp_ext, uv_ssl_ext_t);
    zval retval;
    char read_buf[256];
    int size, ret, read_buf_index = 0;
    params[0] = resource->uv_tcp_ext.object;
    
    if(nread<0){
        if(!Z_ISNULL(tcp_resource->errorCallback.func)){
            ZVAL_LONG(&params[1], nread);
            fci_call_function(&tcp_resource->errorCallback, &retval, 2, params);
            zval_dtor(&params[1]);
            zval_dtor(&retval);
        }
        tcp_close_socket(&resource->uv_tcp_ext);
        efree(buf->base);
        return;
    }

    BIO_write(resource->read_bio, buf->base, nread);

    if(!handleHandshake(resource, nread, buf)){
        efree(buf->base);
        return;
    }

    while(1){
        size = SSL_read(resource->ssl, &read_buf[read_buf_index], sizeof(read_buf) - read_buf_index);
        if(size > 0){
            read_buf_index+=size;
        }
        if(size <= 0 || read_buf_index >= sizeof(read_buf)){
            if(!Z_ISNULL(tcp_resource->readCallback.func)){
                ZVAL_STRINGL(&params[1], read_buf, read_buf_index);
                fci_call_function(&tcp_resource->readCallback, &retval, 2, params);
                zval_dtor(&params[1]);
                zval_dtor(&retval);
            }
            read_buf_index = 0;
        }
        if(size <= 0){
            break;
        }
    }
    efree(buf->base);
}

static int sni_cb(SSL *s, int *ad, void *arg) {
    long n;
    zval param;
    zval retval;
    uv_ssl_ext_t *resource = (uv_ssl_ext_t *) arg;
    uv_tcp_ext_t *tcp_resource = &resource->uv_tcp_ext;
    const char *servername = SSL_get_servername(s, TLSEXT_NAMETYPE_host_name);
    if(servername != NULL && !Z_ISNULL(resource->sslServerNameCallback.func)){
        ZVAL_STRING(&param, servername);
        fci_call_function(&resource->sslServerNameCallback, &retval, 1, &param);
        if(Z_TYPE_P(&retval) == IS_LONG){
            n = Z_LVAL_P(&retval);
            if(n>=0 && n<resource->nctx){
                SSL_set_SSL_CTX(s, resource->ctx[n]);
            }
        }
        zval_dtor(&param);
        zval_dtor(&retval);
    }
    return SSL_TLSEXT_ERR_OK;
}

static void client_connection_cb(uv_connect_t* req, int status) {
    uv_tcp_ext_t *tcp_resource = (uv_tcp_ext_t *) req->handle; 
    uv_ssl_ext_t *resource = FETCH_RESOURCE_FROM_EXTEND(tcp_resource, uv_tcp_ext, uv_ssl_ext_t);
    zval params[2];
    zval retval;
    params[0] = tcp_resource->object;
    ZVAL_LONG(&params[1], status);

    SSL_CTX_set_verify(resource->ctx[0], SSL_VERIFY_PEER, verify_callback);
    SSL_CTX_set_default_verify_paths(resource->ctx[0]);
    SSL_CTX_set_cipher_list(resource->ctx[0], "DEFAULT");
    
    if(uv_read_start((uv_stream_t *) tcp_resource, alloc_cb, (uv_read_cb) read_cb)){
        return;
    }
    
    resource->ssl = SSL_new(resource->ctx[0]);
    SSL_set_ex_data(resource->ssl, 0, resource);
    resource->read_bio = BIO_new(BIO_s_mem());
    resource->write_bio = BIO_new(BIO_s_mem());
    SSL_set_bio(resource->ssl, resource->read_bio, resource->write_bio);

#ifndef OPENSSL_NO_TLSEXT
    SSL_set_tlsext_host_name(resource->ssl, resource->sniConnectHostname);
#endif

    SSL_set_connect_state(resource->ssl);
    SSL_connect(resource->ssl);

    write_bio_to_socket(resource);    
    tcp_resource->flag |= (UV_TCP_HANDLE_START|UV_TCP_READ_START);
    fci_call_function(&tcp_resource->connectCallback, &retval, 2, params);
    zval_dtor(&params[1]);
    zval_dtor(&retval);
}

static void on_addrinfo_resolved(uv_getaddrinfo_ext_t *addrinfo, int status, struct addrinfo *res) {
    int ret;
    struct sockaddr_in addr;
    zval params[2], rv;
    char host[17] = {'\0'};
    uv_ssl_ext_t *resource = addrinfo->ssl_handle;
    uv_tcp_ext_t *tcp_resource = &resource->uv_tcp_ext;
    params[0] = tcp_resource->object;

    if( (ret = status) != 0 ||
        (ret = uv_ip4_name((struct sockaddr_in*) res->ai_addr, host, 16)) != 0 ||
        (ret = uv_ip4_addr(host, addrinfo->port&0xffff, &addr)) != 0 ||
        (ret = uv_tcp_connect(&tcp_resource->connect_req, &tcp_resource->uv_tcp, (const struct sockaddr *) &addr, client_connection_cb)) != 0) {

        ZVAL_LONG(&params[1], ret);
        fci_call_function(&tcp_resource->connectCallback, &rv, 2, params);
        zval_dtor(&params[1]);
        zval_dtor(&rv);
        releaseResource(tcp_resource);
    }

    uv_freeaddrinfo(res);
    efree(addrinfo);
}

static zend_object *createUVSSLResource(zend_class_entry *ce) {
    uv_ssl_ext_t *ssl_resource;
    uv_tcp_ext_t *tcp_resource;

    ssl_resource = ALLOC_RESOURCE(uv_ssl_ext_t);
    tcp_resource = &ssl_resource->uv_tcp_ext;
    zend_object_std_init(&tcp_resource->zo, ce);
    object_properties_init(&tcp_resource->zo, ce);
    tcp_resource->zo.handlers = &OBJECT_HANDLER(UVSSL);
    initUVTcpFunctionCache(tcp_resource);
    initUVSSLFunctionCache(ssl_resource);
    return &tcp_resource->zo;
}

void freeUVSSLResource(zend_object *object) {
    int i;
    uv_ssl_ext_t *resource = FETCH_SSL_RESOURCE(object);
    if(resource->ctx){
        if(resource->ssl){
            SSL_free(resource->ssl);
            resource->ssl = NULL;
        }
        for(i=0; i<resource->nctx;i++){
            SSL_CTX_free(resource->ctx[i]);
        }
        efree(resource->ctx);
        resource->ctx = NULL;
    }
    if(resource->sniConnectHostname){
        efree(resource->sniConnectHostname);
    }
    releaseResource(&resource->uv_tcp_ext);
    releaseUVTcpFunctionCache(&resource->uv_tcp_ext);
    releaseUVSSLFunctionCache(resource);
    zend_object_std_dtor(object);
    efree(resource);
}

PHP_METHOD(UVSSL, setSSLServerNameCallback){
    zval *sslServerNameCallback;
    zval *self = getThis();
    uv_ssl_ext_t *resource = FETCH_OBJECT_SSL_RESOURCE(self);

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "z", &sslServerNameCallback)){
        return;
    }
    
    registerFunctionCache(&resource->sslServerNameCallback, sslServerNameCallback);
}

PHP_METHOD(UVSSL, setSSLHandshakeCallback){
    zval *sslHandshakeCallback;
    zval *self = getThis();
    uv_ssl_ext_t *resource = FETCH_OBJECT_SSL_RESOURCE(self);

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "z", &sslHandshakeCallback)){
        return;
    }
    registerFunctionCache(&resource->sslHandshakeCallback, sslHandshakeCallback);
}

PHP_METHOD(UVSSL, setPrivateKey){
    const char *key = NULL;
    size_t key_len;
    zval *self = getThis();
    long n = 0;
    EVP_PKEY *pkey;
    BIO *key_bio;
    int result;
    uv_ssl_ext_t *resource = FETCH_OBJECT_SSL_RESOURCE(self);

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "s|l", &key, &key_len, &n)){
        return;
    }

    if(n<0 || n>=resource->nctx){
        RETURN_FALSE;
    }
    
    key_bio = BIO_new(BIO_s_mem());
    if(BIO_write(key_bio, key, key_len) <= 0){
        RETURN_FALSE;
    }

    pkey = PEM_read_bio_PrivateKey(key_bio, NULL, NULL, NULL);
    BIO_free(key_bio);
                                                        
    if(pkey == NULL){
        RETURN_FALSE;
    }
    
    result = SSL_CTX_use_PrivateKey(resource->ctx[n], pkey);
    EVP_PKEY_free(pkey);
    RETURN_BOOL(result);
}

PHP_METHOD(UVSSL, setCert){
    const char *cert = NULL;
    size_t cert_len;
    zval *self = getThis();
    long n = 0;
    X509 *pcert;  
    BIO *cert_bio;
    int result;
    uv_ssl_ext_t *resource = FETCH_OBJECT_SSL_RESOURCE(self);

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "s|l", &cert, &cert_len, &n)){
        return;
    }

    if(n<0 || n>=resource->nctx){
        RETURN_FALSE;
    }

    cert_bio = BIO_new(BIO_s_mem());
    if(BIO_write(cert_bio, cert, cert_len) <= 0){
        RETURN_FALSE;
    }
    
    pcert = PEM_read_bio_X509_AUX(cert_bio, NULL, NULL, NULL);
    BIO_free(cert_bio);
    result = SSL_CTX_use_certificate(resource->ctx[n], pcert);    
    X509_free(pcert);
    RETURN_BOOL(result);
}

PHP_METHOD(UVSSL, accept){
    zval *self = getThis();
    uv_ssl_ext_t *server_resource = FETCH_OBJECT_SSL_RESOURCE(self);
    uv_ssl_ext_t *client_resource;
    
    object_init_ex(return_value, CLASS_ENTRY(UVSSL));
    if(!make_accepted_uv_tcp_object(&server_resource->uv_tcp_ext, return_value)){
        RETURN_FALSE;
    }
    
    client_resource = FETCH_OBJECT_SSL_RESOURCE(return_value);
    if(uv_read_start((uv_stream_t *) &client_resource->uv_tcp_ext, alloc_cb, (uv_read_cb) read_cb)){
        RETURN_FALSE;
    }
    client_resource->ssl = SSL_new(server_resource->ctx[0]);
    client_resource->read_bio = BIO_new(BIO_s_mem());
    client_resource->write_bio = BIO_new(BIO_s_mem());
    SSL_set_bio(client_resource->ssl, client_resource->read_bio, client_resource->write_bio);
    SSL_set_accept_state(client_resource->ssl);
    write_bio_to_socket(client_resource);
}

PHP_METHOD(UVSSL, __construct){
    long sslMethod = SSL_METHOD_TLSV1_1, nContexts = 1;
    int i;
    zval *loop = NULL;
    zval *self = getThis();
    uv_ssl_ext_t *resource = FETCH_OBJECT_SSL_RESOURCE(self);
    
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "|zll", &loop, &sslMethod, &nContexts)) {
        return;
    }

    resource->uv_tcp_ext.object = *self;

    if(NULL == loop || Z_ISNULL_P(loop)){
        uv_tcp_init(uv_default_loop(), (uv_tcp_t *) &resource->uv_tcp_ext);
    }
    else{
        zend_update_property(CLASS_ENTRY(UVTcp), self, ZEND_STRL("loop"), loop);
        uv_tcp_init(FETCH_UV_LOOP(), (uv_tcp_t *) &resource->uv_tcp_ext);
    }

    switch(sslMethod){
        case SSL_METHOD_SSLV23:
            resource->ssl_method = SSLv23_method();
            break;
        case SSL_METHOD_SSLV2:
#ifndef OPENSSL_NO_SSL2
            resource->ssl_method = SSLv2_method();
#else
    #ifndef OPENSSL_NO_SSL3_METHOD
            resource->ssl_method = SSLv3_method();
    #endif
#endif
            break;
        case SSL_METHOD_SSLV3:
#ifndef OPENSSL_NO_SSL3_METHOD
            resource->ssl_method = SSLv3_method();
            break;
#endif
        case SSL_METHOD_TLSV1:
            resource->ssl_method = TLSv1_method();
            break;
        case SSL_METHOD_TLSV1_1:
            resource->ssl_method = TLSv1_1_method();
            break;
        case SSL_METHOD_TLSV1_2:
            resource->ssl_method = TLSv1_2_method();
            break;
        default:
            resource->ssl_method = TLSv1_1_method();
            break;
    }
    resource->ctx = emalloc(sizeof(SSL_CTX *) * nContexts);
    resource->nctx = nContexts;

    for(i = 0; i < nContexts; i++){
        resource->ctx[i] = SSL_CTX_new(resource->ssl_method);
        SSL_CTX_set_session_cache_mode(resource->ctx[i], SSL_SESS_CACHE_BOTH);
    }
    
#ifndef OPENSSL_NO_TLSEXT
    if(sslMethod >= SSL_METHOD_TLSV1){
        SSL_CTX_set_tlsext_servername_callback(resource->ctx[0], sni_cb);
        SSL_CTX_set_tlsext_servername_arg(resource->ctx[0], resource);
    }
#endif
}

PHP_METHOD(UVSSL, write){
    zval *self = getThis();
    const char *message = NULL;
    size_t message_len, size;
    uv_ssl_ext_t *resource = FETCH_OBJECT_SSL_RESOURCE(self);
   
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "s", &message, &message_len)) {
        return;
    }
    
    size = SSL_write(resource->ssl, message, message_len);
    
    if(size > 0){
        RETURN_LONG(write_bio_to_socket(resource));
    }
    
    RETURN_LONG(0);
}

static int write_bio_to_socket(uv_ssl_ext_t *resource){
    char buf[512];
    int hasread, ret;
    while(1){
        hasread  = BIO_read(resource->write_bio, buf, sizeof(buf));
        if(hasread <= 0){
            return 0;
        }
        ret = tcp_write_raw((uv_stream_t *) &resource->uv_tcp_ext, buf, hasread);
        if(ret != 0){
            break;   
        }
    }
    return ret;
}

PHP_METHOD(UVSSL, connect){
    long ret, port;
    zval rv;
    zval *self = getThis();
    zval *onConnectCallback;
    zval *loop;
    const char *host = NULL;
    size_t host_len;
    char cstr_host[30];
    struct sockaddr_in addr;
    
    uv_ssl_ext_t *resource = FETCH_OBJECT_SSL_RESOURCE(self);
    
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "slz", &host, &host_len, &port, &onConnectCallback)) {
        return;
    }
    
    if(host_len == 0 || host_len >= 30){        
        RETURN_LONG(-1);
    }

    registerFunctionCache(&resource->uv_tcp_ext.connectCallback, onConnectCallback);

    resource->sniConnectHostname = emalloc(host_len + 1);
    memcpy(resource->sniConnectHostname, host, host_len);
    resource->sniConnectHostname[host_len] = '\0';

    if((ret = uv_ip4_addr(resource->sniConnectHostname, port&0xffff, &addr)) != 0){
        uv_getaddrinfo_ext_t *addrinfo = emalloc(sizeof(uv_getaddrinfo_ext_t));
        addrinfo->ssl_handle = resource;
        loop = zend_read_property(CLASS_ENTRY(UVTcp), self, ZEND_STRL("loop"), 1, &rv);
        uv_getaddrinfo((Z_ISNULL_P(loop)?uv_default_loop():FETCH_UV_LOOP()), (uv_getaddrinfo_t *) addrinfo, (uv_getaddrinfo_cb) on_addrinfo_resolved, resource->sniConnectHostname, NULL, NULL);
        setSelfReference(&resource->uv_tcp_ext);
        resource->uv_tcp_ext.flag |= UV_TCP_HANDLE_START;
        resource->clientMode = 1;
        addrinfo->port = port;
        RETURN_LONG(0);
    }
    
    if((ret = uv_tcp_connect(&resource->uv_tcp_ext.connect_req, &resource->uv_tcp_ext.uv_tcp, (const struct sockaddr *) &addr, client_connection_cb)) != 0){
        RETURN_LONG(ret);
    }

    setSelfReference(&resource->uv_tcp_ext);
    resource->uv_tcp_ext.flag |= UV_TCP_HANDLE_START;
    resource->clientMode = 1;
    RETURN_LONG(ret);
}
