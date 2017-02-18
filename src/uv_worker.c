#include "uv_worker.h"

CLASS_ENTRY_FUNCTION_D(UVWorker){
    REGISTER_CLASS_WITH_OBJECT_NEW(UVWorker, createUVWorkerResource);
    OBJECT_HANDLER(UVWorker).offset = XtOffsetOf(uv_worker_ext_t, zo);
    OBJECT_HANDLER(UVWorker).clone_obj = NULL;
    OBJECT_HANDLER(UVWorker).free_obj = freeUVWorkerResource;
    OBJECT_HANDLER(UVWorker).get_gc = get_gc_UVWorkerResource;
    zend_declare_property_null(CLASS_ENTRY(UVWorker), ZEND_STRL("loop"), ZEND_ACC_PRIVATE);
}

static zend_object *createUVWorkerResource(zend_class_entry *ce) {
    uv_worker_ext_t *resource;
    resource = ALLOC_RESOURCE(uv_worker_ext_t);
    zend_object_std_init(&resource->zo, ce);
    object_properties_init(&resource->zo, ce);    
    resource->zo.handlers = &OBJECT_HANDLER(UVWorker);
    return &resource->zo;
}

static HashTable *get_gc_UVWorkerResource(zval *obj, zval **table, int *n) {
    uv_worker_ext_t *resource;
    resource = FETCH_OBJECT_RESOURCE(obj, uv_worker_ext_t);
    FCI_GC_TABLE(resource, closeCallback);
    *table = (zval *) &resource->gc_table;
    *n = FCI_GC_TABLE_SIZE(resource->gc_table);
    return zend_std_get_properties(obj);
}

void freeUVWorkerResource(zend_object *object) {
    uv_worker_ext_t *resource;
    resource = FETCH_RESOURCE(object, uv_worker_ext_t);
    freeFunctionCache(&resource->closeCallback);
    zend_object_std_dtor(&resource->zo);
}

PHP_METHOD(UVWorker, __construct){
}

PHP_METHOD(UVWorker, kill){
    long signum;
    zval *self = getThis();
    uv_worker_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_worker_ext_t);
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "l", &signum)) {
        return;
    }
    
    RETURN_LONG(uv_process_kill(&resource->process, signum));
}

PHP_METHOD(UVWorker, getPid){
    zval *self = getThis();
    uv_worker_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_worker_ext_t);    
    RETURN_LONG(resource->process.pid);
}

int make_worker(uv_loop_t *loop, char *args[], zval *worker) {
    int retval;
    uv_stdio_container_t child_stdio[3];
    uv_worker_ext_t *resource = FETCH_OBJECT_RESOURCE(worker, uv_worker_ext_t);
    ZVAL_COPY_VALUE(&resource->object, worker);
    uv_pipe_init(loop, &resource->pipe, 1);
    child_stdio[0].flags = UV_CREATE_PIPE | UV_READABLE_PIPE;
    child_stdio[0].data.stream = (uv_stream_t*) &resource->pipe;
    child_stdio[1].flags = UV_IGNORE;
    child_stdio[2].flags =  UV_INHERIT_FD;
    child_stdio[2].data.fd = 2;
    
    resource->options.stdio = child_stdio;
    resource->options.stdio_count = 3;
    
    resource->options.exit_cb = close_process_handle;
    resource->options.file = args[0];
    resource->options.args = args;
    
    if((retval = uv_spawn(loop, &resource->process, &resource->options))==0){
        Z_ADDREF(resource->object);
    }
    return retval;
}

static void close_process_handle(uv_process_t *process, int64_t exit_status, int term_signal) {
    uv_worker_ext_t *resource = (uv_worker_ext_t *) process;    
    printf("process end %d\n", resource->process.pid);
    zval retval;
    zval params[3];
    params[0] = resource->object;
    if(!FCI_ISNULL(resource->closeCallback)){
        ZVAL_LONG(&params[1], exit_status);
        ZVAL_LONG(&params[2], term_signal);
        fci_call_function(&resource->closeCallback, &retval, 3, params);
        zval_ptr_dtor(&retval);
    }
    uv_close((uv_handle_t*) process, NULL);
    zval_ptr_dtor(&resource->object);
}

PHP_METHOD(UVWorker, setCloseCallback){
    long ret;
    zval *self = getThis();
    uv_worker_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_worker_ext_t);
    
    FCI_FREE(resource->closeCallback);
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "f", FCI_PARSE_PARAMETERS_CC(resource->closeCallback))) {
        return;
    }
    FCI_ADDREF(resource->closeCallback);
}

PHP_METHOD(UVWorker, attach){
    zval *self = getThis();
    zval *stream = NULL;
    uv_worker_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_worker_ext_t);
    uv_tcp_ext_t *stream_resource;    
    uv_write_t *write_req;
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "z", &stream)) {
        return;
    }
    stream_resource = FETCH_OBJECT_RESOURCE(stream, uv_tcp_ext_t);
    resource->dummy_buf = uv_buf_init("a", 1);
    write_req = (uv_write_t*) emalloc(sizeof(uv_write_t));
    uv_write2(write_req, (uv_stream_t*) &resource->pipe, &resource->dummy_buf, 1, (uv_stream_t*) &stream_resource->uv_tcp, NULL);
}
