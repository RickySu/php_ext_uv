#ifndef _FCALL_INFO_H
#define _FCALL_INFO_H

typedef struct fcall_info_s {
    zval *func;
    zend_fcall_info fci;
    zend_fcall_info_cache fcc;
} fcall_info_t;

static zend_always_inline int fci_call_function(fcall_info_t *fcall_into, zval *retval_ptr, zend_uint param_count, zval *params[] TSRMLS_DC) {
    zval ***params_array = NULL;
    zend_uint i;
    int ex_retval;
    zval *local_retval_ptr = NULL;
                
    if(fcall_into->fci.size == 0){
        return FAILURE;
    }
    
    if(param_count){
        params_array = (zval ***) emalloc(sizeof(zval **)*param_count);
        for (i=0; i<param_count; i++) {
            params_array[i] = &params[i];
        }
    }
        
    fcall_into->fci.params = params_array;
    fcall_into->fci.retval_ptr_ptr = &local_retval_ptr;
    fcall_into->fci.param_count = param_count;
    ex_retval = zend_call_function(&fcall_into->fci, &fcall_into->fcc TSRMLS_CC);
    if (params_array) {
        efree(params_array);
    }
    if (local_retval_ptr) {
        COPY_PZVAL_TO_ZVAL(*retval_ptr, local_retval_ptr);
    }
    else{
        INIT_ZVAL(*retval_ptr);
    }
    return ex_retval;
}

static zend_always_inline void freeFunctionCache(fcall_info_t *fcall TSRMLS_DC){
    if(fcall->fci.size > 0){
        zval_ptr_dtor(&fcall->fci.function_name);
        zval_ptr_dtor(&fcall->func);
        fcall->fci.size = 0;
    }
}

static zend_always_inline void registerFunctionCache(fcall_info_t *fcall_into, zval *cb TSRMLS_DC) {
    char *errstr = NULL;
    freeFunctionCache(fcall_into TSRMLS_CC);
    if (zend_fcall_info_init(cb, 0, &fcall_into->fci, &fcall_into->fcc, NULL, NULL TSRMLS_CC) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "param cb is not callable");
        return;
    }
    Z_ADDREF_P(fcall_into->fci.function_name);
    MAKE_STD_ZVAL(fcall_into->func);
    ZVAL_ZVAL(fcall_into->func, cb, 1, 0);
}

#endif