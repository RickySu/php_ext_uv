#include "uv_resolver.h"

CLASS_ENTRY_FUNCTION_D(UVResolver){
    REGISTER_CLASS(UVResolver);
    OBJECT_HANDLER(UVResolver).clone_obj = NULL;
    zend_declare_property_null(CLASS_ENTRY(UVResolver), ZEND_STRL("loop"), ZEND_ACC_PRIVATE TSRMLS_CC);
}

static void on_addrinfo_resolved(uv_getaddrinfo_ext_t *info, int status, struct addrinfo *res) {
    zval retval;
    zval *params[] = {NULL, NULL};
    char addr[17] = {'\0'};
    TSRMLS_FETCH();
    ZVAL_NULL(&retval);
    MAKE_STD_ZVAL(params[0]);
    MAKE_STD_ZVAL(params[1]);
    ZVAL_LONG(params[0], status);
    ZVAL_NULL(params[1]);
    if(status == 0){
        uv_ip4_name((struct sockaddr_in*) res->ai_addr, addr, 16);
        ZVAL_STRING(params[1], addr, 1);
        uv_freeaddrinfo(res);
    }
    call_user_function(CG(function_table), NULL, info->callback, &retval, 2, params TSRMLS_CC);
    zval_ptr_dtor(&params[0]);
    zval_ptr_dtor(&params[1]);
    zval_dtor(&retval);
    RELEASE_INFO(info);
}

static void on_nameinfo_resolved(uv_getnameinfo_ext_t *info, int status, const char *hostname, const char *service) {
    int i;
    zval retval;
    zval *params[] = {NULL, NULL, NULL};
    TSRMLS_FETCH();
    for(i=0;i<=2;i++){
        MAKE_STD_ZVAL(params[i]);
        ZVAL_NULL(params[i]);
    }
    ZVAL_NULL(&retval);
    ZVAL_LONG(params[0], status);
    if(status == 0){
        ZVAL_STRING(params[1], hostname, 1);
        ZVAL_STRING(params[2], service, 1);
    }
    call_user_function(CG(function_table), NULL, info->callback, &retval, 3, params TSRMLS_CC);
    for(i=0;i<=2;i++){
        zval_ptr_dtor(&params[i]);
    }
    zval_dtor(&retval);
    RELEASE_INFO(info);
}

PHP_METHOD(UVResolver, __construct){
    zval *loop;
    zval *self = getThis();
                    
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &loop)) {
        return;
    }
                                         
    if (IS_OBJECT != Z_TYPE_P(loop) ||
        !instanceof_function(Z_OBJCE_P(loop), CLASS_ENTRY(UVLoop) TSRMLS_CC)) {
        php_error_docref(NULL TSRMLS_CC, E_RECOVERABLE_ERROR, "$loop must be an instanceof UVLoop.");
        return;
    }
    zend_update_property(CLASS_ENTRY(UVResolver), self, ZEND_STRL("loop"), loop TSRMLS_CC);                                                                                      
}

PHP_METHOD(UVResolver, getnameinfo){
    long ret;
    zval *self = getThis();
    const char *addr;
    int addr_len;
    zval *nameinfoCallback;
    zval *loop;
    static struct sockaddr_in addr4;
    char cstr_addr[30];
    uv_getnameinfo_ext_t *info;
    
    loop = zend_read_property(CLASS_ENTRY(UVResolver), self, ZEND_STRL("loop"), 0 TSRMLS_CC);

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sz", &addr, &addr_len, &nameinfoCallback)) {
        return;
    }
   
    COPY_C_STR(cstr_addr, addr, addr_len);
     
    if((ret = uv_ip4_addr(addr, 0, &addr4)) !=0){
        RETURN_LONG(ret);
    }
    
    if (!zend_is_callable(nameinfoCallback, 0, NULL TSRMLS_CC)) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "param nameinfoCallback is not callable");
    }
    INIT_INFO(info, uv_getnameinfo_ext_t, self, nameinfoCallback);

    if((ret = uv_getnameinfo((uv_loop_t *) FETCH_OBJECT_RESOURCE(loop, uv_loop_ext_t), (uv_getnameinfo_t *) info, (uv_getnameinfo_cb) on_nameinfo_resolved, (const struct sockaddr*) &addr4, 0)) != 0) {
        RELEASE_INFO(info);
    }
    
    RETURN_LONG(ret);    
    
}

PHP_METHOD(UVResolver, getaddrinfo){
    long ret;
    zval *self = getThis();
    const char *node, *service;
    char *c_node, *c_service;
    int node_len, service_len;
    zval *addrinfoCallback;
    zval *loop;
    static struct sockaddr_in addr4;
    char cstr_addr[30];
    uv_getaddrinfo_ext_t *info;
    
    loop = zend_read_property(CLASS_ENTRY(UVResolver), self, ZEND_STRL("loop"), 0 TSRMLS_CC);

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ssz", &node, &node_len, &service, &service_len, &addrinfoCallback)) {
        return;
    }
   
    if (!zend_is_callable(addrinfoCallback, 0, NULL TSRMLS_CC)) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "param addrinfoCallback is not callable");
    }
    INIT_INFO(info, uv_getaddrinfo_ext_t, self, addrinfoCallback);

    MAKE_C_STR(c_node, node, node_len);
    MAKE_C_STR(c_service, service, service_len);
        
    if((ret = uv_getaddrinfo((uv_loop_t *) FETCH_OBJECT_RESOURCE(loop, uv_loop_ext_t), (uv_getaddrinfo_t *) info, (uv_getaddrinfo_cb) on_addrinfo_resolved, c_node, c_service, NULL)) != 0) {
        RELEASE_INFO(info);
    }
    
    efree(c_node);
    efree(c_service);
    RETURN_LONG(ret);    
    
}