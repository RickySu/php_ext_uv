#include "uv_udp.h"

CLASS_ENTRY_FUNCTION_D(UVUdp){
    REGISTER_CLASS_WITH_OBJECT_NEW(UVUdp, createUVUdpResource);
    OBJECT_HANDLER(UVUdp).clone_obj = NULL;
    zend_declare_property_null(CLASS_ENTRY(UVUdp), ZEND_STRL("recvCallback"), ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_null(CLASS_ENTRY(UVUdp), ZEND_STRL("sendCallback"), ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_null(CLASS_ENTRY(UVUdp), ZEND_STRL("errorCallback"), ZEND_ACC_PRIVATE TSRMLS_CC);
}

static void release(uv_udp_ext_t *resource){

    if(resource->flag & UV_UDP_READ_START){
        resource->flag &= UV_UDP_READ_START;
        uv_udp_recv_stop(&resource->uv_udp);
    }
    
    if(resource->flag & UV_UDP_HANDLE_START){
        resource->flag &= ~UV_UDP_HANDLE_START;
        uv_udp_recv_stop(&resource->uv_udp);
    }

    if(resource->flag & UV_UDP_HANDLE_INTERNAL_REF){
        resource->flag &= ~UV_UDP_HANDLE_INTERNAL_REF;
        Z_ADDREF_P(resource->object);
    }

    if(resource->sockPort != 0){
        resource->sockPort = 0;
        efree(resource->sockAddr);
        resource->sockAddr = NULL;
    }
}

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
    send_cb = zend_read_property(CLASS_ENTRY(UVUdp), resource->object, ZEND_STRL("sendCallback"), 0 TSRMLS_CC);
    
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
    zval *recvCallback, *errorCallback;
    zval *params[] = {resource->object, NULL, NULL, NULL, NULL};
    zval retval;
    TSRMLS_FETCH();
    recvCallback = zend_read_property(CLASS_ENTRY(UVUdp), resource->object, ZEND_STRL("recvCallback"), 0 TSRMLS_CC);
    errorCallback = zend_read_property(CLASS_ENTRY(UVUdp), resource->object, ZEND_STRL("errorCallback"), 0 TSRMLS_CC);    
    if(nread > 0){
        if(IS_NULL != Z_TYPE_P(recvCallback)){
            MAKE_STD_ZVAL(params[1]);
            ZVAL_STRING(params[1], sock_addr((struct sockaddr *) addr), 1);
            MAKE_STD_ZVAL(params[2]);
            ZVAL_LONG(params[2], sock_port((struct sockaddr *) addr));
            MAKE_STD_ZVAL(params[3]);
            ZVAL_STRINGL(params[3], buf->base, nread, 1);
            
            MAKE_STD_ZVAL(params[4]);
            ZVAL_LONG(params[4], flags);
    
            call_user_function(CG(function_table), NULL, recvCallback, &retval, 5, params TSRMLS_CC);
    
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

static zend_object_value createUVUdpResource(zend_class_entry *ce TSRMLS_DC) {
    zend_object_value retval;
    uv_udp_ext_t *resource;
    resource = (uv_udp_ext_t *) emalloc(sizeof(uv_udp_ext_t));
    memset(resource, 0, sizeof(uv_udp_ext_t));

    uv_udp_init(uv_default_loop(), &resource->uv_udp);
    zend_object_std_init(&resource->zo, ce TSRMLS_CC);
    object_properties_init(&resource->zo, ce);
    
    retval.handle = zend_objects_store_put(
        &resource->zo,
        (zend_objects_store_dtor_t) zend_objects_destroy_object,
        freeUVUdpResource,
        NULL TSRMLS_CC);

    retval.handlers = &OBJECT_HANDLER(UVUdp);
    return retval;
}

void freeUVUdpResource(void *object TSRMLS_DC) {
    uv_udp_ext_t *resource;
    resource = FETCH_RESOURCE(object, uv_udp_ext_t);
    
    release(resource);
    
    zend_object_std_dtor(&resource->zo TSRMLS_CC);
    efree(resource);
}

inline void resolveSocket(uv_udp_ext_t *resource){
    struct sockaddr addr;
    int addrlen;
    int ret;
    if(resource->sockPort == 0){
        addrlen = sizeof addr;
        if(ret = uv_udp_getsockname(&resource->uv_udp, &addr, &addrlen)){
            return;
        }
        resource->sockPort = sock_port(&addr);
        resource->sockAddr = sock_addr(&addr);
    }
}

PHP_METHOD(UVUdp, getSockname){
    zval *self = getThis();
    uv_udp_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_udp_ext_t);
    resolveSocket(resource);
    if(resource->sockPort == 0){
        RETURN_FALSE;
    }
    RETURN_STRING(resource->sockAddr, 1);
}

PHP_METHOD(UVUdp, getSockport){
   zval *self = getThis();
   uv_udp_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_udp_ext_t);
   resolveSocket(resource);
   if(resource->sockPort == 0){
       RETURN_FALSE;
   }
   RETURN_LONG(resource->sockPort);
}

PHP_METHOD(UVUdp, bind){
    long ret, port;
    zval *self = getThis();
    const char *host;
    int host_len;
    char cstr_host[30];
    struct sockaddr_in addr;
    
    uv_udp_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_udp_ext_t);
    
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sl", &host, &host_len, &port)) {
        return;
    }
    
    if(host_len == 0 || host_len >= 30){        
        RETURN_LONG(-1);
    }
    
    memcpy(cstr_host, host, host_len);
    if((ret = uv_ip4_addr(cstr_host, port&0xffff, &addr)) != 0){
        RETURN_LONG(ret);
    }
    
    if((ret = uv_udp_bind(&resource->uv_udp, (const struct sockaddr*) &addr, 0)) != 0){
        RETURN_LONG(ret);
    }    
    
    resource->flag |= UV_UDP_HANDLE_START;
    
    RETURN_LONG(ret);
}

