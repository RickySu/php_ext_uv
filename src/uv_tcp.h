#ifndef _UV_TCP_H
#define _UV_TCP_H
#include "../php_ext_uv.h"

#define UV_TCP_HANDLE_INTERNAL_REF 1
#define UV_TCP_HANDLE_START (1<<1)
#define UV_TCP_READ_START (1<<2)
#define UV_TCP_CLOSING_START (1<<3)
#define UV_TCP_WRITE_CALLBACK_ENABLE (1<<4)

ZEND_BEGIN_ARG_INFO(ARGINFO(UVTcp, setCallback), 0)
    ZEND_ARG_INFO(0, onRecvCallback)
    ZEND_ARG_INFO(0, onSendCallback)
    ZEND_ARG_INFO(0, onErrorCallback)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(ARGINFO(UVTcp, listen), 0)
    ZEND_ARG_INFO(0, host)
    ZEND_ARG_INFO(0, port)
    ZEND_ARG_INFO(0, onConnectCallback)
ZEND_END_ARG_INFO()

typedef struct uv_tcp_ext_s{
    uv_tcp_t uv_tcp;
    uint flag;
    uv_connect_t connect_req;
    uv_shutdown_t shutdown_req;
    char *sockAddr;
    int sockPort;
    char *peerAddr;
    int peerPort;
    zval *object;
    zend_object zo;    
} uv_tcp_ext_t;

typedef struct write_req_s{
    uv_write_t uv_write;
    uv_buf_t buf;
} write_req_t;

static zend_object_value createUVTcpResource(zend_class_entry *class_type TSRMLS_DC);

void freeUVTcpResource(void *object TSRMLS_DC);

PHP_METHOD(UVTcp, getSockname);
PHP_METHOD(UVTcp, getSockport);
PHP_METHOD(UVTcp, getPeername);
PHP_METHOD(UVTcp, getPeerport);
PHP_METHOD(UVTcp, listen);
PHP_METHOD(UVTcp, accept);
PHP_METHOD(UVTcp, setCallback);

//PHP_METHOD(UVTcp, close);

DECLARE_FUNCTION_ENTRY(UVTcp) = {    
    PHP_ME(UVTcp, getSockname, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(UVTcp, getSockport, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(UVTcp, getPeername, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(UVTcp, getPeerport, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(UVTcp, listen, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(UVTcp, accept, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(UVTcp, setCallback, ARGINFO(UVTcp, setCallback), ZEND_ACC_PUBLIC)
//    PHP_ME(UVTcp, close, NULL, ZEND_ACC_PUBLIC)
//    PHP_ME(UVTcp, bind, ARGINFO(UVTcp, bind), ZEND_ACC_PUBLIC)

    PHP_FE_END
};
#endif
