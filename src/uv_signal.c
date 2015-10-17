#include "uv_signal.h"

CLASS_ENTRY_FUNCTION_D(UVSignal){
    REGISTER_CLASS_WITH_OBJECT_NEW(UVSignal, createUVSignalResource);
    OBJECT_HANDLER(UVSignal).offset = XtOffsetOf(uv_signal_ext_t, zo);
    OBJECT_HANDLER(UVSignal).clone_obj = NULL;
    OBJECT_HANDLER(UVSignal).free_obj = freeUVSignalResource;
    zend_declare_property_null(CLASS_ENTRY(UVSignal), ZEND_STRL("loop"), ZEND_ACC_PRIVATE);
    zend_declare_property_null(CLASS_ENTRY(UVSignal), ZEND_STRL("callback"), ZEND_ACC_PRIVATE);
}

static void signal_handle_callback(uv_signal_ext_t *signal_handle, int signo){
    zval *signal_cb;
    zval retval, rv;
    zval params[2];
    params[0] = signal_handle->object;
    ZVAL_NULL(&retval);
    ZVAL_LONG(&params[1], signo);
    signal_cb = zend_read_property(CLASS_ENTRY(UVSignal), &signal_handle->object, ZEND_STRL("callback"), 1, &rv);
    call_user_function(CG(function_table), NULL, signal_cb, &retval, 2, params);
    zval_ptr_dtor(&params[1]);    
    zval_ptr_dtor(&retval);
}

static zend_object *createUVSignalResource(zend_class_entry *ce) {
    uv_signal_ext_t *resource;
    resource = resource = ALLOC_RESOURCE(uv_signal_ext_t);;

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
    }
    
    uv_unref((uv_handle_t *) resource);
    zend_object_std_dtor(&resource->zo);
    efree(resource);
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
    
    if(!check_zval_type(CLASS_ENTRY(UVSignal), ZEND_STRL("__construct") + 1, CLASS_ENTRY(UVLoop), loop)){
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
    
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "zl", &signal_cb, &signo)) {
        return;
    }
    
    if (!zend_is_callable(signal_cb, 0, NULL)) {
        php_error_docref(NULL, E_WARNING, "param signal_cb is not callable");
    }
    
    ret = uv_signal_start((uv_signal_t *) resource, (uv_signal_cb) signal_handle_callback, signo);
    if(ret == 0){
        zend_update_property(CLASS_ENTRY(UVSignal), self, ZEND_STRL("callback"), signal_cb);
        resource->start = 1;
        resource->object = *self;
        Z_ADDREF_P(self);
    }
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
        Z_DELREF_P(&resource->object);
    }
    RETURN_LONG(ret);
}