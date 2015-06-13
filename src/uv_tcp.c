#include "uv_tcp.h"

CLASS_ENTRY_FUNCTION_D(UVTcp){
    REGISTER_CLASS_WITH_OBJECT_NEW(UVTcp, createUVTcpResource);
    OBJECT_HANDLER(UVTcp).clone_obj = NULL;
    zend_declare_property_null(CLASS_ENTRY(UVTcp), ZEND_STRL("readCallback"), ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_null(CLASS_ENTRY(UVTcp), ZEND_STRL("writeCallback"), ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_null(CLASS_ENTRY(UVTcp), ZEND_STRL("errorCallback"), ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_null(CLASS_ENTRY(UVTcp), ZEND_STRL("connectCallback"), ZEND_ACC_PRIVATE TSRMLS_CC);
}

static void connection_cb(uv_tcp_ext_t *resource, int status) {
    zval *connect_cb;
    zval retval;
    zval *params[] = {resource->object, NULL};
    ZVAL_NULL(&retval);
    MAKE_STD_ZVAL(params[1]);
    ZVAL_LONG(params[1], status);
    TSRMLS_FETCH();
    connect_cb = zend_read_property(CLASS_ENTRY(UVTcp), resource->object, ZEND_STRL("connectCallback"), 0 TSRMLS_CC);
    call_user_function(CG(function_table), NULL, connect_cb, &retval, 2, params TSRMLS_CC);
    zval_ptr_dtor(&params[1]);
    zval_dtor(&retval);
}

static void release(uv_tcp_ext_t *resource){
/*
    if(resource->flag & UV_TCP_READ_START){
        resource->flag &= UV_TCP_READ_START;
        uv_tcp_recv_stop(&resource->uv_tcp);
    }
    
    if(resource->flag & UV_TCP_HANDLE_START){
        resource->flag &= ~UV_TCP_HANDLE_START;
        uv_tcp_recv_stop(&resource->uv_tcp);
    }

    if(resource->flag & UV_TCP_HANDLE_INTERNAL_REF){
        resource->flag &= ~UV_TCP_HANDLE_INTERNAL_REF;
        Z_DELREF_P(resource->object);
    }

    if(resource->sockPort != 0){
        resource->sockPort = 0;
        efree(resource->sockAddr);
        resource->sockAddr = NULL;
    }
*/
}
/*
static void close_cb(uv_handle_t* handle) {
    release((uv_udp_ext_t *) handle);
}

static void send_cb(uv_udp_send_t* sr, int status) {
    send_req_t *req = (send_req_t *) sr;
    uv_udp_ext_t *resource = (uv_udp_ext_t *) sr->handle;
    zval retval;
    zval *params[] = {resource->object, NULL, NULL, NULL};
    zval *send_cb;
    TSRMLS_FETCH();
    send_cb = zend_read_property(CLASS_ENTRY(UVTcp), resource->object, ZEND_STRL("writeCallback"), 0 TSRMLS_CC);
    
    if(IS_NULL != Z_TYPE_P(send_cb)){    
        MAKE_STD_ZVAL(params[1]);
        ZVAL_STRING(params[1], sock_addr((struct sockaddr *) &req->addr), 1);
        MAKE_STD_ZVAL(params[2]);
        ZVAL_LONG(params[2], sock_port((struct sockaddr *) &req->addr));
        MAKE_STD_ZVAL(params[3]);
        ZVAL_LONG(params[3], status);
    
        call_user_function(CG(function_table), NULL, send_cb, &retval, 4, params TSRMLS_CC);
    
        zval_ptr_dtor(&params[1]);
        zval_ptr_dtor(&params[2]);
        zval_ptr_dtor(&params[3]);
        zval_dtor(&retval);
    }
    efree(req->buf.base);
    efree(req);
}

static void alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
    buf->base = emalloc(suggested_size);
    buf->len = suggested_size;
}

static void recv_cb(uv_udp_ext_t* resource, ssize_t nread, const uv_buf_t* buf, const struct sockaddr* addr, unsigned int flags) {
    zval *readCallback, *errorCallback;
    zval *params[] = {resource->object, NULL, NULL, NULL, NULL};
    zval retval;
    TSRMLS_FETCH();
    readCallback = zend_read_property(CLASS_ENTRY(UVTcp), resource->object, ZEND_STRL("readCallback"), 0 TSRMLS_CC);
    errorCallback = zend_read_property(CLASS_ENTRY(UVTcp), resource->object, ZEND_STRL("errorCallback"), 0 TSRMLS_CC);    
    if(nread > 0){
        if(IS_NULL != Z_TYPE_P(readCallback)){
            MAKE_STD_ZVAL(params[1]);
            ZVAL_STRING(params[1], sock_addr((struct sockaddr *) addr), 1);
            MAKE_STD_ZVAL(params[2]);
            ZVAL_LONG(params[2], sock_port((struct sockaddr *) addr));
            MAKE_STD_ZVAL(params[3]);
            ZVAL_STRINGL(params[3], buf->base, nread, 1);
            
            MAKE_STD_ZVAL(params[4]);
            ZVAL_LONG(params[4], flags);
    
            call_user_function(CG(function_table), NULL, readCallback, &retval, 5, params TSRMLS_CC);
    
            zval_ptr_dtor(&params[1]);
            zval_ptr_dtor(&params[2]);
            zval_ptr_dtor(&params[3]);
            zval_ptr_dtor(&params[4]);
            zval_dtor(&retval);
        }    
    }
    else{
        if(IS_NULL != Z_TYPE_P(errorCallback)){
            MAKE_STD_ZVAL(params[1]);
            ZVAL_LONG(params[1], nread);
            MAKE_STD_ZVAL(params[2]);
            ZVAL_LONG(params[2], flags);
    
            call_user_function(CG(function_table), NULL, errorCallback, &retval, 3, params TSRMLS_CC);
    
            zval_ptr_dtor(&params[1]);
            zval_ptr_dtor(&params[2]);
            zval_dtor(&retval);
        }
    }
    efree(buf->base);
}
*/

static zend_object_value createUVTcpResource(zend_class_entry *ce TSRMLS_DC) {
    zend_object_value retval;
    uv_tcp_ext_t *resource;
    resource = (uv_tcp_ext_t *) emalloc(sizeof(uv_tcp_ext_t));
    memset(resource, 0, sizeof(uv_tcp_ext_t));

    uv_tcp_init(uv_default_loop(), &resource->uv_tcp);
    zend_object_std_init(&resource->zo, ce TSRMLS_CC);
    object_properties_init(&resource->zo, ce);
    
    retval.handle = zend_objects_store_put(
        &resource->zo,
        (zend_objects_store_dtor_t) zend_objects_destroy_object,
        freeUVTcpResource,
        NULL TSRMLS_CC);

    retval.handlers = &OBJECT_HANDLER(UVTcp);
    return retval;
}

void freeUVTcpResource(void *object TSRMLS_DC) {
    uv_tcp_ext_t *resource;
    resource = FETCH_RESOURCE(object, uv_tcp_ext_t);
    
    release(resource);
    
    zend_object_std_dtor(&resource->zo TSRMLS_CC);
    efree(resource);
}

static inline void resolveSocket(uv_tcp_ext_t *resource){
    struct sockaddr addr;
    int addrlen;
    int ret;
    if(resource->sockPort == 0){
        addrlen = sizeof addr;
        if(ret = uv_tcp_getsockname(&resource->uv_tcp, &addr, &addrlen)){
            return;
        }
        resource->sockPort = sock_port(&addr);
        resource->sockAddr = sock_addr(&addr);
    }
}

static inline void resolvePeerSocket(uv_tcp_ext_t *resource){
    struct sockaddr addr;
    int addrlen;
    int ret;
    if(resource->peerPort == 0){
        addrlen = sizeof addr;
        if(ret = uv_tcp_getpeername(&resource->uv_tcp, &addr, &addrlen)){
            return;
        }
        resource->peerPort = sock_port(&addr);
        resource->peerAddr = sock_addr(&addr);
    }
}



PHP_METHOD(UVTcp, getSockname){
    zval *self = getThis();
    uv_tcp_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_tcp_ext_t);
    resolveSocket(resource);
    if(resource->sockPort == 0){
        RETURN_FALSE;
    }
    RETURN_STRING(resource->sockAddr, 1);
}

PHP_METHOD(UVTcp, getSockport){
   zval *self = getThis();
   uv_tcp_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_tcp_ext_t);
   resolveSocket(resource);
   if(resource->sockPort == 0){
       RETURN_FALSE;
   }
   RETURN_LONG(resource->sockPort);
}

PHP_METHOD(UVTcp, getPeername){
    zval *self = getThis();
    uv_tcp_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_tcp_ext_t);
    resolvePeerSocket(resource);
    if(resource->peerPort == 0){
        RETURN_FALSE;
    }
    RETURN_STRING(resource->peerAddr, 1);
}

