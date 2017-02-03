#include "uv_tcp.h"

void tcp_close_socket(uv_tcp_ext_t *handle){       
    if(handle->flag & UV_TCP_CLOSING_START){
         return;
    }
    handle->flag |= UV_TCP_CLOSING_START;
    uv_close((uv_handle_t *) handle, tcp_close_cb);
}

void setSelfReference(uv_tcp_ext_t *resource)
{
    if(resource->flag & UV_TCP_HANDLE_INTERNAL_REF){
        return;
    }
    Z_ADDREF(resource->object);
    resource->flag |= UV_TCP_HANDLE_INTERNAL_REF;
}

CLASS_ENTRY_FUNCTION_D(UVTcp){
    REGISTER_CLASS_WITH_OBJECT_NEW(UVTcp, createUVTcpResource);
    OBJECT_HANDLER(UVTcp).offset = XtOffsetOf(uv_tcp_ext_t, zo);
    OBJECT_HANDLER(UVTcp).clone_obj = NULL;
    OBJECT_HANDLER(UVTcp).free_obj = freeUVTcpResource;
    OBJECT_HANDLER(UVTcp).get_gc = get_gc_UVTcpResource;
    zend_declare_property_null(CLASS_ENTRY(UVTcp), ZEND_STRL("loop"), ZEND_ACC_PRIVATE);
}

HashTable *get_gc_UVTcpResource(zval *obj, zval **table, int *n) {
    uv_tcp_ext_t *resource = FETCH_OBJECT_RESOURCE(obj, uv_tcp_ext_t);
    *table = &resource->gc_table;
    int index = 0;
    FCI_GC_TABLE_EX(resource, readCallback, index);
    FCI_GC_TABLE_EX(resource, writeCallback, index);
    FCI_GC_TABLE_EX(resource, errorCallback, index);
    FCI_GC_TABLE_EX(resource, connectCallback, index);
    FCI_GC_TABLE_EX(resource, shutdownCallback, index);
    *n = index;
    return zend_std_get_properties(obj);
}

void releaseResource(uv_tcp_ext_t *resource){
    if(resource->sockPort != 0){
        resource->sockPort = 0;
        efree(resource->sockAddr);
        resource->sockAddr = NULL;
    }

    if(resource->peerPort != 0){
        resource->peerPort = 0;
        efree(resource->peerAddr);
        resource->peerAddr = NULL;
    }

    if(resource->flag & UV_TCP_READ_START){
        resource->flag &= ~UV_TCP_READ_START;
        uv_read_stop((uv_stream_t *) &resource->uv_tcp);
    }
    
    if(resource->flag & UV_TCP_HANDLE_START){
        resource->flag &= ~UV_TCP_HANDLE_START;
        uv_unref((uv_handle_t *) &resource->uv_tcp);
    }

    if(resource->flag & UV_TCP_HANDLE_INTERNAL_REF){
        resource->flag &= ~UV_TCP_HANDLE_INTERNAL_REF;
        zval_dtor(&resource->object);
    }    
}

static void shutdown_cb(uv_shutdown_t* req, int status) {
    uv_tcp_ext_t *resource = (uv_tcp_ext_t *) req->handle;
    zval retval;
    zval params[2];
    params[0] = resource->object;
    if(!FCI_ISNULL(resource->shutdownCallback)){
        ZVAL_LONG(&params[1], status);
        fci_call_function(&resource->shutdownCallback, &retval, 2, params);
        zval_dtor(&retval);
    }
}

static void tcp_close_cb(uv_handle_t* handle) {
    releaseResource((uv_tcp_ext_t *) handle);
}

static void write_cb(uv_write_t *wr, int status){
    zval retval;
    write_req_t *req = (write_req_t *) wr;
    uv_tcp_ext_t *resource = (uv_tcp_ext_t *) req->uv_write.handle;
    zval params[3];
    params[0] = resource->object;

    if(resource->flag & UV_TCP_WRITE_CALLBACK_ENABLE && !FCI_ISNULL(resource->writeCallback)){
        ZVAL_LONG(&params[1], status);
        ZVAL_LONG(&params[2], req->buf.len);    
        fci_call_function(&resource->writeCallback, &retval, 3, params);
        zval_dtor(&retval);
    }
    efree(req->buf.base);
    efree(req);
}

static void alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
    buf->base = emalloc(suggested_size);
    buf->len = suggested_size;
}

