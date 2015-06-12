--TEST--
Check for php_ext_uv presence
--FILE--
<?php 
$signal = new UVSignal();
echo "php_ext_uv extension is ".($signal?'':'un')."available";
?>
--EXPECT--
php_ext_uv extension is available