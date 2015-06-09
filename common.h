#ifndef _COMMON_H
#define _COMMON_H

#define CLASS_ENTRY(name) uv_ce_##name

#define CLASS_ENTRY_FUNCTION_D(name) \
    zend_class_entry *CLASS_ENTRY(name); \
    void init_uv_ce_##name(TSRMLS_D)

#define CLASS_ENTRY_FUNCTION_C(name) \
    init_uv_ce_##name(TSRMLS_C)    

#define DECLARE_CLASS_ENTRY(name) \
    extern zend_class_entry *CLASS_ENTRY(name); \
    CLASS_ENTRY_FUNCTION_D(name);

#define FUNCTION_ENTRY(name) \
    uv_fe_##name
    
#define DECLARE_FUNCTION_ENTRY(name) \
    const zend_function_entry FUNCTION_ENTRY(name)[]
    
#define REGISTER_CLASS(name) \
    zend_class_entry ce; \
    INIT_CLASS_ENTRY(ce, #name, FUNCTION_ENTRY(name)); \
    CLASS_ENTRY(name) = zend_register_internal_class(&ce TSRMLS_CC);

#define ARGINFO(classname, method) \
    arginfo_##classname_##method

#endif

