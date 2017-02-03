--TEST--
Check for UVTime once
--FILE--
<?php 
$loop = new UVLoop();
$timer = new UVTimer($loop);
$time = time();
$timer->start(function($timer) use($timer, $time){
    $timediff = time() - $time;
    echo "timer alerm after $timediff secs\n";
}, 2000);
$loop->run();
$timer->stop();
?>
--EXPECT--
timer alerm after 2 secs
