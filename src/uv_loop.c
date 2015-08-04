#include "uv_loop.h"

CLASS_ENTRY_FUNCTION_D(UVLoop){
    REGISTER_CLASS_WITH_OBJECT_NEW(UVLoop, createUVLoopResource);
    OBJECT_HANDLER(UVLoop).clone_obj = NULL;
    zend_declare_class_constant_long(CLASS_ENTRY(UVLoop), ZEND_STRL("RUN_DEFAULT"), RUN_DEFAULT TSRMLS_CC);
    zend_declare_class_constant_long(CLASS_ENTRY(UVLoop), ZEND_STRL("RUN_ONCE"), RUN_ONCE TSRMLS_CC);
    zend_declare_class_constant_long(CLASS_ENTRY(UVLoop), ZEND_STRL("RUN_NOWAIT"), RUN_NOWAIT TSRMLS_CC);
}

static zend_object_value createUVLoopResource(zend_class_entry *ce TSRMLS_DC) {
    zend_object_value retval;
    uv_loop_ext_t *resource;
    resource = (uv_loop_ext_t *) emalloc(sizeof(uv_loop_ext_t));
    memset(resource, 0, sizeof(uv_loop_ext_t));
    uv_loop_init(&resource->loop);

    zend_object_std_init(&resource->zo, ce TSRMLS_CC);
    object_properties_init(&resource->zo, ce);
    retval.handle = zend_objects_store_put(
        &resource->zo,
        (zend_objects_store_dtor_t) zend_objects_destroy_object,
        freeUVLoopResource,
        NULL TSRMLS_CC);
                                    
    retval.handlers = &OBJECT_HANDLER(UVLoop);
    return retval;
}

void freeUVLoopResource(void *object TSRMLS_DC) {
    uv_loop_ext_t *resource;
    resource = FETCH_RESOURCE(object, uv_loop_ext_t);
    uv_loop_close(&resource->loop);
    zend_object_std_dtor(&resource->zo TSRMLS_CC);
    efree(resource);
}

PHP_METHOD(UVLoop, run){
    long option = RUN_DEFAULT;
    uv_run_mode mode;
    zval *self = getThis();
    uv_loop_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_loop_ext_t);
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l", &option) == FAILURE) {
        return;
    }
    switch(option){
        case RUN_DEFAULT:
            mode = UV_RUN_DEFAULT;
            break;
        case RUN_ONCE:
            mode = UV_RUN_ONCE;
            break;
        case RUN_NOWAIT:
            mode = UV_RUN_NOWAIT;
            break;
        default:
            mode = UV_RUN_DEFAULT;        
    }
    uv_run(&resource->loop, mode);
}

PHP_METHOD(UVLoop, stop) {
    zval *self = getThis();
    uv_loop_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_loop_ext_t);
    uv_stop(&resource->loop);
}

PHP_METHOD(UVLoop, alive) {
    zval *self = getThis();
    uv_loop_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_loop_ext_t);
    RETURN_LONG(uv_loop_alive(&resource->loop));
}

PHP_METHOD(UVLoop, updateTime) {
    zval *self = getThis();
    uv_loop_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_loop_ext_t);
    uv_update_time(&resource->loop);
}

PHP_METHOD(UVLoop, now) {
    zval *self = getThis();
    uv_loop_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_loop_ext_t);
    RETURN_LONG(uv_now(&resource->loop));
}

PHP_METHOD(UVLoop, backendFd) {
    zval *self = getThis();
    uv_loop_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_loop_ext_t);
    RETURN_LONG(uv_backend_fd(&resource->loop));
}

PHP_METHOD(UVLoop, backendTimeout) {
    zval *self = getThis();
    uv_loop_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_loop_ext_t);
    RETURN_LONG(uv_backend_timeout(&resource->loop));
}


