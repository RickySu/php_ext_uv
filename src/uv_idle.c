#include "uv_idle.h"

CLASS_ENTRY_FUNCTION_D(UVIdle){
    REGISTER_CLASS_WITH_OBJECT_NEW(UVIdle, createUVIdleResource);
    OBJECT_HANDLER(UVIdle).offset = XtOffsetOf(uv_idle_ext_t, zo);
    OBJECT_HANDLER(UVIdle).clone_obj = NULL;
    OBJECT_HANDLER(UVIdle).free_obj = freeUVIdleResource;
    OBJECT_HANDLER(UVIdle).get_gc = get_gc_UVIdleResource;
    zend_declare_property_null(CLASS_ENTRY(UVIdle), ZEND_STRL("loop"), ZEND_ACC_PRIVATE);
}

static void idle_handle_callback(uv_idle_ext_t *idle_handle){
    zval retval;
    fci_call_function(&idle_handle->callback, &retval, 1, &idle_handle->object);
    zval_dtor(&retval);
}

static zend_object *createUVIdleResource(zend_class_entry *ce) {
    uv_idle_ext_t *resource;
    resource = ALLOC_RESOURCE(uv_idle_ext_t);
    zend_object_std_init(&resource->zo, ce);
    object_properties_init(&resource->zo, ce);    
    resource->zo.handlers = &OBJECT_HANDLER(UVIdle);
    return &resource->zo;
}

static HashTable *get_gc_UVIdleResource(zval *obj, zval **table, int *n) {
    uv_idle_ext_t *resource;
    resource = FETCH_OBJECT_RESOURCE(obj, uv_idle_ext_t);
    FCI_GC_TABLE(resource, callback);
    *table = (zval *) &resource->gc_table;
    *n = FCI_GC_TABLE_SIZE(resource->gc_table);
    return zend_std_get_properties(obj);
}

void freeUVIdleResource(zend_object *object) {
    uv_idle_ext_t *resource;
    resource = FETCH_RESOURCE(object, uv_idle_ext_t);
    if(resource->start){
        uv_idle_stop((uv_idle_t *) resource);
    }
    uv_unref((uv_handle_t *) resource);
    freeFunctionCache(&resource->callback);
    zend_object_std_dtor(&resource->zo);
}

PHP_METHOD(UVIdle, __construct){
    zval *loop = NULL;
    zval *self = getThis();
    zend_function *fptr;
    uv_idle_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_idle_ext_t);
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "|z", &loop)) {
        return;
    }

    if(NULL == loop || ZVAL_IS_NULL(loop)){
        uv_idle_init(uv_default_loop(), (uv_idle_t *) resource);
        return;
    }

    zend_update_property(CLASS_ENTRY(UVIdle), self, ZEND_STRL("loop"), loop);
    uv_idle_init(FETCH_UV_LOOP(), (uv_idle_t *) resource);
}

PHP_METHOD(UVIdle, start){
    long ret;
    zval *self = getThis();
    uv_idle_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_idle_ext_t);
    FCI_FREE(resource->callback);
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "f", FCI_PARSE_PARAMETERS_CC(resource->callback))) {
        return;
    }
    FCI_ADDREF(resource->callback);
    
    if(ret = uv_idle_start((uv_idle_t *) resource, (uv_idle_cb) idle_handle_callback)){
        FCI_FREE(resource->callback);
        RETURN_LONG(ret);
    }
    
    resource->start = 1;
    ZVAL_COPY_VALUE(&resource->object, self);
    RETURN_LONG(ret);
}

PHP_METHOD(UVIdle, stop){
    long ret;
    zval *self = getThis();
    uv_idle_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_idle_ext_t);
    if(!resource->start){
        RETURN_LONG(-1);
    }
    
    ret = uv_idle_stop((uv_idle_t *) resource);
    if(ret == 0){
        resource->start = 0;
        zval_dtor(&resource->object);
    }
    RETURN_LONG(ret);
}
