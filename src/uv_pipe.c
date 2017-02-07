#include "uv_pipe.h"

void pipe_close_socket(uv_pipe_ext_t *handle){       
    if(handle->flag & UV_PIPE_CLOSING_START){
         return;
    }
    handle->flag |= UV_PIPE_CLOSING_START;
    uv_close((uv_handle_t *) handle, pipe_close_cb);
}

void setSelfReference(uv_pipe_ext_t *resource)
{
    if(resource->flag & UV_PIPE_HANDLE_INTERNAL_REF){
        return;
    }
    Z_ADDREF(resource->object);
    resource->flag |= UV_PIPE_HANDLE_INTERNAL_REF;
}

CLASS_ENTRY_FUNCTION_D(UVPipe){
    REGISTER_CLASS_WITH_OBJECT_NEW(UVPipe, createUVPipeResource);
    OBJECT_HANDLER(UVPipe).offset = XtOffsetOf(uv_pipe_ext_t, zo);
    OBJECT_HANDLER(UVPipe).clone_obj = NULL;
    OBJECT_HANDLER(UVPipe).free_obj = freeUVPipeResource;
    OBJECT_HANDLER(UVPipe).get_gc = get_gc_UVPipeResource;
    zend_declare_property_null(CLASS_ENTRY(UVPipe), ZEND_STRL("loop"), ZEND_ACC_PRIVATE);
}

HashTable *get_gc_UVPipeResource(zval *obj, zval **table, int *n) {
    uv_pipe_ext_t *resource = FETCH_OBJECT_RESOURCE(obj, uv_pipe_ext_t);
    *table = &resource->gc_table[0];
    int index = 0;
    FCI_GC_TABLE_EX(resource, readCallback, index);
    FCI_GC_TABLE_EX(resource, writeCallback, index);
    FCI_GC_TABLE_EX(resource, errorCallback, index);
    FCI_GC_TABLE_EX(resource, connectCallback, index);
    FCI_GC_TABLE_EX(resource, shutdownCallback, index);
    *n = index;
    return zend_std_get_properties(obj);
}

void releaseResource(uv_pipe_ext_t *resource){
    if(resource->sockAddr != NULL){
        efree(resource->sockAddr);
        resource->sockAddr = NULL;
    }

    if(resource->peerAddr != NULL){
        efree(resource->peerAddr);
        resource->peerAddr = NULL;
    }

    if(resource->flag & UV_PIPE_READ_START){
        resource->flag &= ~UV_PIPE_READ_START;
        uv_read_stop((uv_stream_t *) &resource->uv_pipe);
    }
    
    if(resource->flag & UV_PIPE_HANDLE_START){
        resource->flag &= ~UV_PIPE_HANDLE_START;
        uv_unref((uv_handle_t *) &resource->uv_pipe);
    }

    if(resource->flag & UV_PIPE_HANDLE_INTERNAL_REF){
        resource->flag &= ~UV_PIPE_HANDLE_INTERNAL_REF;
        zval_ptr_dtor(&resource->object);
    }    
}

static void shutdown_cb(uv_shutdown_t* req, int status) {
    uv_pipe_ext_t *resource = (uv_pipe_ext_t *) req->handle;
    zval retval;
    zval params[2];
    params[0] = resource->object;
    if(!FCI_ISNULL(resource->shutdownCallback)){
        ZVAL_LONG(&params[1], status);
        fci_call_function(&resource->shutdownCallback, &retval, 2, params);
        zval_ptr_dtor(&retval);
    }
}

static void pipe_close_cb(uv_handle_t* handle) {
    releaseResource((uv_pipe_ext_t *) handle);
}

static void write_cb(uv_write_t *wr, int status){
    zval retval;
    write_req_t *req = (write_req_t *) wr;
    uv_pipe_ext_t *resource = (uv_pipe_ext_t *) req->uv_write.handle;
    zval params[3];
    params[0] = resource->object;

    if(resource->flag & UV_PIPE_WRITE_CALLBACK_ENABLE && !FCI_ISNULL(resource->writeCallback)){
        ZVAL_LONG(&params[1], status);
        ZVAL_LONG(&params[2], req->buf.len);    
        fci_call_function(&resource->writeCallback, &retval, 3, params);
        zval_ptr_dtor(&retval);
    }
    efree(req->buf.base);
    efree(req);
}

static void alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
    buf->base = emalloc(suggested_size);
    buf->len = suggested_size;
}

static void read_cb(uv_pipe_ext_t *resource, ssize_t nread, const uv_buf_t* buf) {
    zval retval;
    zval params[2];
    params[0] = resource->object;
    
    if(nread > 0){
        if(!FCI_ISNULL(resource->readCallback)){
            ZVAL_STRINGL(&params[1], buf->base, nread);
            fci_call_function(&resource->readCallback, &retval, 2, params);
            zval_ptr_dtor(&params[1]);
            zval_ptr_dtor(&retval);
        }
    }
    else{    
        if(!FCI_ISNULL(resource->errorCallback)){
            ZVAL_LONG(&params[1], nread);        
            fci_call_function(&resource->errorCallback, &retval, 2, params);
            zval_ptr_dtor(&retval);
        }
        pipe_close_socket((uv_pipe_ext_t *) &resource->uv_pipe);
    }
    efree(buf->base);
}

