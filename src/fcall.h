#ifndef _FCALL_INFO_H
#define _FCALL_INFO_H

#define FCI_ADDREF(x) Z_TRY_ADDREF(x.fci.function_name)
#define FCI_PARSE_PARAMETERS_CC(x) &x.fci, &x.fcc
#define FCI_ISNULL(x) (x.fci.size == 0)
#define FCI_FREE(x) freeFunctionCache(&x)

typedef struct fcall_info_s {
    zend_fcall_info fci;
    zend_fcall_info_cache fcc;
} fcall_info_t;

static zend_always_inline int fci_call_function(fcall_info_t *fcall, zval *retval, uint32_t param_count, zval *params) {
    if(fcall->fci.size == 0){
        return FAILURE;
    }    
    fcall->fci.params = params;
    fcall->fci.retval = retval;
    fcall->fci.param_count = param_count;
    return zend_call_function(&fcall->fci, &fcall->fcc);
}

static zend_always_inline void freeFunctionCache(fcall_info_t *fcall){
    if(fcall->fci.size > 0){
        if(Z_REFCOUNTED(fcall->fci.function_name)){
            zval_dtor(&fcall->fci.function_name);
        }
        fcall->fci.size = 0;
    }
}

#endif