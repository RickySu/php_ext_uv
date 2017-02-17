#ifndef _PHP_EXT_UV_H
#define _PHP_EXT_UV_H

#ifdef HAVE_CONFIG_H
    #include "config.h"
#endif

#ifdef ZTS
    #warning php_ext_uv module will *NEVER* be thread-safe
    #include <TSRM.h>
#endif

#include <php.h>
#include <uv.h>
#include "common.h"
#include "src/util.h"

extern zend_module_entry php_ext_uv_module_entry;

DECLARE_CLASS_ENTRY(UVLoop);
DECLARE_CLASS_ENTRY(UVSignal);
DECLARE_CLASS_ENTRY(UVTimer);
DECLARE_CLASS_ENTRY(UVUdp);
DECLARE_CLASS_ENTRY(UVTcp);
DECLARE_CLASS_ENTRY(UVPipe);
DECLARE_CLASS_ENTRY(UVIdle);
DECLARE_CLASS_ENTRY(UVProcess);
DECLARE_CLASS_ENTRY(UVWorker);
#ifdef HAVE_OPENSSL
DECLARE_CLASS_ENTRY(UVSSL);
#endif
DECLARE_CLASS_ENTRY(UVResolver);
DECLARE_CLASS_ENTRY(UVUtil);
#endif
