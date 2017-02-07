--TEST--
Check for UVPipe
--FILE--
<?php
$socketName = 'uvpipe.test.sock';
if(file_exists($socketName)){
    unlink($socketName);
}
$loop = UVLoop::defaultLoop();
$pipe = new UVPipe(null);
$pipe->listen($socketName, function($pipe, $status){
    echo "on connect $status\n";
    $client = $pipe->accept();
    $client->setCallback(function($clientTest, $data) use($client){
        echo "server recv: $data\n";
        $client->write($data);
    }, function($clientTest, $status, $len) use($pipe, $client){
        echo "server write:$len\n";
        $client->shutdown(function($clientTest) use($pipe, $client){
            echo "client shutdown end\n";
            $client->close();
            $pipe->close();
        });
    }, function($clientTest, $status) use($client){
        echo "server recv error\n";
        $client->close();
    });
});
echo "server: {$pipe->getSockname()}\n";

$pipe2 = new UVPipe($loop);
$pipe2->connect($socketName, function($pipe2, $status){
    $pipe2->setCallback(function($pipe2Test, $data) use($pipe2){
        echo "Server response: $data\n";
    }, function($pipe2Test, $status, $len) use($pipe2){
        echo "client send $len\n";
    }, function($pipe2Test, $status) use($pipe2){
        echo "client recv error\n";
        $pipe2->close();
    });
    $pipe2->write("client send some data");
});
$loop->run();
?>
--EXPECT--
server: uvpipe.test.sock
on connect 0
client send 21
server recv: client send some data
server write:21
client shutdown end
Server response: client send some data
client recv error
