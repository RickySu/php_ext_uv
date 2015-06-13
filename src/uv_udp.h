#ifndef _UV_UDP_H
#define _UV_UDP_H
#include "../php_ext_uv.h"

#define UV_UDP_HANDLE_INTERNAL_REF 1
#define UV_UDP_HANDLE_START (1<<1)
#define UV_UDP_READ_START (1<<2)

ZEND_BEGIN_ARG_INFO(ARGINFO(UVUdp, bind), 0)
    ZEND_ARG_INFO(0, host)
    ZEND_ARG_INFO(0, port)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(ARGINFO(UVUdp, setCallback), 0)
    ZEND_ARG_INFO(0, onRecvCallback)
    ZEND_ARG_INFO(0, onSendCallback)
    ZEND_ARG_INFO(0, onErrorCallback)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(ARGINFO(UVUdp, sendTo), 0)
    ZEND_ARG_INFO(0, dest)
    ZEND_ARG_INFO(0, port)
    ZEND_ARG_INFO(0, message)
ZEND_END_ARG_INFO()

typedef struct uv_udp_ext_s{
    uv_udp_t uv_udp;
    uint flag;
    char *sockAddr;
    int sockPort;    
    zval *object;
    zend_object zo;    
} uv_udp_ext_t;

typedef struct send_req_s{
    uv_udp_send_t uv_udp_send;
    uv_buf_t buf;
    struct sockaddr_in addr;
} send_req_t;

static zend_object_value createUVUdpResource(zend_class_entry *class_type TSRMLS_DC);

void freeUVUdpResource(void *object TSRMLS_DC);

PHP_METHOD(UVUdp, getSockname);
PHP_METHOD(UVUdp, getSockport);
PHP_METHOD(UVUdp, bind);
PHP_METHOD(UVUdp, setCallback);
PHP_METHOD(UVUdp, close);
PHP_METHOD(UVUdp, sendTo);

DECLARE_FUNCTION_ENTRY(UVUdp) = {    
    PHP_ME(UVUdp, getSockname, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(UVUdp, getSockport, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(UVUdp, close, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(UVUdp, bind, ARGINFO(UVUdp, bind), ZEND_ACC_PUBLIC)
    PHP_ME(UVUdp, sendTo, ARGINFO(UVUdp, sendTo), ZEND_ACC_PUBLIC)
    PHP_ME(UVUdp, setCallback, ARGINFO(UVUdp, setCallback), ZEND_ACC_PUBLIC)
    PHP_FE_END
};
#endif
