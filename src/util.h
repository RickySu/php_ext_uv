#ifndef UTIL_H
#define	UTIL_H
#include "php_ext_uv.h"

#define TIMEVAL_SET(tv, t) \
    do {                                             \
        tv.tv_sec  = (long) t;                       \
        tv.tv_usec = (long) ((t - tv.tv_sec) * 1e6); \
    } while (0)

#define TIMEVAL_TO_DOUBLE(tv) (tv.tv_sec + tv.tv_usec * 1e-6)
    
char *sock_addr(struct sockaddr *addr);
int sock_port(struct sockaddr *addr);
#endif	/* UTIL_H */
