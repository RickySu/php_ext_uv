#ifndef _UV_PROCESS_H
#define _UV_PROCESS_H
#include "../php_ext_uv.h"
#include "uv_loop_resource.h"
#include "fcall.h"

ZEND_BEGIN_ARG_INFO(ARGINFO(UVProcess, __construct), 0)
    ZEND_ARG_OBJ_INFO(0, loop, UVLoop, 1)
    ZEND_END_ARG_INFO()
    
ZEND_BEGIN_ARG_INFO(ARGINFO(UVProcess, spawn), 0)
    ZEND_ARG_INFO(0, args)
    ZEND_END_ARG_INFO()

typedef struct uv_process_ext_s{
    uv_loop_t *loop;
    zval object;
    zend_object zo;    
} uv_process_ext_t;

static zend_object *createUVProcessResource(zend_class_entry *class_type);
static void process_handle_callback(uv_process_ext_t *process_handle);
void freeUVProcessResource(zend_object *object);
static void freeStringArray(char *args[]);

PHP_METHOD(UVProcess, __construct);
PHP_METHOD(UVProcess, spawn);

DECLARE_FUNCTION_ENTRY(UVProcess) = {
    PHP_ME(UVProcess, __construct, ARGINFO(UVProcess, __construct), ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
    PHP_ME(UVProcess, spawn, ARGINFO(UVProcess, spawn), ZEND_ACC_PUBLIC)
    PHP_FE_END
};
#endif
