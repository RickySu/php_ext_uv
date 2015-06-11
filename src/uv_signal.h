#ifndef _UV_SIGNAL_H
#define _UV_SIGNAL_H
#include "../php_ext_uv.h"

ZEND_BEGIN_ARG_INFO(ARGINFO(UVSignal, start), 0)
    ZEND_ARG_INFO(0, signal_cb)
    ZEND_ARG_INFO(0, signo)
ZEND_END_ARG_INFO()

typedef struct uv_signal_ext_s{
    uv_signal_t uv_signal;
    int start;
    zval *object;
    zend_object zo;    
} uv_signal_ext_t;

static zend_object_value createUVSignalResource(zend_class_entry *class_type TSRMLS_CC);
static void signal_handle_callback(uv_signal_ext_t *signal_handle, int signo);

void freeUVSignalResource(void *object TSRMLS_CC);

PHP_METHOD(UVSignal, start);
PHP_METHOD(UVSignal, stop);

DECLARE_FUNCTION_ENTRY(UVSignal) = {    
    PHP_ME(UVSignal, start, ARGINFO(UVSignal, start), ZEND_ACC_PUBLIC)
    PHP_ME(UVSignal, stop, NULL, ZEND_ACC_PUBLIC)    
    PHP_FE_END
};
#endif
