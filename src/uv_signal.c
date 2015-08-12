#include "uv_signal.h"

CLASS_ENTRY_FUNCTION_D(UVSignal){
    REGISTER_CLASS_WITH_OBJECT_NEW(UVSignal, createUVSignalResource);
    OBJECT_HANDLER(UVSignal).clone_obj = NULL;
    zend_declare_property_null(CLASS_ENTRY(UVSignal), ZEND_STRL("loop"), ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_null(CLASS_ENTRY(UVSignal), ZEND_STRL("callback"), ZEND_ACC_PRIVATE TSRMLS_CC);
}

static void signal_handle_callback(uv_signal_ext_t *signal_handle, int signo){
    zval *signal_cb;
    zval retval;
    zval *params[] = {signal_handle->object, NULL};
    TSRMLS_FETCH();
    ZVAL_NULL(&retval);
    MAKE_STD_ZVAL(params[1]);
    ZVAL_LONG(params[1], signo);
    signal_cb = zend_read_property(CLASS_ENTRY(UVSignal), signal_handle->object, ZEND_STRL("callback"), 0 TSRMLS_CC);
    call_user_function(CG(function_table), NULL, signal_cb, &retval, 2, params TSRMLS_CC);
    zval_dtor(params[1]);    
    zval_dtor(&retval);
}

static zend_object_value createUVSignalResource(zend_class_entry *ce TSRMLS_DC) {
    zend_object_value retval;
    uv_signal_ext_t *resource;
    resource = (uv_signal_ext_t *) emalloc(sizeof(uv_signal_ext_t));
    memset(resource, 0, sizeof(uv_signal_ext_t));

    zend_object_std_init(&resource->zo, ce TSRMLS_CC);
    object_properties_init(&resource->zo, ce);
    
    retval.handle = zend_objects_store_put(
        &resource->zo,
        (zend_objects_store_dtor_t) zend_objects_destroy_object,
        freeUVSignalResource,
        NULL TSRMLS_CC);

    retval.handlers = &OBJECT_HANDLER(UVSignal);
    return retval;
}

void freeUVSignalResource(void *object TSRMLS_DC) {
    uv_signal_ext_t *resource;
    resource = FETCH_RESOURCE(object, uv_signal_ext_t);
    if(resource->start){
        uv_signal_stop((uv_signal_t *) resource);
    }
    
    uv_unref((uv_handle_t *) resource);
    zend_object_std_dtor(&resource->zo TSRMLS_CC);
    efree(resource);
}

PHP_METHOD(UVSignal, __construct){
    zval *loop;
    zval *self = getThis();
    uv_signal_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_signal_ext_t);
                    
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &loop)) {
        return;
    }
                                         
    if (IS_OBJECT != Z_TYPE_P(loop) ||
        !instanceof_function(Z_OBJCE_P(loop), CLASS_ENTRY(UVLoop) TSRMLS_CC)) {
        php_error_docref(NULL TSRMLS_CC, E_RECOVERABLE_ERROR, "$loop must be an instanceof UVLoop.");
        return;
    }
    zend_update_property(CLASS_ENTRY(UVSignal), self, ZEND_STRL("loop"), loop TSRMLS_CC);
    uv_signal_init((uv_loop_t *) FETCH_OBJECT_RESOURCE(loop, uv_loop_ext_t), (uv_signal_t *) resource);
                                                                                      
}

PHP_METHOD(UVSignal, start){
    zval *signal_cb;
    long signo, ret;
    zval *self = getThis();
    uv_signal_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_signal_ext_t);
    
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zl", &signal_cb, &signo)) {
        return;
    }
    
    if (!zend_is_callable(signal_cb, 0, NULL TSRMLS_CC)) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "param signal_cb is not callable");
    }
    
    ret = uv_signal_start((uv_signal_t *) resource, (uv_signal_cb) signal_handle_callback, signo);
    if(ret == 0){
        zend_update_property(CLASS_ENTRY(UVSignal), self, ZEND_STRL("callback"), signal_cb TSRMLS_CC);
        resource->start = 1;
        resource->object = self;
        Z_ADDREF_P(resource->object);
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
        Z_DELREF_P(resource->object);
    }
    RETURN_LONG(ret);
}