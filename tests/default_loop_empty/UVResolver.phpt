--TEST--
Check for UVResolver
--FILE--
<?php
$result = array();
$loop = UVLoop::defaultLoop();
$resolver = new UVResolver();
$resolver->getnameinfo('127.0.0.1', function($status, $host, $service) use(&$result){
    $result['127.0.0.1'] = "$status $host $service";
});
$resolver->getaddrinfo('localhost', null, function($status, $host) use(&$result){
    $result['localhost'] = "$status $host";
});
$loop->run();
echo "{$result['127.0.0.1']}\n";
echo "{$result['localhost']}\n";
?>
--EXPECT--
0 localhost 0
0 127.0.0.1