PHP_METHOD(UVTcp, getPeerport){
   zval *self = getThis();
   uv_tcp_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_tcp_ext_t);
   resolvePeerSocket(resource);
   if(resource->peerPort == 0){
       RETURN_FALSE;
   }
   RETURN_LONG(resource->peerPort);
}

PHP_METHOD(UVTcp, accept){
    long ret, port;
    zval *self = getThis();
    zval *onConnectCallback;
    const char *host;
    int host_len;
    char cstr_host[30];
    struct sockaddr_in addr;
    
    uv_tcp_ext_t *server_resource, *client_resource;
    server_resource = FETCH_OBJECT_RESOURCE(self, uv_tcp_ext_t);
    object_init_ex(return_value, CLASS_ENTRY(UVTcp));
    client_resource = FETCH_OBJECT_RESOURCE(return_value, uv_tcp_ext_t); 
    
    if(ret = uv_accept((uv_stream_t *) &server_resource->uv_tcp, (uv_stream_t *) &client_resource->uv_tcp)) {
        zval_ptr_dtor(&return_value);
        RETURN_LONG(ret);
    }
    client_resource->flag |= UV_TCP_HANDLE_INTERNAL_REF;
}    

PHP_METHOD(UVTcp, listen){
    long ret, port;
    zval *self = getThis();
    zval *onConnectCallback;
    const char *host;
    int host_len;
    char cstr_host[30];
    struct sockaddr_in addr;
    
    uv_tcp_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_tcp_ext_t);
    
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
    
    if((ret = uv_tcp_bind(&resource->uv_tcp, (const struct sockaddr*) &addr, 0)) != 0){
        RETURN_LONG(ret);
    }
    
    if((ret = uv_listen((uv_stream_t *) &resource->uv_tcp, SOMAXCONN, (uv_connection_cb) connection_cb)) != 0){
        RETURN_LONG(ret);
    }
    
    if (!zend_is_callable(onConnectCallback, 0, NULL TSRMLS_CC)) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "param onConnectCallback is not callable");
    }    
    
    zend_update_property(CLASS_ENTRY(UVTcp), self, ZEND_STRL("connectCallback"), onConnectCallback TSRMLS_CC);
    resource->object = self;
    Z_ADDREF_P(resource->object);    
    resource->flag |= UV_TCP_HANDLE_START;
    
    RETURN_LONG(ret);
}

