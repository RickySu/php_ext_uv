#ifndef _UV_LOOP_H
#define _UV_LOOP_H
#include "php_ext_uv.h"

CLASS_ENTRY_FUNCTION_D(name);

PHP_METHOD(UVLoop, __construct);

DECLARE_FUNCTION_ENTRY(UVLoop) = {    
    PHP_ME(UVLoop, __construct, NULL, ZEND_ACC_PRIVATE|ZEND_ACC_CTOR|ZEND_ACC_FINAL)
    PHP_FE_END
};

#endif