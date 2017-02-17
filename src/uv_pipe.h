#ifndef _UV_TCP_H
#define _UV_TCP_H
#include "../php_ext_uv.h"
#include "uv_loop_resource.h"
#include "fcall.h"

#define UV_PIPE_HANDLE_INTERNAL_REF 1
#define UV_PIPE_HANDLE_START (1<<1)
#define UV_PIPE_READ_START (1<<2)
#define UV_PIPE_CLOSING_START (1<<3)
#define UV_PIPE_WRITE_CALLBACK_ENABLE (1<<4)
#define UV_PIPE_SOCKENAME_SIZE 200

ZEND_BEGIN_ARG_INFO(ARGINFO(UVPipe, __construct), 0)
    ZEND_ARG_OBJ_INFO(0, loop, UVLoop, 1)
    ZEND_ARG_INFO(0, ipc)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(ARGINFO(UVPipe, setCallback), 0)
    ZEND_ARG_INFO(0, onRecvCallback)
    ZEND_ARG_INFO(0, onSendCallback)
    ZEND_ARG_INFO(0, onErrorCallback)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(ARGINFO(UVPipe, listen), 0)
    ZEND_ARG_INFO(0, host)
    ZEND_ARG_INFO(0, onConnectCallback)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(ARGINFO(UVPipe, connect), 0)
    ZEND_ARG_INFO(0, host)
    ZEND_ARG_INFO(0, onConnectCallback)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(ARGINFO(UVPipe, shutdown), 0)
    ZEND_ARG_INFO(0, onShutdownCallback)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(ARGINFO(UVPipe, write), 0)
    ZEND_ARG_INFO(0, buf)
ZEND_END_ARG_INFO()

typedef struct uv_pipe_ext_s{
    uv_pipe_t uv_pipe;
    uint flag;
    int ipc;
    uv_connect_t connect_req;
    uv_shutdown_t shutdown_req;
    char *sockAddr;
    char *peerAddr;
    zval gc_table[5];
    fcall_info_t readCallback;
    fcall_info_t writeCallback;
    fcall_info_t errorCallback;
    fcall_info_t connectCallback;
    fcall_info_t shutdownCallback;
    zval object;
    zend_object zo;
} uv_pipe_ext_t;

typedef struct write_req_s{
    uv_write_t uv_write;
    uv_buf_t buf;
} write_req_t;

static zend_always_inline void releaseUVPipeFunctionCache(uv_pipe_ext_t *resource){
    freeFunctionCache(&resource->readCallback);
    freeFunctionCache(&resource->writeCallback);
    freeFunctionCache(&resource->errorCallback);
    freeFunctionCache(&resource->connectCallback);
    freeFunctionCache(&resource->shutdownCallback);
}

static zend_object *createUVPipeResource(zend_class_entry *class_type);
static void freeUVPipeResource(zend_object *object);
HashTable *get_gc_UVPipeResource(zval *obj, zval **table, int *n);
static void releaseResource(uv_pipe_ext_t *resource);
int pipe_write_raw(uv_stream_t * handle, const char *message, int size);
zend_bool make_accepted_uv_pipe_object(uv_pipe_ext_t *server_resource, zval *client);
static void pipe_close_cb(uv_handle_t* handle);
void pipe_close_socket(uv_pipe_ext_t *handle);
static void setSelfReference(uv_pipe_ext_t *resource);

PHP_METHOD(UVPipe, getSockname);
PHP_METHOD(UVPipe, getPeername);
PHP_METHOD(UVPipe, listen);
PHP_METHOD(UVPipe, accept);
PHP_METHOD(UVPipe, write);
PHP_METHOD(UVPipe, setCallback);
PHP_METHOD(UVPipe, close);
PHP_METHOD(UVPipe, shutdown);
PHP_METHOD(UVPipe, __construct);
PHP_METHOD(UVPipe, connect);

DECLARE_FUNCTION_ENTRY(UVPipe) = {
    PHP_ME(UVPipe, __construct, ARGINFO(UVPipe, __construct), ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
    PHP_ME(UVPipe, getSockname, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(UVPipe, getPeername, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(UVPipe, listen, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(UVPipe, accept, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(UVPipe, setCallback, ARGINFO(UVPipe, setCallback), ZEND_ACC_PUBLIC)
    PHP_ME(UVPipe, write, ARGINFO(UVPipe, write), ZEND_ACC_PUBLIC)
    PHP_ME(UVPipe, close, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(UVPipe, shutdown, ARGINFO(UVPipe, shutdown), ZEND_ACC_PUBLIC)
    PHP_ME(UVPipe, connect, ARGINFO(UVPipe, connect), ZEND_ACC_PUBLIC)    
    PHP_FE_END
};
#endif
