--TEST--
Check for UVUdp
--FILE--
<?php
$host = '127.0.0.1';
$port = 8888;
$pid = pcntl_fork();
if($pid){
    usleep(100000);    

    $fp = stream_socket_client("udp://$host:$port", $errno, $errstr);
    $randomData = sha1(rand());
    fwrite($fp, $randomData);
    $recv = fread($fp, strlen($randomData));    
    fclose($fp);    
    pcntl_waitpid($pid, $status);
    echo "Client Echo Recv ";
    var_dump($randomData === $recv);    
    exit;
}

$loop = new UVLoop();
$server = new UVUdp($loop);
$bindResult = $server->bind($host, $port);
$setCallbackResult = $server->setCallback(
    function ($serverTest, $clientIP, $clientPort, $data, $flag) use($server){
        $server->sendTo($clientIP, $clientPort, $data);
        echo "Recv Server ";
        var_dump($serverTest === $server);
    },
    function ($serverTest, $clientIP, $clientPort, $status) use($server){
        echo "Server Send ";
        var_dump($status == 0);
        $server->close();
        echo "Send Server ";
        var_dump($serverTest === $server);
    },
    function ($serverTest) use($server){
    }
);

$setCallbackResult1 = $server->setCallback(
    function ($serverTest, $clientIP, $clientPort, $data, $flag) use($server){
        $server->sendTo($clientIP, $clientPort, $data);
        echo "Recv Server ";
        var_dump($serverTest === $server);
    },
    function ($serverTest, $clientIP, $clientPort, $status) use($server){
        echo "Server Send ";
        var_dump($status == 0);
        $server->close();
        echo "Send Server ";
        var_dump($serverTest === $server);
    },
    function ($serverTest) use($server){
    }
);

echo "Server Bind: $bindResult\n";
echo "{$server->getSockname()}:{$server->getSockport()}\n";
echo "setCallback: $setCallbackResult $setCallbackResult1\n";
$loop->run();
?>
--EXPECT--
Server Bind: 0
127.0.0.1:8888
setCallback: 0 -1
Recv Server bool(true)
Server Send bool(true)
Send Server bool(true)
Client Echo Recv bool(true)
