#ifndef _UV_TCP_H
#define _UV_TCP_H
#include "../php_ext_uv.h"
#include "uv_loop_resource.h"

#define UV_TCP_HANDLE_INTERNAL_REF 1
#define UV_TCP_HANDLE_START (1<<1)
#define UV_TCP_READ_START (1<<2)
#define UV_TCP_CLOSING_START (1<<3)
#define UV_TCP_WRITE_CALLBACK_ENABLE (1<<4)

ZEND_BEGIN_ARG_INFO(ARGINFO(UVTcp, __construct), 0)
    ZEND_ARG_INFO(0, loop)
ZEND_END_ARG_INFO()

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

ZEND_BEGIN_ARG_INFO(ARGINFO(UVTcp, connect), 0)
    ZEND_ARG_INFO(0, host)
    ZEND_ARG_INFO(0, port)
    ZEND_ARG_INFO(0, onConnectCallback)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(ARGINFO(UVTcp, shutdown), 0)
    ZEND_ARG_INFO(0, onShutdownCallback)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(ARGINFO(UVTcp, write), 0)
    ZEND_ARG_INFO(0, buf)
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
int tcp_write_raw(uv_stream_t * handle, char *message, int size);

PHP_METHOD(UVTcp, getSockname);
PHP_METHOD(UVTcp, getSockport);
PHP_METHOD(UVTcp, getPeername);
PHP_METHOD(UVTcp, getPeerport);
PHP_METHOD(UVTcp, listen);
PHP_METHOD(UVTcp, accept);
PHP_METHOD(UVTcp, write);
PHP_METHOD(UVTcp, setCallback);
PHP_METHOD(UVTcp, close);
PHP_METHOD(UVTcp, shutdown);
PHP_METHOD(UVTcp, __construct);
PHP_METHOD(UVTcp, connect);

DECLARE_FUNCTION_ENTRY(UVTcp) = {
    PHP_ME(UVTcp, __construct, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
    PHP_ME(UVTcp, getSockname, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(UVTcp, getSockport, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(UVTcp, getPeername, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(UVTcp, getPeerport, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(UVTcp, listen, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(UVTcp, accept, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(UVTcp, setCallback, ARGINFO(UVTcp, setCallback), ZEND_ACC_PUBLIC)
    PHP_ME(UVTcp, write, ARGINFO(UVTcp, write), ZEND_ACC_PUBLIC)
    PHP_ME(UVTcp, close, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(UVTcp, shutdown, ARGINFO(UVTcp, shutdown), ZEND_ACC_PUBLIC)
    PHP_ME(UVTcp, connect, ARGINFO(UVTcp, connect), ZEND_ACC_PUBLIC)    
    PHP_FE_END
};
#endif
