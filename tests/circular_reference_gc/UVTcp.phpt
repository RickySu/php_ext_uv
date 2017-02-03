--TEST--
Check for UVTcp
--FILE--
<?php
$loop = new UVLoop();
$tcp = new UVTcp($loop);
$tcp->listen('127.0.0.1', 54321, function($tcp2, $status) use($tcp){
    echo "on connect $status\n";
    var_dump($tcp === $tcp2);
    $client = $tcp->accept();
    $peerHostPortOK = "{$client->getPeername()}:{$client->getPeerport()}" != ":";
    echo "client connected: $peerHostPortOK\n";
    $client->setCallback(function($clientTest, $data) use($client){
        echo "server recv: $data\n";
        $client->write($data);
    }, function($clientTest, $status, $len) use($tcp, $client){
        echo "server write:$len\n";
        $client->shutdown(function($clientTest) use($tcp, $client){
            echo "client shutdown end\n";
            $client->close();
            $tcp->close();
        });
    }, function($clientTest, $status) use($client){
        echo "server recv error\n";
        $client->close();

    });
});
echo "server: {$tcp->getSockname()}:{$tcp->getSockport()}\n";

$tcp2 = new UVTcp($loop);
$tcp2->connect('127.0.0.1', 54321, function($tcp2, $status){
    $tcp2->setCallback(function($tcp2Test, $data) use($tcp2){
        echo "Server response: $data\n";
    }, function($tcp2Test, $status, $len) use($tcp2){
        echo "client send $len\n";
    }, function($tcp2Test, $status) use($tcp2){
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
