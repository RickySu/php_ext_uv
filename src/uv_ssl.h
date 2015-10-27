#ifndef _UV_SSL_H
#define _UV_SSL_H
#include "uv_tcp.h"
#include "ssl_verify.h"
#include "uv_ssl_constant.h"

#define SSL_METHOD_SSLV2 0
#define SSL_METHOD_SSLV3 1
#define SSL_METHOD_SSLV23 2
#define SSL_METHOD_TLSV1 3
#define SSL_METHOD_TLSV1_1 4
#define SSL_METHOD_TLSV1_2 5
#define OPENSSL_DEFAULT_STREAM_VERIFY_DEPTH 9

ZEND_BEGIN_ARG_INFO_EX(ARGINFO(UVSSL, __construct), 0, 0, 1)
    ZEND_ARG_OBJ_INFO(0, loop, UVLoop, 1)
    ZEND_ARG_INFO(0, sslMethod)
    ZEND_ARG_INFO(0, nContexts)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(ARGINFO(UVSSL, setSSLServerNameCallback), 0)
    ZEND_ARG_INFO(0, callback) 
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(ARGINFO(UVSSL, setSSLHandshakeCallback), 0)
    ZEND_ARG_INFO(0, callback) 
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ARGINFO(UVSSL, setCert), 0, 0, 1)
    ZEND_ARG_INFO(0, cert)
    ZEND_ARG_INFO(0, n)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ARGINFO(UVSSL, setPrivateKey), 0, 0, 1)
    ZEND_ARG_INFO(0, privateKey)
    ZEND_ARG_INFO(0, n)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(ARGINFO(UVSSL, write), 0)
    ZEND_ARG_INFO(0, message)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(ARGINFO(UVSSL, connect), 0)
    ZEND_ARG_INFO(0, host)
    ZEND_ARG_INFO(0, port)
    ZEND_ARG_INFO(0, onConnectCallback)
ZEND_END_ARG_INFO()

typedef struct uv_ssl_ext_s{
    const SSL_METHOD *ssl_method;
    SSL_CTX** ctx;
    int nctx;
    SSL* ssl;
    BIO* read_bio;
    BIO* write_bio;
    char *sniConnectHostname;
    int clientMode;
    uv_tcp_ext_t uv_tcp_ext;
} uv_ssl_ext_t;

typedef struct uv_getaddrinfo_ext_s{   
    uv_getaddrinfo_t addrinfo;
    int port;
    uv_ssl_ext_t *ssl_handle;
} uv_getaddrinfo_ext_t;

static zend_object *createUVSSLResource(zend_class_entry *class_type);
void freeUVSSLResource(zend_object *object);
static int write_bio_to_socket(uv_ssl_ext_t *resource);

PHP_METHOD(UVSSL, __construct);
PHP_METHOD(UVSSL, setSSLServerNameCallback);
PHP_METHOD(UVSSL, setSSLHandshakeCallback);
PHP_METHOD(UVSSL, setCert);
PHP_METHOD(UVSSL, setPrivateKey);
PHP_METHOD(UVSSL, accept);
PHP_METHOD(UVSSL, write);
PHP_METHOD(UVSSL, connect);

DECLARE_FUNCTION_ENTRY(UVSSL) = {
    PHP_ME(UVSSL, __construct, ARGINFO(UVSSL, __construct), ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
    PHP_ME(UVSSL, setSSLServerNameCallback, ARGINFO(UVSSL, setSSLServerNameCallback), ZEND_ACC_PUBLIC)
    PHP_ME(UVSSL, setSSLHandshakeCallback, ARGINFO(UVSSL, setSSLHandshakeCallback), ZEND_ACC_PUBLIC)
    PHP_ME(UVSSL, setCert, ARGINFO(UVSSL, setCert), ZEND_ACC_PUBLIC)
    PHP_ME(UVSSL, setPrivateKey, ARGINFO(UVSSL, setPrivateKey), ZEND_ACC_PUBLIC)
    PHP_ME(UVSSL, accept, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(UVSSL, write, ARGINFO(UVSSL, write), ZEND_ACC_PUBLIC)
    PHP_ME(UVSSL, connect, ARGINFO(UVSSL, connect), ZEND_ACC_PUBLIC)
    PHP_FE_END
};

#define FETCH_SSL_RESOURCE(zo) \
    ((uv_ssl_ext_t *) FETCH_RESOURCE_FROM_EXTEND(FETCH_RESOURCE(zo, uv_tcp_ext_t), uv_tcp_ext, uv_ssl_ext_t))

#define FETCH_OBJECT_SSL_RESOURCE(object) \
    (FETCH_RESOURCE_FROM_EXTEND(FETCH_OBJECT_RESOURCE(object, uv_tcp_ext_t), uv_tcp_ext, uv_ssl_ext_t))

#endif
