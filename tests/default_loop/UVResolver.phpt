--TEST--
Check for UVResolver
--FILE--
<?php
$result = array();
$loop = UVLoop::defaultLoop();
$resolver = new UVResolver($loop);
$resolver->getnameinfo('198.41.0.4', function($status, $host, $service) use(&$result){
    $result['198.41.0.4'] = "$status $host $service";
});
$resolver->getaddrinfo('a.root-servers.net', null, function($status, $host) use(&$result){
    $result['a.root-servers.net'] = "$status $host";
});
$loop->run();
echo "{$result['198.41.0.4']}\n";
echo "{$result['a.root-servers.net']}\n";
?>
--EXPECT--
0 a.root-servers.net 0
0 198.41.0.4
