#include "uv_timer.h"

CLASS_ENTRY_FUNCTION_D(UVTimer){
    REGISTER_CLASS_WITH_OBJECT_NEW(UVTimer, createUVTimerResource);
    OBJECT_HANDLER(UVTimer).offset = XtOffsetOf(uv_timer_ext_t, zo);
    OBJECT_HANDLER(UVTimer).clone_obj = NULL;
    OBJECT_HANDLER(UVTimer).free_obj = freeUVTimerResource;
    zend_declare_property_null(CLASS_ENTRY(UVTimer), ZEND_STRL("loop"), ZEND_ACC_PRIVATE);
    zend_declare_property_null(CLASS_ENTRY(UVTimer), ZEND_STRL("callback"), ZEND_ACC_PRIVATE);
}

static void timer_handle_callback(uv_timer_ext_t *timer_handle){
    zval *timer_cb;
    zval retval, rv;
    timer_cb = zend_read_property(CLASS_ENTRY(UVTimer), &timer_handle->object, ZEND_STRL("callback"), 1, &rv);
    call_user_function(CG(function_table), NULL, timer_cb, &retval, 1, &timer_handle->object);
    zval_dtor(&retval);
}

static zend_object *createUVTimerResource(zend_class_entry *ce) {
    uv_timer_ext_t *resource;
    resource = ALLOC_RESOURCE(uv_timer_ext_t);

    zend_object_std_init(&resource->zo, ce);
    object_properties_init(&resource->zo, ce);

    resource->zo.handlers = &OBJECT_HANDLER(UVTimer);
    return &resource->zo;
}

void freeUVTimerResource(zend_object *object) {
    uv_timer_ext_t *resource;
    resource = FETCH_RESOURCE(object, uv_timer_ext_t);
    if(resource->start){
        uv_timer_stop((uv_timer_t *) resource);
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
    
    if (!zend_is_callable(timer_cb, 0, NULL)) {
        php_error_docref(NULL, E_WARNING, "param timer_cb is not callable");
    }
    
    ret = uv_timer_start((uv_timer_t *) resource, (uv_timer_cb) timer_handle_callback, start, repeat);
    if(ret == 0){
        zend_update_property(CLASS_ENTRY(UVTimer), self, ZEND_STRL("callback"), timer_cb);
        resource->start = 1;
        resource->object = *self;
        Z_ADDREF(resource->object);
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
        Z_DELREF(resource->object);
    }
    RETURN_LONG(ret);
}
