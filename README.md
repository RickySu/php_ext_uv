php_ext_uv
================

[![Build Status](https://travis-ci.org/RickySu/php_ext_uv.svg?branch=master)](https://travis-ci.org/RickySu/php_ext_uv)

php_ext_uv is a [libuv](https://github.com/joyent/libuv) 
extension for [PHP](http://php.net/)
with high performance asynchronous I/O.

## Feature highlights

* Asynchronous TCP and UDP sockets

* Asynchronous DNS resolution

* Signal handling

* SSL support

* TLS [SNI](https://en.wikipedia.org/wiki/Server_Name_Indication) support

## Build Requirement

* autoconf
* automake
* check
* pkg-config
* pcre


Example
--------------------


```php
// TCP Echo Server
$loop = new UVLoop();
$server = new UVTcp($loop);
$server->listen($host, $port, function($server){
    $client = $server->accept();
    $client->setCallback(function($client, $recv){
        //on receive
        $client->write($recv);
    }, function($client, $status){
        //on data sent
        $client->close();
    }, function(){
        //on error maybe client disconnect.
        $client->close();
    });
});
$loop->run();
```

```php
//SSL Echo Server
$loop = new UVLoop();
$server = new UVSSL($loop);
$server->setCert(file_get_contents("server.crt"));    //PEM format
$server->setPrivateKey(file_get_contents("server.key"));  //PEM format
$server->listen($host, $port, function($server){
    $client = $server->accept();
    $client->setSSLHandshakeCallback(function($client){
        echo "ssl handshake ok\n";
    });
    $client->setCallback(function($client, $recv){
        //on receive if ssl handshake finished.
        //otherwise you won't receive any data before ssl handshake finished
        $client->write($recv);
    }, function($client, $status){
        //on data sent
        $client->close();
    }, function(){
        //on error maybe client disconnect.
        $client->close();
    });
});
$loop->run();
```

[Server Name Indication(SNI)](http://en.wikipedia.org/wiki/Server_Name_Indication)

```php
//SSL Echo Server with SNI support
//SNI is an extension to the TLS 
$loop = new UVLoop();
$server = new UVSSL($loop, UVSSL::SSL_METHOD_TLSV1, 2); //with 2 certs
$server->setCert(file_get_contents("server0.crt"), 0);    //PEM format cert 0
$server->setPrivateKey(file_get_contents("server0.key"), 0);  //PEM format cert 0
$server->setCert(file_get_contents("server1.crt"), 1);    //PEM format cert 1
$server->setPrivateKey(file_get_contents("server1.key"), 1);  //PEM format cert 1
$server->setSSLServerNameCallback(function($servername){ //regist Server Name Indication callback
    if($servername == 'server1'){ 
        return 1; //use cert1
    } 
    return 0;  //default use cert0
});
$server->listen($host, $port, function($server){
    $client = $server->accept();
    $client->setSSLHandshakeCallback(function($client){
        echo "ssl handshake ok\n";
    });
    $client->setCallback(function($client, $recv){
        //on receive if ssl handshake finished.
        //otherwise you won't receive any data before ssl handshake finished
        $client->write($recv);
    }, function($client, $status){
        //on data sent
        $client->close();
    }, function(){
        //on error maybe client disconnect.
        $client->close();
    });
});
$loop->run();
```

## LICENSE

This software is released under MIT License.
