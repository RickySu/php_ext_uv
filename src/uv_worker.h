#ifndef _UV_WORKER_H
#define _UV_WORKER_H
#include "../php_ext_uv.h"
#include "uv_loop_resource.h"
#include "fcall.h"
#include "uv_tcp.h"

ZEND_BEGIN_ARG_INFO(ARGINFO(UVWorker, setCloseCallback), 0)
    ZEND_ARG_INFO(0, close_cb)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(ARGINFO(UVWorker, kill), 0)
    ZEND_ARG_INFO(0, signum)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(ARGINFO(UVWorker, attach), 0)
    ZEND_ARG_OBJ_INFO(0, stream, UVTcp, 1)
ZEND_END_ARG_INFO()

typedef struct {
    uv_write_t req;
    uv_tcp_t *client;
} write2_req_t;

typedef struct uv_worker_ext_s{
    uv_process_t process;
    uv_process_options_t options;
    uv_pipe_t pipe;
    uv_loop_t *loop;
    uv_buf_t dummy_buf;
    struct {
        zval closeCallback;
    } gc_table;
    fcall_info_t closeCallback;
    zval object;
    zend_object zo;    
} uv_worker_ext_t;

static zend_object *createUVWorkerResource(zend_class_entry *class_type);
void freeUVWorkerResource(zend_object *object);
static HashTable *get_gc_UVWorkerResource(zval *obj, zval **table, int *n);
static void close_process_handle(uv_process_t *req, int64_t exit_status, int term_signal);
int make_worker(uv_loop_t *loop, char *args[], zval *worker);

PHP_METHOD(UVWorker, __construct);
PHP_METHOD(UVWorker, kill);
PHP_METHOD(UVWorker, getPid);
PHP_METHOD(UVWorker, setCloseCallback);
PHP_METHOD(UVWorker, attach);

DECLARE_FUNCTION_ENTRY(UVWorker) = {
    PHP_ME(UVWorker, __construct, NULL, ZEND_ACC_PRIVATE | ZEND_ACC_FINAL)
    PHP_ME(UVWorker, kill, ARGINFO(UVWorker, kill), ZEND_ACC_PUBLIC)
    PHP_ME(UVWorker, getPid, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(UVWorker, setCloseCallback, ARGINFO(UVWorker, setCloseCallback), ZEND_ACC_PUBLIC)
    PHP_ME(UVWorker, attach, ARGINFO(UVWorker, attach), ZEND_ACC_PUBLIC)
    PHP_FE_END
};
#endif
