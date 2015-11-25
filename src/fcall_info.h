#ifndef _FCALL_INFO_H
#define _FCALL_INFO_H

typedef struct fcall_info_s {
    zval func;
    zend_fcall_info fci;
    zend_fcall_info_cache fcc;
} fcall_info_t;

static zend_always_inline int fci_call_function(fcall_info_t *fcall_into, zval *retval, uint32_t param_count, zval *params) {
    if(fcall_into->fci.size == 0){
        return FAILURE;
    }    
    fcall_into->fci.params = params;
    fcall_into->fci.retval = retval;
    fcall_into->fci.param_count = param_count;
    return zend_call_function(&fcall_into->fci, &fcall_into->fcc);
}

static zend_always_inline void freeFunctionCache(fcall_info_t *fcall){
    if(fcall->fci.size > 0){
        zval_dtor(&fcall->fci.function_name);
        zval_dtor(&fcall->func);
        fcall->fci.size = 0;
    }
}

static zend_always_inline void registerFunctionCache(fcall_info_t *fcall_into, zval *cb) {
    char *errstr = NULL;
    freeFunctionCache(fcall_into);
    if (zend_fcall_info_init(cb, 0, &fcall_into->fci, &fcall_into->fcc, NULL, &errstr) == FAILURE) {
        php_error_docref(NULL, E_WARNING, "param cb is not callable");
        return;
    }
    Z_ADDREF(fcall_into->fci.function_name);
    Z_ADDREF_P(cb);
    ZVAL_COPY_VALUE(&fcall_into->func ,cb);
}



#endif