THIRDPARTY_BUILD_DIR="$srcdir/thirdparty/build"
PHP_ADD_INCLUDE($THIRDPARTY_BUILD_DIR/include)

PHP_ARG_ENABLE(php_ext_uv, whether to enable php_ext_uv support,
Make sure that the comment is aligned:
[ --enable-php_ext_uv Enable php_ext_uv support])

if test "$PHP_PHP_EXT_UV" != "no"; then

  MODULES="php_ext_uv.c src/uv_loop.c src/uv_signal.c src/uv_timer.c"
  PHP_NEW_EXTENSION(php_ext_uv, $MODULES, $ext_shared)
  
fi


PHP_ADD_MAKEFILE_FRAGMENT([Makefile.thirdparty])
PHP_ADD_LIBRARY_WITH_PATH(r3, $THIRDPARTY_BUILD_DIR/lib, R3_LIBADD)
PHP_ADD_LIBRARY_WITH_PATH(uv, $THIRDPARTY_BUILD_DIR/lib, UV_LIBADD)

PHP_MODULES="$THIRDPARTY_BUILD_DIR/lib/libr3.a $THIRDPARTY_BUILD_DIR/lib/libuv.a $PHP_MODULES"
LDFLAGS="$LDFLAGS $R3_LIBADD $UV_LIBADD"
