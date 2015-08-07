#include "uv_ssl.h"

CLASS_ENTRY_FUNCTION_D(UVSSL){
    EXTENDS_CLASS_WITH_OBJECT_NEW(UVSSL, createUVSSLResource, UVTcp);
    OBJECT_HANDLER(UVSSL).clone_obj = NULL;    
    REGISTER_CLASS_CONSTANT_LONG(UVSSL, SSL_METHOD_SSLV2);
    REGISTER_CLASS_CONSTANT_LONG(UVSSL, SSL_METHOD_SSLV3);
    REGISTER_CLASS_CONSTANT_LONG(UVSSL, SSL_METHOD_SSLV23);
    REGISTER_CLASS_CONSTANT_LONG(UVSSL, SSL_METHOD_TLSV1);
    REGISTER_CLASS_CONSTANT_LONG(UVSSL, SSL_METHOD_TLSV1_1);
    REGISTER_CLASS_CONSTANT_LONG(UVSSL, SSL_METHOD_TLSV1_2);
}

static int sni_cb(SSL *s, int *ad, void *arg) {
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
    uv_ssl_ext_t *resource;
    resource = (uv_ssl_ext_t *) FETCH_RESOURCE(object, uv_tcp_ext_t);
    printf("aaaa\n");
    freeUVTcpResource(object);            
}

PHP_METHOD(UVSSL, __construct){
    long sslMethod = SSL_METHOD_TLSV1_1, nContexts = 1;
    int i;
    zval *loop;
    zval *self = getThis();
    uv_tcp_ext_t *tcp_resource = FETCH_OBJECT_RESOURCE(self, uv_tcp_ext_t);
    uv_ssl_ext_t *resource = (uv_ssl_ext_t *) tcp_resource;
   
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|ll", &loop, &sslMethod, &nContexts)) {
        return;
    }

    if (IS_OBJECT != Z_TYPE_P(loop) ||
        instanceof_function(Z_OBJCE_P(loop), CLASS_ENTRY(UVLoop) TSRMLS_CC)) {
        php_error_docref(NULL TSRMLS_CC, E_RECOVERABLE_ERROR, "$loop must be an instanceof UVLoop.");
        return;
    }
    
    tcp_resource->object = self;
    zend_update_property(CLASS_ENTRY(UVTcp), self, ZEND_STRL("loop"), loop TSRMLS_CC);
    uv_tcp_init((uv_loop_t *) FETCH_OBJECT_RESOURCE(loop, uv_loop_ext_t), (uv_tcp_t *) resource);
    
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
