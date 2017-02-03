#include "uv_udp.h"

CLASS_ENTRY_FUNCTION_D(UVUdp){
    REGISTER_CLASS_WITH_OBJECT_NEW(UVUdp, createUVUdpResource);
    OBJECT_HANDLER(UVUdp).offset = XtOffsetOf(uv_udp_ext_t, zo);;
    OBJECT_HANDLER(UVUdp).clone_obj = NULL;
    OBJECT_HANDLER(UVUdp).free_obj = freeUVUdpResource;
    OBJECT_HANDLER(UVUdp).get_gc = get_gc_UVUdpResource;
    zend_declare_property_null(CLASS_ENTRY(UVUdp), ZEND_STRL("loop"), ZEND_ACC_PRIVATE);
}

static HashTable *get_gc_UVUdpResource(zval *obj, zval **table, int *n) {
    uv_udp_ext_t *resource = FETCH_OBJECT_RESOURCE(obj, uv_udp_ext_t);   
    *table = &resource->gc_table;
    int index = 0;
    FCI_GC_TABLE_EX(resource, recvCallback, index);
    FCI_GC_TABLE_EX(resource, sendCallback, index);
    FCI_GC_TABLE_EX(resource, errorCallback, index);
    *n = index;
    return zend_std_get_properties(obj);
}

static void release(uv_udp_ext_t *resource){

    if(resource->sockPort != 0){
        resource->sockPort = 0;
        efree(resource->sockAddr);
        resource->sockAddr = NULL;
    }

    if(resource->flag & UV_UDP_READ_START){
        resource->flag &= ~UV_UDP_READ_START;
        uv_udp_recv_stop(&resource->uv_udp);
    }
    
    if(resource->flag & UV_UDP_HANDLE_START){
        resource->flag &= ~UV_UDP_HANDLE_START;
        uv_udp_recv_stop(&resource->uv_udp);
    }

    if(resource->flag & UV_UDP_HANDLE_INTERNAL_REF){
        resource->flag &= ~UV_UDP_HANDLE_INTERNAL_REF;
        zval_dtor(&resource->object);
    }
}

static void close_cb(uv_handle_t* handle) {
    release((uv_udp_ext_t *) handle);
}

static void send_cb(uv_udp_send_t* sr, int status) {
    send_req_t *req = (send_req_t *) sr;
    uv_udp_ext_t *resource = (uv_udp_ext_t *) sr->handle;
    char *s_addr;
    zval retval;
    zval params[4];
    params[0] = resource->object;
    
    if(!FCI_ISNULL(resource->sendCallback)){
        s_addr = sock_addr((struct sockaddr *) &req->addr);
        ZVAL_STRING(&params[1], s_addr);
        ZVAL_LONG(&params[2], sock_port((struct sockaddr *) &req->addr));
        ZVAL_LONG(&params[3], status);

        fci_call_function(&resource->sendCallback, &retval, 4, params);

        zval_dtor(&params[1]);
        zval_dtor(&retval);
        efree(s_addr);
    }
    efree(req->buf.base);
    efree(req);
}

static void alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
    buf->base = emalloc(suggested_size);
    buf->len = suggested_size;
}

static void recv_cb(uv_udp_ext_t* resource, ssize_t nread, const uv_buf_t* buf, const struct sockaddr* addr, unsigned int flags) {
    char *s_addr;
    zval params[5];
    zval retval;
    params[0] = resource->object;
    if(nread > 0){
        if(!FCI_ISNULL(resource->recvCallback)){
            s_addr = sock_addr((struct sockaddr *) addr);
            ZVAL_STRING(&params[1], s_addr);
            ZVAL_LONG(&params[2], sock_port((struct sockaddr *) addr));
            ZVAL_STRINGL(&params[3], buf->base, nread);
            ZVAL_LONG(&params[4], flags);
    
            fci_call_function(&resource->recvCallback, &retval, 5, params);
    
            zval_dtor(&params[1]);
            zval_dtor(&params[3]);
            zval_dtor(&retval);
            efree(s_addr);
        }    
    }
    else{
        if(!FCI_ISNULL(resource->errorCallback)){
            ZVAL_LONG(&params[1], nread);
            ZVAL_LONG(&params[2], flags);
    
            fci_call_function(&resource->errorCallback, &retval, 3, params);
    
            zval_dtor(&retval);
        }
    }
    efree(buf->base);
}

