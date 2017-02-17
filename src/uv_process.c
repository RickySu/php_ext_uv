#include "uv_process.h"

CLASS_ENTRY_FUNCTION_D(UVProcess){
    REGISTER_CLASS_WITH_OBJECT_NEW(UVProcess, createUVProcessResource);
    OBJECT_HANDLER(UVProcess).offset = XtOffsetOf(uv_process_ext_t, zo);
    OBJECT_HANDLER(UVProcess).clone_obj = NULL;
    OBJECT_HANDLER(UVProcess).free_obj = freeUVProcessResource;
    zend_declare_property_null(CLASS_ENTRY(UVProcess), ZEND_STRL("loop"), ZEND_ACC_PRIVATE);
}

static zend_object *createUVProcessResource(zend_class_entry *ce) {
    uv_process_ext_t *resource;
    resource = ALLOC_RESOURCE(uv_process_ext_t);
    zend_object_std_init(&resource->zo, ce);
    object_properties_init(&resource->zo, ce);    
    resource->zo.handlers = &OBJECT_HANDLER(UVProcess);
    return &resource->zo;
}

void freeUVProcessResource(zend_object *object) {
    uv_process_ext_t *resource;
    resource = FETCH_RESOURCE(object, uv_process_ext_t);
    zend_object_std_dtor(&resource->zo);
}

PHP_METHOD(UVProcess, __construct){
    zval *loop = NULL;
    zval *self = getThis();

    uv_process_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_process_ext_t);
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "|z", &loop)) {
        return;
    }

    if(NULL == loop || ZVAL_IS_NULL(loop)){
        resource->loop = uv_default_loop();
        return;
    }

    zend_update_property(CLASS_ENTRY(UVProcess), self, ZEND_STRL("loop"), loop);
    resource->loop =  FETCH_UV_LOOP();
}

PHP_METHOD(UVProcess, spawn){
    char **args;
    zval *self = getThis();
    zval *zargs, *tmp;
    HashTable *htargs;
    int i, n, retval;
    uv_process_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_process_ext_t);
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "a", &zargs)) {
        return;
    }
    
    htargs = Z_ARRVAL_P(zargs);
    n = zend_array_count(htargs);
    args = ecalloc(n + 1, sizeof(char *));
    for(i=0; i<n; i++){
        if((tmp = zend_hash_index_find(htargs, i)) != NULL && Z_TYPE_P(tmp) == IS_STRING){
            args[i] = estrndup(Z_STRVAL_P(tmp), Z_STRLEN_P(tmp));            
        }
        else{
            php_error_docref(NULL, E_ERROR, "string type is required");
        }
    }
    object_init_ex(return_value, CLASS_ENTRY(UVWorker));
    retval = make_worker(resource->loop, args, return_value);
    if(retval){
        freeStringArray(args);
        RETURN_LONG(retval);
    }
    freeStringArray(args);
}

static void freeStringArray(char *args[]) {
    int i = 0;
    for(;;){
        if(args[i] == NULL){
            break;
        }
        efree(args[i++]);
    }
    efree(args);
}