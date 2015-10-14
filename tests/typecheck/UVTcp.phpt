--TEST--
Check for UVTcp
--FILE--
<?php
$class = 'UVTcp';
include('error_handler.php');
$idle = new UVTcp(new stdClass());
?>
--EXPECT--
UVTcp ok