static void client_connection_cb(uv_connect_t* req, int status) {
    zval retval;
    uv_pipe_ext_t *resource = (uv_pipe_ext_t *) req->handle;
    zval params[2];
    params[0] = resource->object;
    ZVAL_NULL(&retval);
    ZVAL_LONG(&params[1], status);

    if(uv_read_start((uv_stream_t *) resource, alloc_cb, (uv_read_cb) read_cb)){
        return;
    }
    resource->flag |= (UV_PIPE_HANDLE_START|UV_PIPE_READ_START);
    
    fci_call_function(&resource->connectCallback, &retval, 2, params);
    zval_ptr_dtor(&retval);
}

static void connection_cb(uv_pipe_ext_t *resource, int status) {
    zval retval;
    zval params[2];
    params[0] = resource->object;
    ZVAL_NULL(&retval);
    ZVAL_LONG(&params[1], status);
    fci_call_function(&resource->connectCallback, &retval, 2, params);
    zval_ptr_dtor(&retval);
}


static zend_object *createUVPipeResource(zend_class_entry *ce) {
    uv_pipe_ext_t *resource;
    resource = ALLOC_RESOURCE(uv_pipe_ext_t);
    zend_object_std_init(&resource->zo, ce);
    object_properties_init(&resource->zo, ce);    
    resource->zo.handlers = &OBJECT_HANDLER(UVPipe);
    return &resource->zo;
}

static void freeUVPipeResource(zend_object *object) {
    uv_pipe_ext_t *resource;
    resource = FETCH_RESOURCE(object, uv_pipe_ext_t);
    releaseResource(resource);
    releaseUVPipeFunctionCache(resource);
    zend_object_std_dtor(object);
}

static zend_always_inline void resolveSocket(uv_pipe_ext_t *resource){
    size_t addrlen = UV_PIPE_SOCKENAME_SIZE;
    char *addr;
    if(resource->sockAddr == NULL){
        addr = emalloc(addrlen);
        if(uv_pipe_getsockname(&resource->uv_pipe, addr, &addrlen)){
            efree(addr);
            return;
        }
        resource->sockAddr = addr;
    }
}

static zend_always_inline void resolvePeerSocket(uv_pipe_ext_t *resource){
    size_t addrlen = UV_PIPE_SOCKENAME_SIZE;
    char *addr;
    if(resource->peerAddr == NULL){
        addr = emalloc(addrlen);
        if(uv_pipe_getpeername(&resource->uv_pipe, addr, &addrlen)){
            efree(addr);
            return;
        }
        resource->peerAddr = addr;
    }
}

PHP_METHOD(UVPipe, getSockname){
    zval *self = getThis();
    uv_pipe_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_pipe_ext_t);
    resolveSocket(resource);
    if(resource->sockAddr == NULL){
        RETURN_FALSE;
    }
    RETURN_STRING(resource->sockAddr);
}

PHP_METHOD(UVPipe, getPeername){
    zval *self = getThis();
    uv_pipe_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_pipe_ext_t);
    resolvePeerSocket(resource);
    if(resource->peerAddr == NULL){
        RETURN_FALSE;
    }
    RETURN_STRING(resource->peerAddr);
}

PHP_METHOD(UVPipe, accept){
    long ret;
    zval *self = getThis();
    const char *host;
    int host_len;
    char cstr_host[30];
    struct sockaddr_in addr;
    uv_pipe_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_pipe_ext_t);
    uv_pipe_ext_t *client_resource;
    
    object_init_ex(return_value, CLASS_ENTRY(UVPipe));
    if(!make_accepted_uv_pipe_object(resource, return_value)){
        RETURN_FALSE;
    }
    
    client_resource = FETCH_OBJECT_RESOURCE(return_value, uv_pipe_ext_t);
    if(uv_read_start((uv_stream_t *) client_resource, alloc_cb, (uv_read_cb) read_cb)){
        RETURN_FALSE;
    }
}

PHP_METHOD(UVPipe, listen){
    long ret;
    zval *self = getThis();
    const char *host = NULL;
    size_t host_len;
    struct sockaddr_in addr;
    
    uv_pipe_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_pipe_ext_t);    

    if(((uv_pipe_ext_t *) &resource->uv_pipe)->flag & UV_PIPE_HANDLE_START){
        RETURN_LONG(-1);
    }
    
    FCI_FREE(resource->connectCallback);
    
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "sf", &host, &host_len, FCI_PARSE_PARAMETERS_CC(resource->connectCallback))) {
        return;
    }
    
    if(host_len == 0){
        RETURN_LONG(-1);
    }
    
    if((ret = uv_pipe_bind(&resource->uv_pipe, host)) != 0){
        RETURN_LONG(ret);
    }
    
    if((ret = uv_listen((uv_stream_t *) &resource->uv_pipe, SOMAXCONN, (uv_connection_cb) connection_cb)) != 0){
        RETURN_LONG(ret);
    }

    FCI_ADDREF(resource->connectCallback);
    setSelfReference(resource);
    resource->flag |= UV_PIPE_HANDLE_START;    
    RETURN_LONG(ret);
}

