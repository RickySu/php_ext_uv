--TEST--
Check for UVTimer
--FILE--
<?php
$class = 'UVTimer';
include('error_handler.php');
$idle = new UVTimer(new stdClass());
?>
--EXPECT--
UVTimer ok
