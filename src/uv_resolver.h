#ifndef _UV_RESOLVER_H
#define _UV_RESOLVER_H
#include "../php_ext_uv.h"
#include "uv_loop_resource.h"

#define INIT_INFO(i, t, o, c) \
    i = emalloc(sizeof(t)); \
    i->object = o; \
    i->callback = c; \
    Z_ADDREF_P(c); \
    Z_ADDREF_P(o)
    
#define RELEASE_INFO(info) \
    Z_DELREF_P(info->callback); \
    Z_DELREF_P(info->object); \
    efree(info)

ZEND_BEGIN_ARG_INFO(ARGINFO(UVResolver, __construct), 0)
    ZEND_ARG_INFO(0, loop)
    ZEND_END_ARG_INFO()
    
ZEND_BEGIN_ARG_INFO(ARGINFO(UVResolver, getaddrinfo), 0)
    ZEND_ARG_INFO(0, node)
    ZEND_ARG_INFO(0, service)
    ZEND_ARG_INFO(0, callback)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(ARGINFO(UVResolver, getnameinfo), 0)
    ZEND_ARG_INFO(0, addr)
    ZEND_ARG_INFO(0, callback)
ZEND_END_ARG_INFO()

typedef struct uv_getaddrinfo_ext_s{
    uv_getaddrinfo_t uv_getaddrinfo;
    zval *object;
    zval *callback;
} uv_getaddrinfo_ext_t;

typedef struct uv_getnameinfo_ext_s{
    uv_getnameinfo_t uv_getnameinfo;
    zval *object;
    zval *callback;
} uv_getnameinfo_ext_t;

PHP_METHOD(UVResolver, __construct);
PHP_METHOD(UVResolver, getnameinfo);
PHP_METHOD(UVResolver, getaddrinfo);

DECLARE_FUNCTION_ENTRY(UVResolver) = {
    PHP_ME(UVResolver, __construct, ARGINFO(UVResolver, __construct), ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
    PHP_ME(UVResolver, getaddrinfo, ARGINFO(UVResolver, getaddrinfo), ZEND_ACC_PUBLIC)
    PHP_ME(UVResolver, getnameinfo, ARGINFO(UVResolver, getnameinfo), ZEND_ACC_PUBLIC)    
    PHP_FE_END
};
#endif
