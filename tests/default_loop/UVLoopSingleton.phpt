--TEST--
Check for UVLoop Singleton
--FILE--
<?php
$loop = UVLoop::defaultLoop();
echo "UVLoop ".($loop === UVLoop::defaultLoop()?'ok':'fail');
?>
--EXPECT--
UVLoop ok