PHP_METHOD(UVPipe, setCallback){
    long ret;
    zval *self = getThis();
    uv_pipe_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_pipe_ext_t);

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

PHP_METHOD(UVPipe, close){
    long ret;
    zval *self = getThis();
    uv_pipe_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_pipe_ext_t);
    pipe_close_socket((uv_pipe_ext_t *) &resource->uv_pipe);
}

PHP_METHOD(UVPipe, __construct){
    zval *loop = NULL;
    zval *self = getThis();
    int ipc = 0;
    uv_pipe_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_pipe_ext_t);
   
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "|zb", &loop, &ipc)) {
        return;
    }
    
    ZVAL_COPY_VALUE(&resource->object, self);
        
    if(NULL == loop || ZVAL_IS_NULL(loop)){
        uv_pipe_init(uv_default_loop(), (uv_pipe_t *) resource, ipc);
        return;
    }
    
    zend_update_property(CLASS_ENTRY(UVPipe), self, ZEND_STRL("loop"), loop);
    uv_pipe_init(FETCH_UV_LOOP(), (uv_pipe_t *) resource, ipc);
    resource->ipc = ipc;
}

PHP_METHOD(UVPipe, write){
    long ret;
    zval *self = getThis();
    const char *buf = NULL;
    size_t buf_len;
    uv_pipe_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_pipe_ext_t);

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "s", &buf, &buf_len)) {
        return;
    }
    resource->flag |= UV_PIPE_WRITE_CALLBACK_ENABLE;
    ret = pipe_write_raw((uv_stream_t *) &resource->uv_pipe, buf, buf_len);
    RETURN_LONG(ret);
}

int pipe_write_raw(uv_stream_t * handle, const char *message, int size) {
    write_req_t *req;
    req = emalloc(sizeof(write_req_t));
    req->buf.base = emalloc(size);
    req->buf.len = size;
    memcpy(req->buf.base, message, size);
    return uv_write((uv_write_t *) req, handle, &req->buf, 1, write_cb);
}

PHP_METHOD(UVPipe, shutdown){
    long ret;
    zval *self = getThis();
    char *buf;
    int buf_len;
    uv_pipe_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_pipe_ext_t);

    FCI_FREE(resource->shutdownCallback);
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "f", FCI_PARSE_PARAMETERS_CC(resource->shutdownCallback))) {
        return;
    }
    FCI_ADDREF(resource->shutdownCallback);
    
    if(ret = uv_shutdown(&resource->shutdown_req, (uv_stream_t *) &resource->uv_pipe, shutdown_cb)){
        RETURN_LONG(ret);
    }
    
    setSelfReference(resource);
    RETURN_LONG(ret);
}

PHP_METHOD(UVPipe, connect){
    long ret;
    zval *self = getThis();
    const char *host = NULL;
    size_t host_len;
    
    uv_pipe_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_pipe_ext_t);

    FCI_FREE(resource->connectCallback);
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "sf", &host, &host_len, FCI_PARSE_PARAMETERS_CC(resource->connectCallback))) {
        return;
    }
    FCI_ADDREF(resource->connectCallback);
    
    if(host_len == 0){
        FCI_FREE(resource->connectCallback);
        RETURN_LONG(-1);
    }
    
    uv_pipe_connect(&resource->connect_req, &resource->uv_pipe, host, client_connection_cb);
    
    setSelfReference(resource);
    resource->flag |= UV_PIPE_HANDLE_START;    
    RETURN_LONG(ret);
}

zend_bool make_accepted_uv_pipe_object(uv_pipe_ext_t *server_resource, zval *client){
    zval rv;
    uv_pipe_ext_t *client_resource;
    client_resource = FETCH_OBJECT_RESOURCE(client, uv_pipe_ext_t);
    zval *loop = zend_read_property(CLASS_ENTRY(UVPipe), &server_resource->object, ZEND_STRL("loop"), 1, &rv);
    zend_update_property(CLASS_ENTRY(UVPipe), client, ZEND_STRL("loop"), loop);
    uv_pipe_init(ZVAL_IS_NULL(loop)?uv_default_loop():FETCH_UV_LOOP(), (uv_pipe_t *) client_resource, server_resource->ipc);
    
    if(uv_accept((uv_stream_t *) &server_resource->uv_pipe, (uv_stream_t *) &client_resource->uv_pipe)) {
        zval_ptr_dtor(client);
        return 0;
    }
    
    client_resource->flag |= (UV_PIPE_HANDLE_START|UV_PIPE_READ_START);
    ZVAL_COPY_VALUE(&client_resource->object, client);
    return 1;
}
