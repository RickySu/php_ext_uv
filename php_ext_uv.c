#include "php_ext_uv.h"

PHP_MINIT_FUNCTION(php_ext_uv);
PHP_MSHUTDOWN_FUNCTION(php_ext_uv);
PHP_MINFO_FUNCTION(php_ext_uv);

zend_module_entry php_ext_uv_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
    STANDARD_MODULE_HEADER,
#endif
    "libuv Module",
    NULL,
    PHP_MINIT(php_ext_uv),
    PHP_MSHUTDOWN(php_ext_uv),
    NULL,
    NULL,
    PHP_MINFO(php_ext_uv),
#if ZEND_MODULE_API_NO >= 20010901
    "0.1",
#endif
    STANDARD_MODULE_PROPERTIES
};

#if COMPILE_DL_PHP_EXT_UV
ZEND_GET_MODULE(php_ext_uv)
#endif

PHP_MINIT_FUNCTION(php_ext_uv) {
    CLASS_ENTRY_FUNCTION_C(UVLoop);
    CLASS_ENTRY_FUNCTION_C(UVSignal);
    CLASS_ENTRY_FUNCTION_C(UVTimer);
    CLASS_ENTRY_FUNCTION_C(UVUdp);
    CLASS_ENTRY_FUNCTION_C(UVTcp);
#ifdef HAVE_OPENSSL
    CLASS_ENTRY_FUNCTION_C(UVSSL);
#endif
    CLASS_ENTRY_FUNCTION_C(UVResolver);
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(php_ext_uv) {
    return SUCCESS;
}

PHP_MINFO_FUNCTION(php_ext_uv) {
    php_info_print_table_start();
    php_info_print_table_header(2, "php_ext_uv support", "enabled");
    php_info_print_table_end();
}
