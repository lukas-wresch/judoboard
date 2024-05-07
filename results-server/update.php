<?php
//phpinfo();

//$parsed = yaml_parse(file_get_contents("php://input"));
$parsed = json_decode(file_get_contents("php://input"));

var_dump($parsed);

echo "ok";

?>