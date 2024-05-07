<?php

$parsed = yaml_parse(file_get_contents("php://input"));

var_dump($parsed);

?>