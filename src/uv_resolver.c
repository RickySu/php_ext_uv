#include "uv_resolver.h"

CLASS_ENTRY_FUNCTION_D(UVResolver){
    REGISTER_CLASS(UVResolver);
    OBJECT_HANDLER(UVResolver).clone_obj = NULL;
    zend_declare_property_null(CLASS_ENTRY(UVResolver), ZEND_STRL("loop"), ZEND_ACC_PRIVATE);
}

static void on_addrinfo_resolved(uv_getaddrinfo_ext_t *info, int status, struct addrinfo *res) {
    zval retval;
    zval params[2];
    char addr[17] = {'\0'};
    ZVAL_LONG(&params[0], status);
    ZVAL_NULL(&params[1]);
    if(status == 0){
        uv_ip4_name((struct sockaddr_in*) res->ai_addr, addr, 16);
        ZVAL_STRING(&params[1], addr);
        uv_freeaddrinfo(res);
    }
    fci_call_function(&info->callback, &retval, 2, params);
    zval_dtor(&params[1]);
    zval_dtor(&retval);
    RELEASE_INFO(info);
}

static void on_nameinfo_resolved(uv_getnameinfo_ext_t *info, int status, const char *hostname, const char *service) {
    zval retval;
    zval params[3];
    ZVAL_NULL(&retval);
    ZVAL_LONG(&params[0], status);
    ZVAL_NULL(&params[1]);
    ZVAL_NULL(&params[2]); 
    if(status == 0){
        ZVAL_STRING(&params[1], hostname);
        ZVAL_STRING(&params[2], service);
    }
    fci_call_function(&info->callback, &retval, 3, params);
    zval_dtor(&params[1]);
    zval_dtor(&params[2]);
    zval_dtor(&retval);
    RELEASE_INFO(info);
}

PHP_METHOD(UVResolver, __construct){
    zval *loop = NULL;
    zval *self = getThis();
                    
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "|z", &loop)) {
        return;
    }
    
    if(NULL == loop || ZVAL_IS_NULL(loop)){
        return;
    }
    
    zend_update_property(CLASS_ENTRY(UVResolver), self, ZEND_STRL("loop"), loop);                                                                                      
}

PHP_METHOD(UVResolver, getnameinfo){
    long ret;
    zval *self = getThis();
    const char *addr = NULL;
    size_t addr_len;
    zval *loop, rv;
    static struct sockaddr_in addr4;
    char cstr_addr[30];
    uv_getnameinfo_ext_t *info;
    loop = zend_read_property(CLASS_ENTRY(UVResolver), self, ZEND_STRL("loop"), 1, &rv);

    INIT_INFO(info, uv_getnameinfo_ext_t, self);
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "sf", &addr, &addr_len, FCI_PARSE_PARAMETERS_CC(info->callback))) {
        efree(info);
        return;
    }
    FCI_ADDREF(info->callback);
    
    COPY_C_STR(cstr_addr, addr, addr_len);
    if((ret = uv_ip4_addr(addr, 0, &addr4)) !=0){
        RELEASE_INFO(info);
        RETURN_LONG(ret);
    }

    if((ret = uv_getnameinfo(ZVAL_IS_NULL(loop)?uv_default_loop():FETCH_UV_LOOP(), (uv_getnameinfo_t *) info, (uv_getnameinfo_cb) on_nameinfo_resolved, (const struct sockaddr*) &addr4, 0)) != 0) {
        RELEASE_INFO(info);
    }
    
    RETURN_LONG(ret);
}

PHP_METHOD(UVResolver, getaddrinfo){
    long ret;
    zval *self = getThis();
    const char *node = NULL, *service = NULL;
    char *c_node, *c_service;
    size_t node_len, service_len;
    zval *loop, rv;
    static struct sockaddr_in addr4;
    char cstr_addr[30];
    uv_getaddrinfo_ext_t *info;
    
    loop = zend_read_property(CLASS_ENTRY(UVResolver), self, ZEND_STRL("loop"), 1, &rv);

    INIT_INFO(info, uv_getaddrinfo_ext_t, self);
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "ssf", &node, &node_len, &service, &service_len, FCI_PARSE_PARAMETERS_CC(info->callback))) {
        efree(info);
        return;
    }
    FCI_ADDREF(info->callback);
    
    MAKE_C_STR(c_node, node, node_len);
    MAKE_C_STR(c_service, service, service_len);
        
    if((ret = uv_getaddrinfo(ZVAL_IS_NULL(loop)?uv_default_loop():FETCH_UV_LOOP(), (uv_getaddrinfo_t *) info, (uv_getaddrinfo_cb) on_addrinfo_resolved, c_node, c_service, NULL)) != 0) {
        RELEASE_INFO(info);
    }
    
    efree(c_node);
    efree(c_service);
    RETURN_LONG(ret);    
}