#include "uv_ssl.h"

CLASS_ENTRY_FUNCTION_D(UVSSL){
    EXTENDS_CLASS_WITH_OBJECT_NEW(UVSSL, createUVSSLResource, UVTcp);
    OBJECT_HANDLER(UVSSL).clone_obj = NULL;
    zend_declare_property_null(CLASS_ENTRY(UVSSL), ZEND_STRL("sslServerNameCallback"), ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_null(CLASS_ENTRY(UVSSL), ZEND_STRL("sslHandshakeCallback"), ZEND_ACC_PRIVATE TSRMLS_CC);
    REGISTER_CLASS_CONSTANT_LONG(UVSSL, SSL_METHOD_SSLV2);
    REGISTER_CLASS_CONSTANT_LONG(UVSSL, SSL_METHOD_SSLV3);
    REGISTER_CLASS_CONSTANT_LONG(UVSSL, SSL_METHOD_SSLV23);
    REGISTER_CLASS_CONSTANT_LONG(UVSSL, SSL_METHOD_TLSV1);
    REGISTER_CLASS_CONSTANT_LONG(UVSSL, SSL_METHOD_TLSV1_1);
    REGISTER_CLASS_CONSTANT_LONG(UVSSL, SSL_METHOD_TLSV1_2);
}


static void alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
    buf->base = emalloc(suggested_size);
    buf->len = suggested_size;
}

static void read_cb(uv_ssl_ext_t *resource, ssize_t nread, const uv_buf_t* buf) {
    TSRMLS_FETCH();
    zval *params[] = {resource->uv_tcp_ext.object, NULL};
    uv_tcp_ext_t *tcp_resource = (uv_tcp_ext_t *) resource;
    zval *readCallback = zend_read_property(CLASS_ENTRY(UVTcp), tcp_resource->object, ZEND_STRL("readCallback"), 0 TSRMLS_CC);
    zval *errorCallback = zend_read_property(CLASS_ENTRY(UVTcp), tcp_resource->object, ZEND_STRL("errorCallback"), 0 TSRMLS_CC);
    zval *sslHandshakeCallback = zend_read_property(CLASS_ENTRY(UVSSL), tcp_resource->object, ZEND_STRL("sslHandshakeCallback"), 0 TSRMLS_CC);
    zval retval;
    char read_buf[256];
    int size, err, ret, read_buf_index;
    if(nread > 0){
        BIO_write(resource->read_bio, buf->base, nread);
        if (!SSL_is_init_finished(resource->ssl)) {
            ret = SSL_do_handshake(resource->ssl);
            write_bio_to_socket(resource);
            if(ret != 1){
                err = SSL_get_error(resource->ssl, ret);
                if (err == SSL_ERROR_WANT_READ) {
                }
                else if(err == SSL_ERROR_WANT_WRITE){
                    write_bio_to_socket(resource);
                }
                else{
                }
            }
            else{
                if(IS_NULL != Z_TYPE_P(sslHandshakeCallback)){
                    ZVAL_NULL(&retval);
                    call_user_function(CG(function_table), NULL, sslHandshakeCallback, &retval, 1, params TSRMLS_CC);
                    zval_dtor(&retval);
                }
            }
            efree(buf->base);
            return;
        }
        tcp_resource->flag |= UV_TCP_WRITE_CALLBACK_ENABLE;
        read_buf_index = 0;
        while(1){
            size = SSL_read(resource->ssl, &read_buf[read_buf_index], sizeof(read_buf) - read_buf_index);
            if(size > 0){
                read_buf_index+=size;
            }
            if(size <= 0 || read_buf_index >= sizeof(read_buf)){
                if(IS_NULL != Z_TYPE_P(readCallback)){
                    ZVAL_NULL(&retval);
                    MAKE_STD_ZVAL(params[1]);
                    ZVAL_STRINGL(params[1], read_buf, read_buf_index, 1);
                    call_user_function(CG(function_table), NULL, readCallback, &retval, 2, params TSRMLS_CC);
                    zval_ptr_dtor(&params[1]);
                    zval_dtor(&retval);
                }
                read_buf_index = 0;
            }
            if(size <= 0){
                break;
            }
        }
    }
    else if(nread<0){
        if(IS_NULL != Z_TYPE_P(errorCallback)){
            ZVAL_NULL(&retval);
            MAKE_STD_ZVAL(params[1]);
            ZVAL_LONG(params[1], nread);
            call_user_function(CG(function_table), NULL, errorCallback, &retval, 2, params TSRMLS_CC);
            zval_ptr_dtor(&params[1]);
            zval_dtor(&retval);
        }
        tcp_close_socket((uv_tcp_ext_t *) resource);
    }
    efree(buf->base);
}