static void read_cb(uv_tcp_ext_t *resource, ssize_t nread, const uv_buf_t* buf) {
    zval retval;
    zval params[2];
    params[0] = resource->object;
    
    if(nread > 0){
        if(!FCI_ISNULL(resource->readCallback)){
            ZVAL_STRINGL(&params[1], buf->base, nread);
            fci_call_function(&resource->readCallback, &retval, 2, params);
            zval_dtor(&params[1]);
            zval_dtor(&retval);
        }
    }
    else{    
        if(!FCI_ISNULL(resource->errorCallback)){
            ZVAL_LONG(&params[1], nread);        
            fci_call_function(&resource->errorCallback, &retval, 2, params);
            zval_dtor(&retval);
        }
        tcp_close_socket((uv_tcp_ext_t *) &resource->uv_tcp);
    }
    efree(buf->base);
}

static void client_connection_cb(uv_connect_t* req, int status) {
    zval retval;
    uv_tcp_ext_t *resource = (uv_tcp_ext_t *) req->handle;
    zval params[2];
    params[0] = resource->object;
    ZVAL_NULL(&retval);
    ZVAL_LONG(&params[1], status);

    if(uv_read_start((uv_stream_t *) resource, alloc_cb, (uv_read_cb) read_cb)){
        return;
    }
    resource->flag |= (UV_TCP_HANDLE_START|UV_TCP_READ_START);
    
    fci_call_function(&resource->connectCallback, &retval, 2, params);
    zval_dtor(&retval);
}

static void connection_cb(uv_tcp_ext_t *resource, int status) {
    zval retval;
    zval params[2];
    params[0] = resource->object;
    ZVAL_NULL(&retval);
    ZVAL_LONG(&params[1], status);
    fci_call_function(&resource->connectCallback, &retval, 2, params);
    zval_dtor(&retval);
}


static zend_object *createUVTcpResource(zend_class_entry *ce) {
    uv_tcp_ext_t *resource;
    resource = ALLOC_RESOURCE(uv_tcp_ext_t);
    zend_object_std_init(&resource->zo, ce);
    object_properties_init(&resource->zo, ce);    
    resource->zo.handlers = &OBJECT_HANDLER(UVTcp);
    return &resource->zo;
}

static void freeUVTcpResource(zend_object *object) {
    uv_tcp_ext_t *resource;
    resource = FETCH_RESOURCE(object, uv_tcp_ext_t);
    releaseResource(resource);
    releaseUVTcpFunctionCache(resource);
    zend_object_std_dtor(object);
}

static zend_always_inline void resolveSocket(uv_tcp_ext_t *resource){
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

static zend_always_inline void resolvePeerSocket(uv_tcp_ext_t *resource){
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
    RETURN_STRING(resource->sockAddr);
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
    RETURN_STRING(resource->peerAddr);
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
    const char *host;
    int host_len;
    char cstr_host[30];
    struct sockaddr_in addr;
    uv_tcp_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_tcp_ext_t);
    uv_tcp_ext_t *client_resource;
    
    object_init_ex(return_value, CLASS_ENTRY(UVTcp));
    if(!make_accepted_uv_tcp_object(resource, return_value)){
        RETURN_FALSE;
    }
    
    client_resource = FETCH_OBJECT_RESOURCE(return_value, uv_tcp_ext_t);
    if(uv_read_start((uv_stream_t *) client_resource, alloc_cb, (uv_read_cb) read_cb)){
        RETURN_FALSE;
    }
}

PHP_METHOD(UVTcp, listen){
    long ret, port;
    zval *self = getThis();
    const char *host = NULL;
    size_t host_len;
    char cstr_host[30];
    struct sockaddr_in addr;
    
    uv_tcp_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_tcp_ext_t);    

    if(((uv_tcp_ext_t *) &resource->uv_tcp)->flag & UV_TCP_HANDLE_START){
        RETURN_LONG(-1);
    }
    
    FCI_FREE(resource->connectCallback);
    
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "slf", &host, &host_len, &port, FCI_PARSE_PARAMETERS_CC(resource->connectCallback))) {
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

    FCI_ADDREF(resource->connectCallback);
    setSelfReference(resource);
    resource->flag |= UV_TCP_HANDLE_START;    
    RETURN_LONG(ret);
}

PHP_METHOD(UVTcp, setCallback){
    long ret;
    zval *self = getThis();
    uv_tcp_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_tcp_ext_t);

    FCI_FREE(resource->readCallback);
    FCI_FREE(resource->writeCallback);
    FCI_FREE(resource->errorCallback);
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "fff", FCI_PARSE_PARAMETERS_CC(resource->readCallback), FCI_PARSE_PARAMETERS_CC(resource->writeCallback), FCI_PARSE_PARAMETERS_CC(resource->errorCallback))) {
        return;
    }
    
    FCI_ADDREF(resource->readCallback);
    FCI_ADDREF(resource->writeCallback);
    FCI_ADDREF(resource->errorCallback);
    setSelfReference(resource);
    RETURN_LONG(ret);
}

