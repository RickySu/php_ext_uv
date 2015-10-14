--TEST--
Check for UVUdp
--FILE--
<?php
$class = 'UVUdp';
include('error_handler.php');
$idle = new UVUdp(new stdClass());
?>
--EXPECT--
UVUdp ok
