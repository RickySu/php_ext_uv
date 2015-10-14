--TEST--
Check for UVSSL
--FILE--
<?php
$class = 'UVSSL';
include('error_handler.php');
$idle = new UVSSL(new stdClass());
?>
--EXPECT--
UVSSL ok
