#include "uv_idle.h"

CLASS_ENTRY_FUNCTION_D(UVIdle){
    REGISTER_CLASS_WITH_OBJECT_NEW(UVIdle, createUVIdleResource);
    OBJECT_HANDLER(UVIdle).clone_obj = NULL;
    zend_declare_property_null(CLASS_ENTRY(UVIdle), ZEND_STRL("loop"), ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_null(CLASS_ENTRY(UVIdle), ZEND_STRL("callback"), ZEND_ACC_PRIVATE TSRMLS_CC);
}

static void idle_handle_callback(uv_idle_ext_t *idle_handle){
    zval *idle_cb;
    zval retval;
    zval *params[] = {idle_handle->object};
    TSRMLS_FETCH();
    ZVAL_NULL(&retval);
    idle_cb = zend_read_property(CLASS_ENTRY(UVIdle), idle_handle->object, ZEND_STRL("callback"), 0 TSRMLS_CC);
    call_user_function(CG(function_table), NULL, idle_cb, &retval, 1, params TSRMLS_CC);
    zval_dtor(&retval);
}

static zend_object_value createUVIdleResource(zend_class_entry *ce TSRMLS_DC) {
    zend_object_value retval;
    uv_idle_ext_t *resource;
    resource = (uv_idle_ext_t *) emalloc(sizeof(uv_idle_ext_t));
    memset(resource, 0, sizeof(uv_idle_ext_t));

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
    }
    
    uv_unref((uv_handle_t *) resource);
    zend_object_std_dtor(&resource->zo TSRMLS_CC);
    efree(resource);
}

PHP_METHOD(UVIdle, __construct){
    zval *loop;
    zval *self = getThis();
    uv_idle_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_idle_ext_t);
                    
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &loop)) {
        return;
    }
                                         
    if (IS_OBJECT != Z_TYPE_P(loop) ||
        !instanceof_function(Z_OBJCE_P(loop), CLASS_ENTRY(UVLoop) TSRMLS_CC)) {
        php_error_docref(NULL TSRMLS_CC, E_RECOVERABLE_ERROR, "$loop must be an instanceof UVLoop.");
        return;
    }
    zend_update_property(CLASS_ENTRY(UVIdle), self, ZEND_STRL("loop"), loop TSRMLS_CC);
    uv_idle_init((uv_loop_t *) FETCH_OBJECT_RESOURCE(loop, uv_loop_ext_t), (uv_idle_t *) resource);
                                                                                      
}

PHP_METHOD(UVIdle, start){
    zval *idle_cb;
    long ret;
    zval *self = getThis();
    uv_idle_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_idle_ext_t);
    
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &idle_cb)) {
        return;
    }
    
    if (!zend_is_callable(idle_cb, 0, NULL TSRMLS_CC)) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "param idle_cb is not callable");
    }
    
    ret = uv_idle_start((uv_idle_t *) resource, (uv_idle_cb) idle_handle_callback);
    if(ret == 0){
        zend_update_property(CLASS_ENTRY(UVIdle), self, ZEND_STRL("callback"), idle_cb TSRMLS_CC);
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
        Z_DELREF_P(resource->object);
    }
    RETURN_LONG(ret);
}