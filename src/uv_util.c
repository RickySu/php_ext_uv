#include "uv_util.h"

CLASS_ENTRY_FUNCTION_D(UVUtil){
    REGISTER_CLASS(UVUtil);
    OBJECT_HANDLER(UVUtil).clone_obj = NULL;
}

PHP_METHOD(UVUtil, __construct){
}

PHP_METHOD(UVUtil, version) {
    RETURN_LONG(uv_version());
}

PHP_METHOD(UVUtil, versionString) {
    RETURN_STRING(uv_version_string(), 1);
}

PHP_METHOD(UVUtil, errorMessage) {
    long err;
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &err) == FAILURE) {
        return;
    }
    RETURN_STRING(uv_strerror(err), 1);
}

