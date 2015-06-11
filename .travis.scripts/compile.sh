#!/bin/bash
sudo apt-get -y install check pkg-config
phpize
./configure
make all 
sudo install
sudo echo "extension=mongo.so" >> `php --ini | grep "Loaded Configuration" | sed -e "s|.*:\s*||"`
