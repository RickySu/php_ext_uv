#ifndef _UV_TCP_H
#define _UV_TCP_H
#include "../php_ext_uv.h"
#include "uv_loop_resource.h"
#include "fcall.h"

#define UV_TCP_HANDLE_INTERNAL_REF 1
#define UV_TCP_HANDLE_START (1<<1)
#define UV_TCP_READ_START (1<<2)
#define UV_TCP_CLOSING_START (1<<3)
#define UV_TCP_WRITE_CALLBACK_ENABLE (1<<4)

ZEND_BEGIN_ARG_INFO(ARGINFO(UVTcp, __construct), 0)
    ZEND_ARG_OBJ_INFO(0, loop, UVLoop, 1)
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
    fcall_info_t readCallback;
    fcall_info_t writeCallback;
    fcall_info_t errorCallback;
    fcall_info_t connectCallback;
    fcall_info_t shutdownCallback;
    zval object;
    zend_object zo;
} uv_tcp_ext_t;

typedef struct write_req_s{
    uv_write_t uv_write;
    uv_buf_t buf;
} write_req_t;

static zend_always_inline void releaseUVTcpFunctionCache(uv_tcp_ext_t *resource){
    freeFunctionCache(&resource->readCallback);
    freeFunctionCache(&resource->writeCallback);
    freeFunctionCache(&resource->errorCallback);
    freeFunctionCache(&resource->connectCallback);
    freeFunctionCache(&resource->shutdownCallback);
}

static zend_object *createUVTcpResource(zend_class_entry *class_type);
static void freeUVTcpResource(zend_object *object);
void releaseResource(uv_tcp_ext_t *resource);
int tcp_write_raw(uv_stream_t * handle, const char *message, int size);
zend_bool make_accepted_uv_tcp_object(uv_tcp_ext_t *server_resource, zval *client);
static void tcp_close_cb(uv_handle_t* handle);
void tcp_close_socket(uv_tcp_ext_t *handle);
void setSelfReference(uv_tcp_ext_t *resource);

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
    PHP_ME(UVTcp, __construct, ARGINFO(UVTcp, __construct), ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
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
