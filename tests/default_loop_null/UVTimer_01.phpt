--TEST--
Check for UVTime once
--FILE--
<?php 
$loop = UVLoop::defaultLoop();
$timer = new UVTimer(null);
$time = time();
$timer->start(function($timer2) use($timer, $time){
    $timediff = time() - $time;
    echo "timer alerm after $timediff secs\n";
    echo "timer object is ";
    var_dump($timer === $timer2);    
}, 2000);
$loop->run();
$timer->stop();
?>
--EXPECT--
timer alerm after 2 secs
timer object is bool(true)