PHP_METHOD(UVTcp, close){
    long ret;
    zval *self = getThis();
    uv_tcp_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_tcp_ext_t);
    tcp_close_socket((uv_tcp_ext_t *) &resource->uv_tcp);
}

PHP_METHOD(UVTcp, __construct){
    zval *loop = NULL;
    zval *self = getThis();
    uv_tcp_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_tcp_ext_t);
   
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "|z", &loop)) {
        return;
    }
    
    ZVAL_COPY_VALUE(&resource->object, self);
    
    if(NULL == loop || ZVAL_IS_NULL(loop)){
        uv_tcp_init(uv_default_loop(), (uv_tcp_t *) resource);
        return;
    }
    
    zend_update_property(CLASS_ENTRY(UVTcp), self, ZEND_STRL("loop"), loop);
    uv_tcp_init(FETCH_UV_LOOP(), (uv_tcp_t *) resource);
}

PHP_METHOD(UVTcp, write){
    long ret;
    zval *self = getThis();
    const char *buf = NULL;
    size_t buf_len;
    uv_tcp_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_tcp_ext_t);

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "s", &buf, &buf_len)) {
        return;
    }
    resource->flag |= UV_TCP_WRITE_CALLBACK_ENABLE;
    ret = tcp_write_raw((uv_stream_t *) &resource->uv_tcp, buf, buf_len);
    RETURN_LONG(ret);
}

int tcp_write_raw(uv_stream_t * handle, const char *message, int size) {
    write_req_t *req;
    req = emalloc(sizeof(write_req_t));
    req->buf.base = emalloc(size);
    req->buf.len = size;
    memcpy(req->buf.base, message, size);
    return uv_write((uv_write_t *) req, handle, &req->buf, 1, write_cb);
}

PHP_METHOD(UVTcp, shutdown){
    long ret;
    zval *self = getThis();
    char *buf;
    int buf_len;
    uv_tcp_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_tcp_ext_t);

    FCI_FREE(resource->shutdownCallback);
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "f", FCI_PARSE_PARAMETERS_CC(resource->shutdownCallback))) {
        return;
    }
    FCI_ADDREF(resource->shutdownCallback);
    
    if(ret = uv_shutdown(&resource->shutdown_req, (uv_stream_t *) &resource->uv_tcp, shutdown_cb)){
        RETURN_LONG(ret);
    }
    
    setSelfReference(resource);
    RETURN_LONG(ret);
}

PHP_METHOD(UVTcp, connect){
    long ret, port;
    zval *self = getThis();
    const char *host = NULL;
    size_t host_len;
    char cstr_host[30];
    struct sockaddr_in addr;
    
    uv_tcp_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_tcp_ext_t);

    FCI_FREE(resource->connectCallback);
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "slf", &host, &host_len, &port, FCI_PARSE_PARAMETERS_CC(resource->connectCallback))) {
        return;
    }
    FCI_ADDREF(resource->connectCallback);
    
    if(host_len == 0 || host_len >= 30){
        FCI_FREE(resource->connectCallback);
        RETURN_LONG(-1);
    }

    memcpy(cstr_host, host, host_len);
    cstr_host[host_len] = '\0';
    if((ret = uv_ip4_addr(cstr_host, port&0xffff, &addr)) != 0){
        FCI_FREE(resource->connectCallback);
        RETURN_LONG(ret);
    }
    
    if((ret = uv_tcp_connect(&resource->connect_req, &resource->uv_tcp, (const struct sockaddr *) &addr, client_connection_cb)) != 0){
        FCI_FREE(resource->connectCallback);
        RETURN_LONG(ret);
    }
    
    setSelfReference(resource);
    resource->flag |= UV_TCP_HANDLE_START;    
    RETURN_LONG(ret);
}

zend_bool make_accepted_uv_tcp_object(uv_tcp_ext_t *server_resource, zval *client){
    zval rv;
    uv_tcp_ext_t *client_resource;
    client_resource = FETCH_OBJECT_RESOURCE(client, uv_tcp_ext_t);
    zval *loop = zend_read_property(CLASS_ENTRY(UVTcp), &server_resource->object, ZEND_STRL("loop"), 1, &rv);
    zend_update_property(CLASS_ENTRY(UVTcp), client, ZEND_STRL("loop"), loop);
    uv_tcp_init(ZVAL_IS_NULL(loop)?uv_default_loop():FETCH_UV_LOOP(), (uv_tcp_t *) client_resource);
    
    if(uv_accept((uv_stream_t *) &server_resource->uv_tcp, (uv_stream_t *) &client_resource->uv_tcp)) {
        zval_dtor(client);
        return 0;
    }
    
    client_resource->flag |= (UV_TCP_HANDLE_START|UV_TCP_READ_START);
    ZVAL_COPY_VALUE(&client_resource->object, client);
    return 1;
}
