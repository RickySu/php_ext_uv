#ifndef _UV_UDP_H
#define _UV_UDP_H
#include "../php_ext_uv.h"
#include "uv_loop_resource.h"
#include "fcall.h"

#define UV_UDP_HANDLE_INTERNAL_REF 1
#define UV_UDP_HANDLE_START (1<<1)
#define UV_UDP_READ_START (1<<2)

ZEND_BEGIN_ARG_INFO(ARGINFO(UVUdp, __construct), 0)
    ZEND_ARG_OBJ_INFO(0, loop, UVLoop, 1)
ZEND_END_ARG_INFO()

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
    zval gc_table[3];
    fcall_info_t recvCallback;
    fcall_info_t sendCallback;
    fcall_info_t errorCallback;
    zval object;
    zend_object zo;    
} uv_udp_ext_t;

typedef struct send_req_s{
    uv_udp_send_t uv_udp_send;
    uv_buf_t buf;
    struct sockaddr_in addr;
} send_req_t;

static zend_object *createUVUdpResource(zend_class_entry *class_type);
static HashTable *get_gc_UVUdpResource(zval *obj, zval **table, int *n);
void freeUVUdpResource(zend_object *object);

PHP_METHOD(UVUdp, __construct);
PHP_METHOD(UVUdp, getSockname);
PHP_METHOD(UVUdp, getSockport);
PHP_METHOD(UVUdp, bind);
PHP_METHOD(UVUdp, setCallback);
PHP_METHOD(UVUdp, close);
PHP_METHOD(UVUdp, sendTo);

DECLARE_FUNCTION_ENTRY(UVUdp) = {
    PHP_ME(UVUdp, __construct, ARGINFO(UVUdp, __construct), ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
    PHP_ME(UVUdp, getSockname, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(UVUdp, getSockport, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(UVUdp, close, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(UVUdp, bind, ARGINFO(UVUdp, bind), ZEND_ACC_PUBLIC)
    PHP_ME(UVUdp, sendTo, ARGINFO(UVUdp, sendTo), ZEND_ACC_PUBLIC)
    PHP_ME(UVUdp, setCallback, ARGINFO(UVUdp, setCallback), ZEND_ACC_PUBLIC)
    PHP_FE_END
};

static zend_always_inline void releaseUVUdpFunctionCache(uv_udp_ext_t *resource){
    freeFunctionCache(&resource->recvCallback);
    freeFunctionCache(&resource->sendCallback);
    freeFunctionCache(&resource->errorCallback);
}
#endif
