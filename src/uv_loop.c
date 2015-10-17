#include "uv_loop.h"

CLASS_ENTRY_FUNCTION_D(UVLoop){
    REGISTER_CLASS_WITH_OBJECT_NEW(UVLoop, createUVLoopResource);
    OBJECT_HANDLER(UVLoop).offset = XtOffsetOf(uv_loop_ext_t, zo);
    OBJECT_HANDLER(UVLoop).clone_obj = NULL;
    OBJECT_HANDLER(UVLoop).free_obj = freeUVLoopResource;
    REGISTER_CLASS_CONSTANT_LONG(UVLoop, RUN_DEFAULT);
    REGISTER_CLASS_CONSTANT_LONG(UVLoop, RUN_ONCE);
    REGISTER_CLASS_CONSTANT_LONG(UVLoop, RUN_NOWAIT);
    zend_declare_property_null(CLASS_ENTRY(UVLoop), ZEND_STRL("loop"), ZEND_ACC_PRIVATE|ZEND_ACC_STATIC);
}

static zend_object *createUVLoopResource(zend_class_entry *ce) {
    uv_loop_ext_t *resource;
    resource = ALLOC_RESOURCE(uv_loop_ext_t);
    resource->loop = (uv_loop_t *) resource;
    uv_loop_init(resource->loop);
    zend_object_std_init(&resource->zo, ce);
    object_properties_init(&resource->zo, ce);
    resource->zo.handlers = &OBJECT_HANDLER(UVLoop);
    return &resource->zo;
}

void freeUVLoopResource(zend_object *object) {
    uv_loop_ext_t *resource;
    resource = FETCH_RESOURCE(object, uv_loop_ext_t);
    if((uv_loop_t *)resource == resource->loop){
        uv_loop_close((uv_loop_t *) resource);
    }
    zend_object_std_dtor(&resource->zo);
    efree(resource);
}

PHP_METHOD(UVLoop, run){
    long option = RUN_DEFAULT;
    uv_run_mode mode;
    zval *self = getThis();
    uv_loop_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_loop_ext_t);
    if(zend_parse_parameters(ZEND_NUM_ARGS(), "|l", &option) == FAILURE) {
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
    uv_run(resource->loop, mode);
}

PHP_METHOD(UVLoop, stop) {
    zval *self = getThis();
    uv_loop_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_loop_ext_t);
    uv_stop(resource->loop);
}

PHP_METHOD(UVLoop, alive) {
    zval *self = getThis();
    uv_loop_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_loop_ext_t);
    RETURN_LONG(uv_loop_alive(resource->loop));
}

PHP_METHOD(UVLoop, updateTime) {
    zval *self = getThis();
    uv_loop_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_loop_ext_t);
    uv_update_time(resource->loop);
}

PHP_METHOD(UVLoop, now) {
    zval *self = getThis();
    uv_loop_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_loop_ext_t);
    RETURN_LONG(uv_now(resource->loop));
}

PHP_METHOD(UVLoop, backendFd) {
    zval *self = getThis();
    uv_loop_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_loop_ext_t);
    RETURN_LONG(uv_backend_fd(resource->loop));
}

PHP_METHOD(UVLoop, backendTimeout) {
    zval *self = getThis();
    uv_loop_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_loop_ext_t);
    RETURN_LONG(uv_backend_timeout(resource->loop));
}

PHP_METHOD(UVLoop, defaultLoop) {
    zval *instance = zend_read_static_property(CLASS_ENTRY(UVLoop), ZEND_STRL("loop"), 1);
    if(ZVAL_IS_NULL(instance)) {
        instance = NULL;
        MAKE_STD_ZVAL(instance);
        object_init_ex(instance, CLASS_ENTRY(UVLoop));
        zend_update_static_property(CLASS_ENTRY(UVLoop), ZEND_STRL("loop"), instance);
        uv_loop_ext_t *resource = FETCH_OBJECT_RESOURCE(instance, uv_loop_ext_t);
        resource->loop = uv_default_loop();
    }
    RETURN_ZVAL(instance, 1, 0);
}
