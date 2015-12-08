--TEST--
Check for UVSignal
--FILE--
<?php 
$loop = new UVLoop();
$signal = new UVSignal($loop);
$signal->start(function($signal, $signno){
    echo "receive signal\n";
    $signal->stop();
    echo "signal stop";
}, SIGUSR1);
if($pid = pcntl_fork()){
    posix_kill($pid, SIGUSR1);
    pcntl_wait($status);
}
else{
    $loop->run();
}
?>
--EXPECT--
receive signal
signal stop