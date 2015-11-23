#ifndef _FCALL_INFO_H
#define _FCALL_INFO_H

typedef struct fcall_info_s {
    zval func;
    zend_fcall_info fci;
    zend_fcall_info_cache fcc;
} fcall_info_t;

static zend_always_inline int fci_call_function(fcall_info_t *fcall_into, zval *retval, uint32_t param_count, zval *params) {
    fcall_into->fci.params = params;
    fcall_into->fci.retval = retval;
    fcall_into->fci.param_count = param_count;
    zend_call_function(&fcall_into->fci, &fcall_into->fcc);
    return 0;
}

static zend_always_inline void registerFunctionCache(fcall_info_t *fcall_into, zval *cb) {
    char *errstr = NULL;
    if (zend_fcall_info_init(cb, 0, &fcall_into->fci, &fcall_into->fcc, NULL, &errstr) == FAILURE) {
        php_error_docref(NULL, E_WARNING, "param cb is not callable");
        return;
    }
    Z_ADDREF_P(cb);
    zval_dtor(&fcall_into->func);
    ZVAL_COPY_VALUE(&fcall_into->func ,cb);
}

static zend_always_inline void freeFunctionCache(fcall_info_t *fcall){
    zval_dtor(&fcall->func);
    zval_dtor(&fcall->fci.function_name);
}

#endif