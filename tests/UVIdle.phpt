--TEST--
Check for UVIdle
--FILE--
<?php
$count = 0;
$loop = new UVLoop();
$idle = new UVIdle($loop);
$idle->start(function($idle) use(&$count){
    if($count++ > 10){
        $idle->stop();
    }   
});
$loop->run();
if($count > 10){
    echo "UVIdle ok";
}
?>
--EXPECT--
UVIdle ok
