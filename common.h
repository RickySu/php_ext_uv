#ifndef _COMMON_H
#define _COMMON_H

#define CLASS_ENTRY(name) uv_ce_##name

#define OBJECT_HANDLER(name) object_handler_##name

#define CLASS_ENTRY_FUNCTION_D(name) \
    void init_uv_ce_##name(TSRMLS_D)

#define CLASS_ENTRY_FUNCTION_C(name) \
    init_uv_ce_##name(TSRMLS_C)    

#define DECLARE_CLASS_ENTRY(name) \
    static zend_object_handlers OBJECT_HANDLER(name); \
    static zend_class_entry *CLASS_ENTRY(name); \
    CLASS_ENTRY_FUNCTION_D(name)

#define FUNCTION_ENTRY(name) \
    uv_fe_##name

#define DECLARE_FUNCTION_ENTRY(name) \
    static zend_function_entry FUNCTION_ENTRY(name)[]

#define REGISTER_CLASS_WITH_OBJECT_NEW(name, create_function) \
    zend_class_entry ce; \
    memcpy(&OBJECT_HANDLER(name), zend_get_std_object_handlers(), sizeof(zend_object_handlers)); \
    INIT_CLASS_ENTRY(ce, #name, FUNCTION_ENTRY(name)); \
    ce.create_object = create_function; \
    CLASS_ENTRY(name) = zend_register_internal_class(&ce TSRMLS_CC)

#define REGISTER_CLASS(name) \
    zend_class_entry ce; \
    INIT_CLASS_ENTRY(ce, #name, FUNCTION_ENTRY(name)); \
    CLASS_ENTRY(name) = zend_register_internal_class(&ce TSRMLS_CC)

#define ARGINFO(classname, method) \
    arginfo_##classname_##method

#define FETCH_RESOURCE(pointer, type) (type *) (pointer - offsetof(type, zo))
    
#define FETCH_OBJECT_RESOURCE(object, type) FETCH_RESOURCE(zend_object_store_get_object(object TSRMLS_CC), type)

#endif
    
