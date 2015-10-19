#include "uv_timer.h"

CLASS_ENTRY_FUNCTION_D(UVTimer){
    REGISTER_CLASS_WITH_OBJECT_NEW(UVTimer, createUVTimerResource);
    OBJECT_HANDLER(UVTimer).clone_obj = NULL;
    zend_declare_property_null(CLASS_ENTRY(UVTimer), ZEND_STRL("loop"), ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_null(CLASS_ENTRY(UVTimer), ZEND_STRL("callback"), ZEND_ACC_PRIVATE TSRMLS_CC);
}

static void timer_handle_callback(uv_timer_ext_t *timer_handle){
    zval *timer_cb;
    zval retval;
    zval *params[] = {timer_handle->object};
    TSRMLS_FETCH();
    ZVAL_NULL(&retval);
    timer_cb = zend_read_property(CLASS_ENTRY(UVTimer), timer_handle->object, ZEND_STRL("callback"), 0 TSRMLS_CC);
    call_user_function(CG(function_table), NULL, timer_cb, &retval, 1, params TSRMLS_CC);
    zval_dtor(&retval);
}

static zend_object_value createUVTimerResource(zend_class_entry *ce TSRMLS_DC) {
    zend_object_value retval;
    uv_timer_ext_t *resource;
    resource = (uv_timer_ext_t *) emalloc(sizeof(uv_timer_ext_t));
    memset(resource, 0, sizeof(uv_timer_ext_t));

    zend_object_std_init(&resource->zo, ce TSRMLS_CC);
    object_properties_init(&resource->zo, ce);

    retval.handle = zend_objects_store_put(
        &resource->zo,
        (zend_objects_store_dtor_t) zend_objects_destroy_object,
        freeUVTimerResource,
        NULL TSRMLS_CC);

    retval.handlers = &OBJECT_HANDLER(UVTimer);
    return retval;
}

void freeUVTimerResource(void *object TSRMLS_DC) {
    uv_timer_ext_t *resource;
    resource = FETCH_RESOURCE(object, uv_timer_ext_t);
    if(resource->start){
        uv_timer_stop((uv_timer_t *) resource);
    }
    uv_unref((uv_handle_t *) resource);
    zend_object_std_dtor(&resource->zo TSRMLS_CC);
    efree(resource);
}

PHP_METHOD(UVTimer, __construct){
    zval *loop = NULL;
    zval *self = getThis();
    uv_timer_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_timer_ext_t);
        
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|z", &loop)) {
        return;
    }
    
    if(NULL == loop || ZVAL_IS_NULL(loop)){
        uv_timer_init(uv_default_loop(), (uv_timer_t *) resource);
        return;
    }
    
    zend_update_property(CLASS_ENTRY(UVTimer), self, ZEND_STRL("loop"), loop TSRMLS_CC);
    uv_timer_init(FETCH_UV_LOOP(), (uv_timer_t *) resource);
}

PHP_METHOD(UVTimer, start){
    zval *timer_cb;
    long start, repeat = 0, ret;
    zval *self = getThis();
    uv_timer_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_timer_ext_t);
    
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zl|l", &timer_cb, &start, &repeat)) {
        return;
    }
    
    if (!zend_is_callable(timer_cb, 0, NULL TSRMLS_CC)) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "param timer_cb is not callable");
    }
    
    ret = uv_timer_start((uv_timer_t *) resource, (uv_timer_cb) timer_handle_callback, start, repeat);
    if(ret == 0){
        zend_update_property(CLASS_ENTRY(UVTimer), self, ZEND_STRL("callback"), timer_cb TSRMLS_CC);
        resource->start = 1;
        resource->object = self;
        Z_ADDREF_P(resource->object);
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
        Z_DELREF_P(resource->object);
    }
    RETURN_LONG(ret);
}
