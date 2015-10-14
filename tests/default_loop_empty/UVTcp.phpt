--TEST--
Check for UVTcp
--FILE--
<?php
$loop = UVLoop::defaultLoop();
$tcp = new UVTcp();
$tcp->listen('127.0.0.1', 54321, function($tcp2, $status) use($tcp){
    echo "on connect $status\n";
    var_dump($tcp === $tcp2);
    $client = $tcp->accept();
    $peerHostPortOK = "{$client->getPeername()}:{$client->getPeerport()}" != ":";
    echo "client connected: $peerHostPortOK\n";
    $client->setCallback(function($client, $data){
        echo "server recv: $data\n";
        $client->write($data);
    }, function($client, $status, $len) use($tcp){
        echo "server write:$len\n";
        $client->shutdown(function($client) use($tcp){
            echo "client shutdown end\n";
            $client->close();
            $tcp->close();
        });
    }, function($client, $status){
        echo "server recv error\n";
        $client->close();

    });
});
echo "server: {$tcp->getSockname()}:{$tcp->getSockport()}\n";

$tcp2 = new UVTcp();
$tcp2->connect('127.0.0.1', 54321, function($tcp2, $status){
    $tcp2->setCallback(function($tcp2, $data){
        echo "Server response: $data\n";
    }, function($tcp2, $status, $len){
        echo "client send $len\n";
    }, function($tcp2, $status){
        echo "client recv error\n";
        $tcp2->close();
    });
    $tcp2->write("client send some data");
});
$loop->run();
?>
--EXPECT--
server: 127.0.0.1:54321
on connect 0
bool(true)
client connected: 1
client send 21
server recv: client send some data
server write:21
client shutdown end
Server response: client send some data
client recv error
