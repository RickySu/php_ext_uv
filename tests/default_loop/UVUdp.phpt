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

function recvCallback($server, $clientIP, $clientPort, $data, $flag){
    $server->sendTo($clientIP, $clientPort, $data);
}

function sendCallback($server, $clientIP, $clientPort, $status){
    echo "Server Send ";
    var_dump($status == 0);
    $server->close();
}

function errorCallback(){
}

$loop = UVLoop::defaultLoop();
$server = new UVUdp($loop);
echo "Server Bind: {$server->bind($host, $port)}\n";
echo "{$server->getSockname()}:{$server->getSockport()}\n";
echo "setCallback: {$server->setCallback('recvCallback', 'sendCallback', 'errorCallback')}\n";
$loop->run();
?>
--EXPECT--
Server Bind: 0
127.0.0.1:8888
setCallback: 0
Server Send bool(true)
Client Echo Recv bool(true)
