<?php
set_error_handler(function($errno, $errstr, $errfile, $errline) use($class){         
    $status = $errstr == "Argument 1 passed to $class::__construct() must be an instance of UVLoop, instance of stdClass given"?'ok':'fail';
    echo "$class $status";
    die;
});