#ifndef _UV_LOOP_RESOURCE_H
#define _UV_LOOP_RESOURCE_H

typedef struct uv_loop_ext_s{
    uv_loop_t __loop__;
    uv_loop_t *loop;
    zend_object zo;
} uv_loop_ext_t;

#endif
