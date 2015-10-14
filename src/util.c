#include "util.h"

int check_zval_type(zend_class_entry *call_ce, const char * function_name, uint function_name_len, zend_class_entry *instance_ce, zval *val TSRMLS_DC){
    zend_function *fptr;
    if (IS_OBJECT != Z_TYPE_P(val)){
        if(zend_hash_find(&call_ce->function_table, function_name, function_name_len, (void **) &fptr) == SUCCESS) {
            zend_verify_arg_error(E_RECOVERABLE_ERROR, fptr, 1, "be an ", instance_ce->name, "", zend_zval_type_name(val) TSRMLS_CC);
        }
        return 0;
    }
                                            
    if(!instanceof_function(Z_OBJCE_P(val), instance_ce TSRMLS_CC)) {
        if(zend_hash_find(&call_ce->function_table, function_name, function_name_len, (void **) &fptr) == SUCCESS) {
            zend_verify_arg_error(E_RECOVERABLE_ERROR, fptr, 1, "be an ", instance_ce->name, "instance of ", Z_OBJCE_P(val)->name TSRMLS_CC);
        }
        return 0;
    }

    return 1;
}