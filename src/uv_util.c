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
    RETURN_STRING(uv_version_string());
}

PHP_METHOD(UVUtil, errorMessage) {
    long err;
    if(zend_parse_parameters(ZEND_NUM_ARGS(), "l", &err) == FAILURE) {
        return;
    }
    RETURN_STRING(uv_strerror(err));
}

PHP_METHOD(UVUtil, cpuinfo) {
    zval retval, times;
    uv_cpu_info_t *info;
    int cpu_count;
    uv_cpu_info(&info, &cpu_count);
    array_init(&retval);
    array_init(&times);
    add_assoc_long(&times, "user", info->cpu_times.user);
    add_assoc_long(&times, "nice", info->cpu_times.nice);
    add_assoc_long(&times, "sys", info->cpu_times.sys);
    add_assoc_long(&times, "idle", info->cpu_times.idle);
    add_assoc_long(&times, "irq", info->cpu_times.irq);
    add_assoc_long(&retval, "count", cpu_count);
    add_assoc_string(&retval, "model", info->model);
    add_assoc_long(&retval, "speed", info->speed);
    add_assoc_zval(&retval, "times", &times);
    uv_free_cpu_info(info, cpu_count);
    RETURN_ZVAL(&retval, 0, 0);
}