static zend_object *createUVUdpResource(zend_class_entry *ce) {
    uv_udp_ext_t *resource;
    resource = ALLOC_RESOURCE(uv_udp_ext_t);
    zend_object_std_init(&resource->zo, ce);
    object_properties_init(&resource->zo, ce);    
    resource->zo.handlers = &OBJECT_HANDLER(UVUdp);
    return &resource->zo;
}

void freeUVUdpResource(zend_object *object) {
    uv_udp_ext_t *resource;
    resource = FETCH_RESOURCE(object, uv_udp_ext_t);    
    release(resource);
    releaseUVUdpFunctionCache(resource);
    zend_object_std_dtor(&resource->zo);
}

static zend_always_inline void resolveSocket(uv_udp_ext_t *resource){
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

PHP_METHOD(UVUdp, __construct){
    zval *loop = NULL;
    zval *self = getThis();
    uv_udp_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_udp_ext_t);
                    
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "|z", &loop)) {
        return;
    }
    
    if(NULL == loop || ZVAL_IS_NULL(loop)){
        uv_udp_init(uv_default_loop(), (uv_udp_t *) resource);
        return;
    }

    zend_update_property(CLASS_ENTRY(UVUdp), self, ZEND_STRL("loop"), loop);
    uv_udp_init(FETCH_UV_LOOP(), (uv_udp_t *) resource);
}

PHP_METHOD(UVUdp, getSockname){
    zval *self = getThis();
    uv_udp_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_udp_ext_t);
    resolveSocket(resource);
    if(resource->sockPort == 0){
        RETURN_FALSE;
    }
    RETURN_STRING(resource->sockAddr);
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
    const char *host = NULL;
    size_t host_len;
    char cstr_host[30];
    struct sockaddr_in addr;
    
    uv_udp_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_udp_ext_t);
    
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "sl", &host, &host_len, &port)) {
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
    
    if((ret = uv_udp_bind(&resource->uv_udp, (const struct sockaddr*) &addr, 0)) != 0){
        RETURN_LONG(ret);
    }    
    
    resource->flag |= UV_UDP_HANDLE_START;
    
    RETURN_LONG(ret);
}

PHP_METHOD(UVUdp, setCallback){
    long ret;
    zval *self = getThis();
    uv_udp_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_udp_ext_t);

    if(resource->flag & UV_UDP_READ_START){
      RETURN_LONG(-1);
    }
    
    FCI_FREE(resource->recvCallback);
    FCI_FREE(resource->sendCallback);
    FCI_FREE(resource->errorCallback);
    
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "fff", FCI_PARSE_PARAMETERS_CC(resource->recvCallback), FCI_PARSE_PARAMETERS_CC(resource->sendCallback), FCI_PARSE_PARAMETERS_CC(resource->errorCallback))) {
        return;
    }

    if(ret = uv_udp_recv_start(&resource->uv_udp, alloc_cb, (uv_udp_recv_cb) recv_cb)){
        RETURN_LONG(ret);
    }

    FCI_ADDREF(resource->recvCallback);
    FCI_ADDREF(resource->sendCallback);
    FCI_ADDREF(resource->errorCallback);
    resource->flag |= (UV_UDP_HANDLE_INTERNAL_REF|UV_UDP_HANDLE_START|UV_UDP_READ_START);
    ZVAL_COPY(&resource->object, self);
    RETURN_LONG(ret);
}

PHP_METHOD(UVUdp, sendTo){
    char *dest = NULL, *message = NULL;
    char cstr_dest[30];
    size_t dest_len, message_len;
    long port, ret;
    send_req_t *req;
    zval *self = getThis();
    uv_udp_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_udp_ext_t);
    
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "sls", &dest, &dest_len, &port, &message, &message_len)) {
        return;
    }
    
    if(dest_len == 0 || dest_len >= 30){        
        RETURN_LONG(-1);
    }
    
    req = emalloc(sizeof(send_req_t));
    cstr_dest[dest_len] = '\0';    
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
