--TEST--
Check for UVSignal
--FILE--
<?php 
$loop = new UVLoop();
$signal = new UVSignal($loop);
$signal->start(function($signal2, $signno) use($signal){
    echo "receive signal\n";
    echo "signal object is ";
    var_dump($signal === $signal2);
    $signal->stop();
    echo "signal stop";
}, SIGUSR1);
$signal->start(function($signal2, $signno) use($signal){
    echo "receive signal\n";
    echo "signal object is ";
    var_dump($signal === $signal2);
    $signal->stop();
    echo "signal stop";
}, SIGUSR1);
if($pid = pcntl_fork()){
    $signal->stop();
    posix_kill($pid, SIGUSR1);
    pcntl_wait($status);
}
else{
    $loop->run();
}
?>
--EXPECT--
receive signal
signal object is bool(true)
signal stop