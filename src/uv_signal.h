#ifndef _UV_SIGNAL_H
#define _UV_SIGNAL_H
#include "../php_ext_uv.h"
#include "uv_loop_resource.h"
#include "fcall.h"

ZEND_BEGIN_ARG_INFO(ARGINFO(UVSignal, __construct), 0)
    ZEND_ARG_OBJ_INFO(0, loop, UVLoop, 1)
    ZEND_END_ARG_INFO()
    
ZEND_BEGIN_ARG_INFO(ARGINFO(UVSignal, start), 0)
    ZEND_ARG_INFO(0, signal_cb)
    ZEND_ARG_INFO(0, signo)
ZEND_END_ARG_INFO()

typedef struct uv_signal_ext_s{
    uv_signal_t uv_signal;
    int start;
    struct {
        zval callback;
    } gc_table;
    fcall_info_t callback;
    zval object;
    zend_object zo;
} uv_signal_ext_t;

static zend_object *createUVSignalResource(zend_class_entry *class_type);
static void signal_handle_callback(uv_signal_ext_t *signal_handle, int signo);
void freeUVSignalResource(zend_object *object);
static HashTable *get_gc_UVSignalResource(zval *obj, zval **table, int *n);

PHP_METHOD(UVSignal, __construct);
PHP_METHOD(UVSignal, start);
PHP_METHOD(UVSignal, stop);

DECLARE_FUNCTION_ENTRY(UVSignal) = {
    PHP_ME(UVSignal, __construct, ARGINFO(UVSignal, __construct), ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
    PHP_ME(UVSignal, start, ARGINFO(UVSignal, start), ZEND_ACC_PUBLIC)
    PHP_ME(UVSignal, stop, NULL, ZEND_ACC_PUBLIC)    
    PHP_FE_END
};
#endif