static int sni_cb(SSL *s, int *ad, void *arg) {
    TSRMLS_FETCH();
    long n;
    zval *params[] = {NULL};
    zval retval;
    uv_ssl_ext_t *resource = (uv_ssl_ext_t *) arg;
    uv_tcp_ext_t *tcp_resource = &resource->uv_tcp_ext;
    const char *servername = SSL_get_servername(s, TLSEXT_NAMETYPE_host_name);
    zval *sslServerNameCallback = zend_read_property(CLASS_ENTRY(UVSSL), tcp_resource->object, ZEND_STRL("sslServerNameCallback"), 0 TSRMLS_CC);
    if(servername != NULL && IS_NULL != Z_TYPE_P(sslServerNameCallback)){
        ZVAL_NULL(&retval);
        MAKE_STD_ZVAL(params[0]);
        ZVAL_STRING(params[0], servername, 1);
        call_user_function(CG(function_table), NULL, sslServerNameCallback, &retval, 1, params TSRMLS_CC);
        if(Z_TYPE_P(&retval) == IS_LONG){
            n = Z_LVAL_P(&retval);
            if(n>=0 && n<resource->nctx){
                SSL_set_SSL_CTX(s, resource->ctx[n]);
            }
        }
        zval_ptr_dtor(&params[0]);
        zval_dtor(&retval);
    }
    return SSL_TLSEXT_ERR_OK;
}

static void client_connection_cb(uv_connect_t* req, int status) { 
    TSRMLS_FETCH();
    uv_ssl_ext_t *resource = (uv_ssl_ext_t *) req->handle;
    uv_tcp_ext_t *tcp_resource = (uv_tcp_ext_t *) resource;
    zval *params[] = {tcp_resource->object, NULL};
    zval retval;
    ZVAL_NULL(&retval);
    MAKE_STD_ZVAL(params[1]);
    ZVAL_LONG(params[1], status);
    
    zval *callback = zend_read_property(CLASS_ENTRY(UVTcp), tcp_resource->object, ZEND_STRL("connectCallback"), 0 TSRMLS_CC);
    
    if(uv_read_start((uv_stream_t *) resource, alloc_cb, (uv_read_cb) read_cb)){
        return;
    }
    
    tcp_resource->flag |= (UV_TCP_HANDLE_START|UV_TCP_READ_START);
    resource->ssl = SSL_new(resource->ctx[0]);
    resource->read_bio = BIO_new(BIO_s_mem());
    resource->write_bio = BIO_new(BIO_s_mem());
    SSL_set_bio(resource->ssl, resource->read_bio, resource->write_bio);
    SSL_set_connect_state(resource->ssl);
    SSL_connect(resource->ssl);
    write_bio_to_socket(resource);
    call_user_function(CG(function_table), NULL, callback, &retval, 2, params TSRMLS_CC);
    zval_ptr_dtor(&params[1]);
    zval_dtor(&retval);
}


static zend_object_value createUVSSLResource(zend_class_entry *ce TSRMLS_DC) {
    zend_object_value retval;
    uv_ssl_ext_t *ssl_resource;
    uv_tcp_ext_t *tcp_resource;

    ssl_resource = (uv_ssl_ext_t *) emalloc(sizeof(uv_ssl_ext_t));
    memset(ssl_resource, 0, sizeof(uv_ssl_ext_t));
    tcp_resource = (uv_tcp_ext_t *) ssl_resource;
    zend_object_std_init(&tcp_resource->zo, ce TSRMLS_CC);
    object_properties_init(&tcp_resource->zo, ce);
                            
    retval.handle = zend_objects_store_put(
        &tcp_resource->zo,
        (zend_objects_store_dtor_t) zend_objects_destroy_object,
        freeUVSSLResource,
        NULL TSRMLS_CC);
                                                                
    retval.handlers = &OBJECT_HANDLER(UVSSL);
    return retval;
}

void freeUVSSLResource(void *object TSRMLS_DC) {
    int i;
    uv_ssl_ext_t *resource;
    resource = (uv_ssl_ext_t *) FETCH_RESOURCE(object, uv_tcp_ext_t);
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
    freeUVTcpResource(object TSRMLS_CC);
}

PHP_METHOD(UVSSL, setSSLServerNameCallback){
    zval *sslServerNameCallback;
    zval *self = getThis();
    
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &sslServerNameCallback)){
        return;
    }
    
    if (!zend_is_callable(sslServerNameCallback, 0, NULL TSRMLS_CC) && IS_NULL != Z_TYPE_P(sslServerNameCallback)) {
       php_error_docref(NULL TSRMLS_CC, E_WARNING, "param callback is not callable");
    }
    
    zend_update_property(CLASS_ENTRY(UVSSL), self, ZEND_STRL("sslServerNameCallback"), sslServerNameCallback TSRMLS_CC);
}

PHP_METHOD(UVSSL, setSSLHandshakeCallback){
    zval *sslHandshakeCallback;
    zval *self = getThis();
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &sslHandshakeCallback)){
        return;
    }
    
    if (!zend_is_callable(sslHandshakeCallback, 0, NULL TSRMLS_CC) && IS_NULL != Z_TYPE_P(sslHandshakeCallback)) {
       php_error_docref(NULL TSRMLS_CC, E_WARNING, "param callback is not callable");
    }
    
    zend_update_property(CLASS_ENTRY(UVSSL), self, ZEND_STRL("sslHandshakeCallback"), sslHandshakeCallback TSRMLS_CC);
}

