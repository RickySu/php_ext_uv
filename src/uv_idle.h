#ifndef _UV_IDLE_H
#define _UV_IDLE_H
#include "../php_ext_uv.h"
#include "uv_loop_resource.h"
#include "fcall.h"

ZEND_BEGIN_ARG_INFO(ARGINFO(UVIdle, __construct), 0)
    ZEND_ARG_OBJ_INFO(0, loop, UVLoop, 1)
    ZEND_END_ARG_INFO()
    
ZEND_BEGIN_ARG_INFO(ARGINFO(UVIdle, start), 0)
    ZEND_ARG_INFO(0, idle_cb)
ZEND_END_ARG_INFO()

typedef struct uv_idle_ext_s{
    uv_idle_t uv_idle;
    int start;
    fcall_info_t callback;
    zval object;
    zend_object zo;    
} uv_idle_ext_t;

static zend_object *createUVIdleResource(zend_class_entry *class_type);
static void idle_handle_callback(uv_idle_ext_t *idle_handle);

void freeUVIdleResource(zend_object *object);

PHP_METHOD(UVIdle, __construct);
PHP_METHOD(UVIdle, start);
PHP_METHOD(UVIdle, stop);

DECLARE_FUNCTION_ENTRY(UVIdle) = {
    PHP_ME(UVIdle, __construct, ARGINFO(UVIdle, __construct), ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
    PHP_ME(UVIdle, start, ARGINFO(UVIdle, start), ZEND_ACC_PUBLIC)
    PHP_ME(UVIdle, stop, NULL, ZEND_ACC_PUBLIC)    
    PHP_FE_END
};
#endif
