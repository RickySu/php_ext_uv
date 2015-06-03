#!/bin/bash
sudo apt-get install gdb
phpize
./configure --quiet
make all install
echo "extension=mongo.so" >> `php --ini | grep "Loaded Configuration" | sed -e "s|.*:\s*||"`
