--TEST--
Check for UVResolver
--FILE--
<?php
$rootServerA = 'a.root-servers.net';
$rootServerAIP = '198.41.0.4';
$result = array();
$loop = UVLoop::defaultLoop();
$resolver = new UVResolver(null);
$resolver->getnameinfo($rootServerAIP, function($status, $host, $service) use(&$result, $rootServerAIP){
    $result[$rootServerAIP] = "$status $host $service";
});
$resolver->getaddrinfo($rootServerA, null, function($status, $host) use(&$result, $rootServerA){
    $result[$rootServerA] = "$status $host";
});
$loop->run();
echo "{$result[$rootServerAIP]}\n";
echo "{$result[$rootServerA]}\n";
?>
--EXPECT--
0 a.root-servers.net 0
0 198.41.0.4
