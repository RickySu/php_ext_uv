--TEST--
Check for UVSignal
--FILE--
<?php
$class = 'UVSignal';
include('error_handler.php');
$idle = new UVSignal(new stdClass());
?>
--EXPECT--
UVSignal ok
