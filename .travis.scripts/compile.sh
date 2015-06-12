#!/bin/bash
sudo apt-get -y install check pkg-config
thirdparty="`pwd`/thirdparty"

cd "$thirdparty/r3" && \
./autogen.sh && \
CFLAGS="-O2 -fPIC" ./configure --prefix="$thirdparty/build" && make clean install

cd "$thirdparty/libuv" && \
./autogen.sh && \
CFLAGS="-O2 -fPIC" ./configure --prefix="$thirdparty/build" && make clean install

phpize
./configure
make all 
sudo install
sudo echo "extension=php_ext_uv.so" >> `php --ini | grep "Loaded Configuration" | sed -e "s|.*:\s*||"`
