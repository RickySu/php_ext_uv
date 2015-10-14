--TEST--
Check for UVIdle
--FILE--
<?php
$class = 'UVIdle';
include('error_handler.php');
$idle = new UVIdle(new stdClass());
?>
--EXPECT--
UVIdle ok
