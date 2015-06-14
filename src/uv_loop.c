#include "uv_loop.h"

CLASS_ENTRY_FUNCTION_D(UVLoop){
    REGISTER_CLASS(UVLoop);
    zend_declare_class_constant_long(CLASS_ENTRY(UVLoop), ZEND_STRL("RUN_DEFAULT"), RUN_DEFAULT TSRMLS_CC);
    zend_declare_class_constant_long(CLASS_ENTRY(UVLoop), ZEND_STRL("RUN_ONCE"), RUN_ONCE TSRMLS_CC);
    zend_declare_class_constant_long(CLASS_ENTRY(UVLoop), ZEND_STRL("RUN_NOWAIT"), RUN_NOWAIT TSRMLS_CC);
    zend_declare_property_null(CLASS_ENTRY(UVLoop), ZEND_STRL("loop"), ZEND_ACC_PRIVATE|ZEND_ACC_STATIC TSRMLS_CC);
}

PHP_METHOD(UVLoop, __construct){

}

PHP_METHOD(UVLoop, defaultLoop){    
    zval *instance = zend_read_static_property(CLASS_ENTRY(UVLoop), ZEND_STRL("loop"), 1 TSRMLS_CC);
    if(ZVAL_IS_NULL(instance)){
        instance = NULL;
        MAKE_STD_ZVAL(instance);
        object_init_ex(instance, CLASS_ENTRY(UVLoop));
        zend_update_static_property(CLASS_ENTRY(UVLoop), ZEND_STRL("loop"), instance TSRMLS_CC);
    }
    RETURN_ZVAL(instance, 1, 0);
}

PHP_METHOD(UVLoop, run){
    long option = RUN_DEFAULT;
    uv_run_mode mode;
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
    uv_run(uv_default_loop(), mode);
}

PHP_METHOD(UVLoop, stop) {
    uv_stop(uv_default_loop());
}

PHP_METHOD(UVLoop, alive) {
    RETURN_LONG(uv_loop_alive(uv_default_loop()));
}

PHP_METHOD(UVLoop, updateTime) {
    uv_update_time(uv_default_loop());
}

PHP_METHOD(UVLoop, now) {
    RETURN_LONG(uv_now(uv_default_loop()));
}

PHP_METHOD(UVLoop, backendFd) {
    RETURN_LONG(uv_backend_fd(uv_default_loop()));
}

PHP_METHOD(UVLoop, backendTimeout) {
    RETURN_LONG(uv_backend_timeout(uv_default_loop()));
}


