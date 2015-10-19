<?php
try{
    new $class(new stdClass());
}
catch(TypeError $e){
    $status = $e->getMessage() == "Argument 1 passed to $class::__construct() must be an instance of UVLoop, instance of stdClass given"?'ok':'fail';
    echo "$class $status";
}