#include "uv_timer.h"

CLASS_ENTRY_FUNCTION_D(UVTimer){
    REGISTER_CLASS_WITH_OBJECT_NEW(UVTimer, createUVTimerResource);
    OBJECT_HANDLER(UVTimer).offset = XtOffsetOf(uv_timer_ext_t, zo);
    OBJECT_HANDLER(UVTimer).clone_obj = NULL;
    OBJECT_HANDLER(UVTimer).free_obj = freeUVTimerResource;
    zend_declare_property_null(CLASS_ENTRY(UVTimer), ZEND_STRL("loop"), ZEND_ACC_PRIVATE);
}

static void timer_handle_callback(uv_timer_ext_t *timer_handle){
    zval retval;
    fci_call_function(&timer_handle->callback, &retval, 1, &timer_handle->object);
    zval_dtor(&retval);
}

static zend_object *createUVTimerResource(zend_class_entry *ce) {
    uv_timer_ext_t *resource;
    resource = ALLOC_RESOURCE(uv_timer_ext_t);

    zend_object_std_init(&resource->zo, ce);
    object_properties_init(&resource->zo, ce);

    resource->zo.handlers = &OBJECT_HANDLER(UVTimer);
    ZVAL_NULL(&resource->callback.func);
    return &resource->zo;
}

void freeUVTimerResource(zend_object *object) {
    uv_timer_ext_t *resource;
    resource = FETCH_RESOURCE(object, uv_timer_ext_t);
    if(resource->start){
        uv_timer_stop((uv_timer_t *) resource);
        freeFunctionCache(&resource->callback);
    }
    uv_unref((uv_handle_t *) resource);

    zend_object_std_dtor(&resource->zo);
    efree(resource);
}

PHP_METHOD(UVTimer, __construct){
    zval *loop = NULL;
    zval *self = getThis();
    uv_timer_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_timer_ext_t);
        
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "|z", &loop)) {
        return;
    }
    
    if(NULL == loop || ZVAL_IS_NULL(loop)){
        uv_timer_init(uv_default_loop(), (uv_timer_t *) resource);
        return;
    }
    
    zend_update_property(CLASS_ENTRY(UVTimer), self, ZEND_STRL("loop"), loop);
    uv_timer_init(FETCH_UV_LOOP(), (uv_timer_t *) resource);
}

PHP_METHOD(UVTimer, start){
    zval *timer_cb;
    long start, repeat = 0, ret;
    zval *self = getThis();
    uv_timer_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_timer_ext_t);
    
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "zl|l", &timer_cb, &start, &repeat)) {
        return;
    }
    
    ret = uv_timer_start((uv_timer_t *) resource, (uv_timer_cb) timer_handle_callback, start, repeat);
    if(ret == 0){
        registerFunctionCache(&resource->callback, timer_cb);
        resource->start = 1;
        ZVAL_COPY(&resource->object, self);
    }
    RETURN_LONG(ret);
}

PHP_METHOD(UVTimer, stop){
    long ret;
    zval *self = getThis();
    uv_timer_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_timer_ext_t);
    
    if(!resource->start){
        RETURN_LONG(-1);
    }
    
    ret = uv_timer_stop((uv_timer_t *) resource);
    if(ret == 0){
        resource->start = 0;
        freeFunctionCache(&resource->callback);
        Z_DELREF(resource->object);
    }
    RETURN_LONG(ret);
}
