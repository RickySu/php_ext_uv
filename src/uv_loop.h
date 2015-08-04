#ifndef _UV_LOOP_H
#define _UV_LOOP_H
#include "../php_ext_uv.h"
#include "uv_loop_resource.h"

#define RUN_DEFAULT  0
#define RUN_ONCE  1
#define RUN_NOWAIT 2


CLASS_ENTRY_FUNCTION_D(UVLoop);

static zend_object_value createUVLoopResource(zend_class_entry *class_type TSRMLS_DC);
void freeUVLoopResource(void *object TSRMLS_DC);

ZEND_BEGIN_ARG_INFO(ARGINFO(UVLoop, run), 0)
    ZEND_ARG_INFO(0, option)
ZEND_END_ARG_INFO()

PHP_METHOD(UVLoop, defaultLoop);
PHP_METHOD(UVLoop, run);
PHP_METHOD(UVLoop, stop);
PHP_METHOD(UVLoop, alive);
PHP_METHOD(UVLoop, updateTime);
PHP_METHOD(UVLoop, now);
PHP_METHOD(UVLoop, backendFd);
PHP_METHOD(UVLoop, backendTimeout);

DECLARE_FUNCTION_ENTRY(UVLoop) = {    
    PHP_ME(UVLoop, run, ARGINFO(UVLoop, run), ZEND_ACC_PUBLIC)
    PHP_ME(UVLoop, stop, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(UVLoop, alive, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(UVLoop, updateTime, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(UVLoop, now, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(UVLoop, backendFd, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(UVLoop, backendTimeout, NULL, ZEND_ACC_PUBLIC)
    PHP_FE_END
};

#endif
