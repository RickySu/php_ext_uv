#ifndef UTIL_H
#define	UTIL_H
#include "../php_ext_uv.h"

#define TIMEVAL_SET(tv, t) \
    do {                                             \
        tv.tv_sec  = (long) t;                       \
        tv.tv_usec = (long) ((t - tv.tv_sec) * 1e6); \
    } while (0)

#define TIMEVAL_TO_DOUBLE(tv) (tv.tv_sec + tv.tv_usec * 1e-6)

static zend_always_inline char *sock_addr(struct sockaddr *addr) {
    struct sockaddr_in addr_in = *(struct sockaddr_in *) addr;
    char *ip = emalloc(20);
    uv_ip4_name(&addr_in, ip, 20);
    return ip;
}
    
static zend_always_inline int sock_port(struct sockaddr *addr) {
    struct sockaddr_in addr_in = *(struct sockaddr_in *) addr;
    return ntohs(addr_in.sin_port);
}

#define COPY_C_STR(c_str, str, str_len) \
    memcpy(c_str, str, str_len); \
    c_str[str_len] = '\0'

#define MAKE_C_STR(c_str, str, str_len) \
    c_str = emalloc(str_len + 1); \
    COPY_C_STR(c_str, str, str_len)

int check_zval_type(zend_class_entry *call_ce, const char * function_name, uint function_name_len, zend_class_entry *instance_ce, zval *val);
#endif	/* UTIL_H */
