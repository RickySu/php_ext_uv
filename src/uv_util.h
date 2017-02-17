#ifndef _UV_UTIL_H
#define _UV_UTIL_H
#include "../php_ext_uv.h"

CLASS_ENTRY_FUNCTION_D(UVUtil);

ZEND_BEGIN_ARG_INFO(ARGINFO(UVUtil, errorMessage), 0)
    ZEND_ARG_INFO(0, err)
ZEND_END_ARG_INFO()

PHP_METHOD(UVUtil, version);
PHP_METHOD(UVUtil, versionString);
PHP_METHOD(UVUtil, errorMessage);
PHP_METHOD(UVUtil, __construct);
PHP_METHOD(UVUtil, cpuinfo);

DECLARE_FUNCTION_ENTRY(UVUtil) = {
    PHP_ME(UVUtil, __construct, NULL, ZEND_ACC_PRIVATE | ZEND_ACC_FINAL)
    PHP_ME(UVUtil, errorMessage, ARGINFO(UVUtil, errorMessage), ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(UVUtil, version, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(UVUtil, versionString, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(UVUtil, cpuinfo, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_FE_END
};

#endif
