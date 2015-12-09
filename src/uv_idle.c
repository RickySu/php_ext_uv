#include "uv_idle.h"

CLASS_ENTRY_FUNCTION_D(UVIdle){
    REGISTER_CLASS_WITH_OBJECT_NEW(UVIdle, createUVIdleResource);
    OBJECT_HANDLER(UVIdle).clone_obj = NULL;
    zend_declare_property_null(CLASS_ENTRY(UVIdle), ZEND_STRL("loop"), ZEND_ACC_PRIVATE TSRMLS_CC);
}

static void idle_handle_callback(uv_idle_ext_t *idle_handle){
    zval retval;
    zval *params[] = {idle_handle->object};
    TSRMLS_FETCH();
    ZVAL_NULL(&retval);
    fci_call_function(&idle_handle->callback, &retval, 1, params TSRMLS_CC);
    zval_dtor(&retval);
}

static zend_object_value createUVIdleResource(zend_class_entry *ce TSRMLS_DC) {
    zend_object_value retval;
    uv_idle_ext_t *resource;
    resource = (uv_idle_ext_t *) ecalloc(1, sizeof(uv_idle_ext_t));

    zend_object_std_init(&resource->zo, ce TSRMLS_CC);
    object_properties_init(&resource->zo, ce);
    
    retval.handle = zend_objects_store_put(
        &resource->zo,
        (zend_objects_store_dtor_t) zend_objects_destroy_object,
        freeUVIdleResource,
        NULL TSRMLS_CC);

    retval.handlers = &OBJECT_HANDLER(UVIdle);
    return retval;
}

void freeUVIdleResource(void *object TSRMLS_DC) {
    uv_idle_ext_t *resource;
    resource = FETCH_RESOURCE(object, uv_idle_ext_t);
    if(resource->start){
        uv_idle_stop((uv_idle_t *) resource);
        zval_ptr_dtor(&resource->object);
    }
    uv_unref((uv_handle_t *) resource);
    freeFunctionCache(&resource->callback TSRMLS_CC);
    zend_object_std_dtor(&resource->zo TSRMLS_CC);
    efree(resource);
}

PHP_METHOD(UVIdle, __construct){
    zval *loop = NULL;
    zval *self = getThis();
    zend_function *fptr;
    uv_idle_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_idle_ext_t);
                    
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|z", &loop)) {
        return;
    }

    if(NULL == loop || ZVAL_IS_NULL(loop)){
        uv_idle_init(uv_default_loop(), (uv_idle_t *) resource);
        return;
    }
    
    zend_update_property(CLASS_ENTRY(UVIdle), self, ZEND_STRL("loop"), loop TSRMLS_CC);
    uv_idle_init(FETCH_UV_LOOP(), (uv_idle_t *) resource);
}

PHP_METHOD(UVIdle, start){
    zval *idle_cb;
    long ret;
    zval *self = getThis();
    uv_idle_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_idle_ext_t);
    
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &idle_cb)) {
        return;
    }
    
    ret = uv_idle_start((uv_idle_t *) resource, (uv_idle_cb) idle_handle_callback);
    if(ret == 0){
        registerFunctionCache(&resource->callback, idle_cb TSRMLS_CC);
        resource->start = 1;
        resource->object = self;
        Z_ADDREF_P(resource->object);
    }
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
        zval_ptr_dtor(&resource->object);
    }
    RETURN_LONG(ret);
}