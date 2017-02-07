THIRDPARTY_BUILD_DIR="$srcdir/thirdparty/build"

PHP_ADD_INCLUDE("$srcdir/thirdparty/libuv/include")

PHP_ARG_ENABLE(php_ext_uv, whether to enable php_ext_uv support,
Make sure that the comment is aligned:
[ --enable-php_ext_uv Enable php_ext_uv support])

PHP_ARG_WITH(openssl, for OpenSSL support in event,
[  --with-openssl Include OpenSSL support], yes, no)

PHP_ARG_WITH(openssl-dir, for OpenSSL installation prefix,
[  --with-openssl-dir[=DIR] openssl installation prefix], no, no)

if test "$PHP_PHP_EXT_UV" != "no"; then

  MODULES="
      php_ext_uv.c
      src/uv_loop.c
      src/uv_signal.c
      src/uv_timer.c
      src/uv_udp.c
      src/uv_tcp.c
      src/uv_resolver.c
      src/uv_idle.c
      src/uv_util.c
      src/uv_pipe.c
      src/ssl_verify.c
  "

  dnl {{{ --with-event-openssl
  if test "$PHP_OPENSSL" != "no"; then
      test -z "$PHP_OPENSSL" && PHP_OPENSSL=no
        
      if test -z "$PHP_OPENSSL_DIR" || test $PHP_OPENSSL_DIR == "no"; then
          PHP_OPENSSL_DIR=yes
      fi
                      
      PHP_SETUP_OPENSSL(PHP_EXT_UV_SHARED_LIBADD, [
              AC_DEFINE(HAVE_OPENSSL,1,[ ])
          ], [
              AC_MSG_ERROR([OpenSSL libraries not found. 
                                                
                           Check the path given to --with-openssl-dir and output in config.log)
              ])
      ])                          
      MODULES="$MODULES src/uv_ssl.c"      
      PHP_ADD_LIBRARY(ssl, PHP_EXT_UV_SHARED_LIBADD)
  fi
  dnl }}}


  PHP_NEW_EXTENSION(php_ext_uv, $MODULES, $ext_shared)
  
fi

PHP_EXT_UV_SHARED_LIBADD="-lrt -lpthread $PHP_EXT_UV_SHARED_LIBADD"
PHP_ADD_MAKEFILE_FRAGMENT([Makefile.thirdparty])

shared_objects_php_ext_uv="$THIRDPARTY_BUILD_DIR/lib/libuv.a $shared_objects_php_ext_uv"
PHP_SUBST(PHP_EXT_UV_SHARED_LIBADD)
