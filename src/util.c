#include "util.h"

static inline char *sock_addr(struct sockaddr *addr) {
    struct sockaddr_in addr_in = *(struct sockaddr_in *) addr;
    char *ip = emalloc(20);
    uv_ip4_name(&addr_in, ip, 20);
    return ip;
}
    
static inline int sock_port(struct sockaddr *addr) {
    struct sockaddr_in addr_in = *(struct sockaddr_in *) addr;
    return ntohs(addr_in.sin_port);
}
