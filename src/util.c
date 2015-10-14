#include "util.h"

int check_zval_type(zend_class_entry *call_ce, const char * function_name, uint function_name_len, zend_class_entry *instance_ce, zval *val){
    zend_function *fptr;
    zend_string *zs_function_name;
    if (IS_OBJECT != Z_TYPE_P(val)){
        if(fptr = (zend_function *) zend_hash_str_find(&call_ce->function_table, function_name, function_name_len)) {
            zend_verify_arg_error(E_RECOVERABLE_ERROR, fptr, 1, "be an ", instance_ce->name, "", zend_zval_type_name(val));
        }
        return 0;
    }
                                            
    if(!instanceof_function(Z_OBJCE_P(val), instance_ce)) {
        if(fptr = (zend_function *) zend_hash_str_find(&call_ce->function_table, function_name, function_name_len)) {
            zend_verify_arg_error(E_RECOVERABLE_ERROR, fptr, 1, "be an ", instance_ce->name, "instance of ", Z_OBJCE_P(val)->name);
        }
        return 0;
    }

    return 1;
}