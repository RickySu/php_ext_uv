#!/bin/bash
sudo apt-get -y install check pkg-config

currentpwd=`pwd`

thirdparty="$currentpwd/thirdparty"

cd "$thirdparty/r3" && \
./autogen.sh && \
CFLAGS="-O2 -fPIC" ./configure --prefix="$thirdparty/build" && make clean install

cd "$thirdparty/libuv" && \
./autogen.sh && \
CFLAGS="-O2 -fPIC" ./configure --prefix="$thirdparty/build" && make clean install

cd $currentpwd

phpize
./configure
make all 
sudo make install
sudo echo "extension=php_ext_uv.so" >> `php --ini | grep "Loaded Configuration" | sed -e "s|.*:\s*||"`