PHP_METHOD(UVUdp, setCallback){
    long ret;
    zval *onRecvCallback, *onSendCallback, *onErrorCallback;
    zval *self = getThis();
    uv_udp_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_udp_ext_t);

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zzz", &onRecvCallback, &onSendCallback, &onErrorCallback)) {
        return;
    }
    
    if (!zend_is_callable(onRecvCallback, 0, NULL TSRMLS_CC) && IS_NULL != Z_TYPE_P(onRecvCallback)) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "param onRecvCallback is not callable");
    }
    
    if (!zend_is_callable(onSendCallback, 0, NULL TSRMLS_CC) && IS_NULL != Z_TYPE_P(onSendCallback)) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "param onSendCallback is not callable");
    }    
    
    if (!zend_is_callable(onErrorCallback, 0, NULL TSRMLS_CC) && IS_NULL != Z_TYPE_P(onErrorCallback)) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "param onErrorCallback is not callable");
    }
    
    ret = uv_udp_recv_start(&resource->uv_udp, alloc_cb, (uv_udp_recv_cb) recv_cb);

    if(ret == 0) {
        zend_update_property(CLASS_ENTRY(UVUdp), self, ZEND_STRL("recvCallback"), onRecvCallback TSRMLS_CC);
        zend_update_property(CLASS_ENTRY(UVUdp), self, ZEND_STRL("sendCallback"), onSendCallback TSRMLS_CC);
        zend_update_property(CLASS_ENTRY(UVUdp), self, ZEND_STRL("errorCallback"), onErrorCallback TSRMLS_CC);
        resource->object = self;
        resource->flag |= (UV_UDP_HANDLE_INTERNAL_REF|UV_UDP_HANDLE_START|UV_UDP_READ_START);
        Z_ADDREF_P(resource->object);
    }
}

PHP_METHOD(UVUdp, sendTo){
    char *dest, *message, cstr_dest[30];
    int dest_len, message_len;
    long port, ret;
    send_req_t *req;
    zval *self = getThis();
    uv_udp_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_udp_ext_t);
    
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sls", &dest, &dest_len, &port, &message, &message_len)) {
        return;
    }
    
    if(dest_len == 0 || dest_len >= 30){        
        RETURN_LONG(-1);
    }
    
    req = emalloc(sizeof(send_req_t));
    memcpy(cstr_dest, dest, dest_len);
    if((ret = uv_ip4_addr(cstr_dest, port&0xffff, &req->addr)) != 0){
        efree(req);
        RETURN_LONG(ret);
    }    
    req->buf.base = emalloc(message_len);
    req->buf.len = message_len;
    memcpy(req->buf.base, message, message_len);
    ret = uv_udp_send(
        (uv_udp_send_t *)req, 
        &resource->uv_udp, 
        &req->buf, 
        1, 
        (const struct sockaddr *) &req->addr,
        send_cb
    );
    RETURN_LONG(ret);
}

PHP_METHOD(UVUdp, close){
    zval *self = getThis();
    uv_udp_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_udp_ext_t);
    uv_close((uv_handle_t *) &resource->uv_udp, close_cb);
}