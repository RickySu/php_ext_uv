#include "uv_signal.h"

CLASS_ENTRY_FUNCTION_D(UVSignal){
    REGISTER_CLASS_WITH_OBJECT_NEW(UVSignal, createUVSignalResource);
    OBJECT_HANDLER(UVSignal).offset = XtOffsetOf(uv_signal_ext_t, zo);
    OBJECT_HANDLER(UVSignal).clone_obj = NULL;
    OBJECT_HANDLER(UVSignal).free_obj = freeUVSignalResource;
    OBJECT_HANDLER(UVIdle).get_gc = get_gc_UVSignalResource;
    zend_declare_property_null(CLASS_ENTRY(UVSignal), ZEND_STRL("loop"), ZEND_ACC_PRIVATE);
}

static HashTable *get_gc_UVSignalResource(zval *obj, zval **table, int *n) {
    uv_signal_ext_t *resource;
    resource = FETCH_OBJECT_RESOURCE(obj, uv_signal_ext_t);
    FCI_GC_TABLE(resource, callback);
    *table = (zval *) &resource->gc_table;
    *n = FCI_GC_TABLE_SIZE(resource->gc_table);
    return zend_std_get_properties(obj);
}

static void signal_handle_callback(uv_signal_ext_t *signal_handle, int signo){
    zval retval;
    zval params[2];
    params[0] = signal_handle->object;
    ZVAL_LONG(&params[1], signo);
    fci_call_function(&signal_handle->callback, &retval, 2, params);
    zval_ptr_dtor(&retval);
}

static zend_object *createUVSignalResource(zend_class_entry *ce) {
    uv_signal_ext_t *resource;
    resource = ALLOC_RESOURCE(uv_signal_ext_t);
    zend_object_std_init(&resource->zo, ce);
    object_properties_init(&resource->zo, ce);
    resource->zo.handlers = &OBJECT_HANDLER(UVSignal);
    return &resource->zo;
}

void freeUVSignalResource(zend_object *object) {
    uv_signal_ext_t *resource;
    resource = FETCH_RESOURCE(object, uv_signal_ext_t);
    if(resource->start){
        uv_signal_stop((uv_signal_t *) resource);
        FCI_FREE(resource->callback);
    }
    uv_unref((uv_handle_t *) resource);
    zend_object_std_dtor(&resource->zo);
}

PHP_METHOD(UVSignal, __construct){
    zval *loop = NULL;
    zval *self = getThis();
    uv_signal_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_signal_ext_t);
                    
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "|z", &loop)) {
        return;
    }

    if(NULL == loop || ZVAL_IS_NULL(loop)){
        uv_signal_init(uv_default_loop(), (uv_signal_t *) resource);
        return;
    }
    
    zend_update_property(CLASS_ENTRY(UVSignal), self, ZEND_STRL("loop"), loop);
    uv_signal_init(FETCH_UV_LOOP(), (uv_signal_t *) resource);                                                                                  
}

PHP_METHOD(UVSignal, start){
    zval *signal_cb;
    long signo, ret;
    zval *self = getThis();
    uv_signal_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_signal_ext_t);
    
    FCI_FREE(resource->callback);
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "fl", FCI_PARSE_PARAMETERS_CC(resource->callback), &signo)) {
        return;
    }
    Z_ADDREF(resource->callback.fci.function_name);

    if(ret = uv_signal_start((uv_signal_t *) resource, (uv_signal_cb) signal_handle_callback, signo)){
        FCI_FREE(resource->callback);
    }
    resource->start = 1;
    ZVAL_COPY_VALUE(&resource->object, self);
    RETURN_LONG(ret);
}

PHP_METHOD(UVSignal, stop){
    long ret;
    zval *self = getThis();
    uv_signal_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_signal_ext_t);
    
    if(!resource->start){
        RETURN_LONG(-1);
    }
    
    ret = uv_signal_stop((uv_signal_t *) resource);
    if(ret == 0){
        resource->start = 0;
        FCI_FREE(resource->callback);
        zval_dtor(&resource->object);
    }
    RETURN_LONG(ret);
}
