--TEST--
Check for UVResolver
--FILE--
<?php
$rootServerA = 'a.root-servers.net';
$rootServerAIP = '198.41.0.4';
$result = array();
$loop = new UVLoop();
$resolver = new UVResolver($loop);
$resolver->getnameinfo($rootServerAIP, function($status, $host, $service) use(&$result, $resolver, $rootServerAIP){
    $result[$rootServerAIP] = "$status $host $service";
});
$resolver->getaddrinfo($rootServerA, null, function($status, $host) use(&$result, $resolver, $rootServerA){
    $result[$rootServerA] = "$status $host";
});
$loop->run();
echo "{$result[$rootServerAIP]}\n";
echo "{$result[$rootServerA]}\n";
?>
--EXPECT--
0 a.root-servers.net 0
0 198.41.0.4
