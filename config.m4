THIRDPARTY_BUILD_DIR="$srcdir/thirdparty/build"

PHP_ADD_INCLUDE("$srcdir/thirdparty/r3/include")
PHP_ADD_INCLUDE("$srcdir/thirdparty/libuv/include")

PHP_ARG_ENABLE(php_ext_uv, whether to enable php_ext_uv support,
Make sure that the comment is aligned:
[ --enable-php_ext_uv Enable php_ext_uv support])

if test "$PHP_PHP_EXT_UV" != "no"; then

  MODULES="
      php_ext_uv.c
      src/uv_loop.c
      src/uv_signal.c
      src/uv_timer.c
      src/uv_udp.c
      src/uv_tcp.c
  "
  PHP_NEW_EXTENSION(php_ext_uv, $MODULES, $ext_shared)
  
fi


PHP_ADD_MAKEFILE_FRAGMENT([Makefile.thirdparty])

PHP_EXT_UV_SHARED_DEPENDENCIES="$THIRDPARTY_BUILD_DIR/lib/libr3.a $THIRDPARTY_BUILD_DIR/lib/libuv.a"
EXTRA_LDFLAGS="$EXTRA_LDFLAGS $THIRDPARTY_BUILD_DIR/lib/libr3.a $THIRDPARTY_BUILD_DIR/lib/libuv.a"

PHP_SUBST(PHP_EXT_UV_SHARED_DEPENDENCIES)
