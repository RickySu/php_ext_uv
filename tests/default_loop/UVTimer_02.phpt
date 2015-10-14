--TEST--
Check for UVTime repeat
--FILE--
<?php 
$loop = UVLoop::defaultLoop();
$timer = new UVTimer($loop);
$time = time();
$timer->start(function($timer2) use($timer, $time){
    $timediff = time() - $time;
    echo "timer alerm after $timediff secs\n";
    if($timediff>=5){
        $timer->stop();
    }
}, 2000, 1000);
$loop->run();
?>
--EXPECT--
timer alerm after 2 secs
timer alerm after 3 secs
timer alerm after 4 secs
timer alerm after 5 secs
