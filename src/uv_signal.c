#include "uv_signal.h"

CLASS_ENTRY_FUNCTION_D(UVSignal){
    REGISTER_CLASS_WITH_OBJECT_NEW(UVSignal, createUVSignalResource);
    OBJECT_HANDLER(UVSignal).clone_obj = NULL;
    zend_declare_property_null(CLASS_ENTRY(UVSignal), ZEND_STRL("loop"), ZEND_ACC_PRIVATE TSRMLS_CC);
}

static void signal_handle_callback(uv_signal_ext_t *signal_handle, int signo){
    zval retval;
    zval *params[] = {signal_handle->object, NULL};
    TSRMLS_FETCH();
    ZVAL_NULL(&retval);
    MAKE_STD_ZVAL(params[1]);
    ZVAL_LONG(params[1], signo);
    fci_call_function(&signal_handle->callback, &retval, 2, params TSRMLS_CC);
    zval_ptr_dtor(&params[1]);    
    zval_dtor(&retval);
}

static zend_object_value createUVSignalResource(zend_class_entry *ce TSRMLS_DC) {
    zend_object_value retval;
    uv_signal_ext_t *resource;
    resource = (uv_signal_ext_t *) ecalloc(1, sizeof(uv_signal_ext_t));

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
        zval_ptr_dtor(&resource->object);
    }    
    uv_unref((uv_handle_t *) resource);
    freeFunctionCache(&resource->callback TSRMLS_CC);
    zend_object_std_dtor(&resource->zo TSRMLS_CC);
    efree(resource);
}

PHP_METHOD(UVSignal, __construct){
    zval *loop = NULL;
    zval *self = getThis();
    uv_signal_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_signal_ext_t);
                    
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|z", &loop)) {
        return;
    }

    if(NULL == loop || ZVAL_IS_NULL(loop)){
        uv_signal_init(uv_default_loop(), (uv_signal_t *) resource);
        return;
    }
    
    zend_update_property(CLASS_ENTRY(UVSignal), self, ZEND_STRL("loop"), loop TSRMLS_CC);
    uv_signal_init(FETCH_UV_LOOP(), (uv_signal_t *) resource);
                                                                                      
}

PHP_METHOD(UVSignal, start){
    zval *signal_cb;
    long signo, ret;
    zval *self = getThis();
    uv_signal_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_signal_ext_t);
    
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zl", &signal_cb, &signo)) {
        return;
    }
    
    ret = uv_signal_start((uv_signal_t *) resource, (uv_signal_cb) signal_handle_callback, signo);
    if(ret == 0){
        resource->start = 1;
        resource->object = self;
        Z_ADDREF_P(resource->object);
        registerFunctionCache(&resource->callback, signal_cb TSRMLS_CC);
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
        zval_ptr_dtor(&resource->object);
    }
    RETURN_LONG(ret);
}
