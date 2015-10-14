--TEST--
Check for UVResolver
--FILE--
<?php
$class = 'UVResolver';
include('error_handler.php');
$idle = new UVResolver(new stdClass());
?>
--EXPECT--
UVResolver ok