PHP_METHOD(UVSSL, setPrivateKey){
    const char *key;
    int key_len;
    zval *self = getThis();
    long n = 0;
    EVP_PKEY *pkey;
    BIO *key_bio;
    int result;
    uv_tcp_ext_t *tcp_resource = FETCH_OBJECT_RESOURCE(self, uv_tcp_ext_t);
    uv_ssl_ext_t *resource = (uv_ssl_ext_t *) tcp_resource;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|l", &key, &key_len, &n)){
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
    const char *cert;
    int cert_len;
    zval *self = getThis();
    long n = 0;
    X509 *pcert;  
    BIO *cert_bio;
    int result;
    uv_tcp_ext_t *tcp_resource = FETCH_OBJECT_RESOURCE(self, uv_tcp_ext_t);
    uv_ssl_ext_t *resource = (uv_ssl_ext_t *) tcp_resource;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|l", &cert, &cert_len, &n)){
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
    uv_ssl_ext_t *server_resource = (uv_ssl_ext_t *) FETCH_OBJECT_RESOURCE(self, uv_tcp_ext_t);
    uv_ssl_ext_t *client_resource;
    
    object_init_ex(return_value, CLASS_ENTRY(UVSSL));
    if(!make_accepted_uv_tcp_object((uv_tcp_ext_t *) server_resource, return_value TSRMLS_CC)){
        RETURN_FALSE;
    }
    
    client_resource = (uv_ssl_ext_t *) FETCH_OBJECT_RESOURCE(return_value, uv_tcp_ext_t);
    if(uv_read_start((uv_stream_t *) client_resource, alloc_cb, (uv_read_cb) read_cb)){
        zval_dtor(return_value);
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
    uv_tcp_ext_t *tcp_resource = FETCH_OBJECT_RESOURCE(self, uv_tcp_ext_t);
    uv_ssl_ext_t *resource = (uv_ssl_ext_t *) tcp_resource;
   
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|zll", &loop, &sslMethod, &nContexts)) {
        return;
    }

    tcp_resource->object = self;

    if(NULL == loop || ZVAL_IS_NULL(loop)){
        uv_tcp_init(uv_default_loop(), (uv_tcp_t *) resource);
    }
    else{
        if(!check_zval_type(CLASS_ENTRY(UVSSL), ZEND_STRL("__construct") + 1, CLASS_ENTRY(UVLoop), loop TSRMLS_CC)){
            return;
        }
        zend_update_property(CLASS_ENTRY(UVTcp), self, ZEND_STRL("loop"), loop TSRMLS_CC);
        uv_tcp_init(FETCH_UV_LOOP(), (uv_tcp_t *) resource);
    }

    switch(sslMethod){
        case SSL_METHOD_SSLV2:
#ifdef OPENSSL_NO_SSL2
            resource->ssl_method = SSLv3_method();
#else
            resource->ssl_method = SSLv2_method();
#endif
            break;
        case SSL_METHOD_SSLV3:
            resource->ssl_method = SSLv3_method();
            break;
        case SSL_METHOD_SSLV23:
            resource->ssl_method = SSLv23_method();
            break;
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
    const char *message;
    int message_len, size;
    uv_tcp_ext_t *tcp_resource = FETCH_OBJECT_RESOURCE(self, uv_tcp_ext_t);
    uv_ssl_ext_t *resource = (uv_ssl_ext_t *) tcp_resource;
   
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &message, &message_len)) {
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
        ret = tcp_write_raw((uv_stream_t *) resource, buf, hasread);
        if(ret != 0){
            break;   
        }
    }
    return ret;
}
PHP_METHOD(UVSSL, connect){
    long ret, port;
    zval *self = getThis();
    zval *onConnectCallback;
    const char *host;
    int host_len;
    char cstr_host[30];
    struct sockaddr_in addr;
    
    uv_tcp_ext_t *tcp_resource = FETCH_OBJECT_RESOURCE(self, uv_tcp_ext_t);
    uv_ssl_ext_t *resource = (uv_ssl_ext_t *) tcp_resource;
    
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "slz", &host, &host_len, &port, &onConnectCallback)) {
        return;
    }
    
    if(host_len == 0 || host_len >= 30){        
        RETURN_LONG(-1);
    }

    memcpy(cstr_host, host, host_len);
    cstr_host[host_len] = '\0';
    if((ret = uv_ip4_addr(cstr_host, port&0xffff, &addr)) != 0){
        RETURN_LONG(ret);
    }
    
    if((ret = uv_tcp_connect(&tcp_resource->connect_req, &tcp_resource->uv_tcp, (const struct sockaddr *) &addr, client_connection_cb)) != 0){
        RETURN_LONG(ret);
    }
    
    if (!zend_is_callable(onConnectCallback, 0, NULL TSRMLS_CC)) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "param onConnectCallback is not callable");
    }    
    
    zend_update_property(CLASS_ENTRY(UVTcp), self, ZEND_STRL("connectCallback"), onConnectCallback TSRMLS_CC);
    setSelfReference(tcp_resource);
    tcp_resource->flag |= UV_TCP_HANDLE_START;    
    RETURN_LONG(ret);
}