PHP_METHOD(UVTcp, setCallback){
    long ret;
    zval *onReadCallback, *onWriteCallback, *onErrorCallback;
    zval *self = getThis();
    uv_tcp_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_tcp_ext_t);

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zzz", &onReadCallback, &onWriteCallback, &onErrorCallback)) {
        return;
    }
    
    if (!zend_is_callable(onReadCallback, 0, NULL TSRMLS_CC) && IS_NULL != Z_TYPE_P(onReadCallback)) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "param onReadCallback is not callable");
    }
    
    if (!zend_is_callable(onWriteCallback, 0, NULL TSRMLS_CC) && IS_NULL != Z_TYPE_P(onWriteCallback)) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "param onWriteCallback is not callable");
    }    
    
    if (!zend_is_callable(onErrorCallback, 0, NULL TSRMLS_CC) && IS_NULL != Z_TYPE_P(onErrorCallback)) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "param onErrorCallback is not callable");
    }
    
//    ret = uv_tcp_recv_start(&resource->uv_tcp, alloc_cb, (uv_tcp_recv_cb) recv_cb);

    if(ret == 0) {
        zend_update_property(CLASS_ENTRY(UVTcp), self, ZEND_STRL("readCallback"), onReadCallback TSRMLS_CC);
        zend_update_property(CLASS_ENTRY(UVTcp), self, ZEND_STRL("writeCallback"), onWriteCallback TSRMLS_CC);
        zend_update_property(CLASS_ENTRY(UVTcp), self, ZEND_STRL("errorCallback"), onErrorCallback TSRMLS_CC);
        resource->object = self;
        resource->flag |= (UV_TCP_HANDLE_INTERNAL_REF|UV_TCP_HANDLE_START|UV_TCP_READ_START);
        Z_ADDREF_P(resource->object);
    }
     RETURN_LONG(ret);
}
