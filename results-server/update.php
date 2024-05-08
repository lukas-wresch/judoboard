<?php

include "common.php";

//$parsed = yaml_parse(file_get_contents("php://input"));
$json = json_decode(file_get_contents("php://input"));

var_dump($json);


sqlite_query("CREATE TABLE `schedule` ("
			."`id`     INTEGER PRIMARY KEY AUTOINCREMENT,"
			."`mat_id` INTEGER,"
			."`white`  TEXT,"
			."`blue`   TEXT,"
			."`state`  INTEGER,"
			."`winner` INTEGER,"
			."`score`  INTEGER,"
			."`time`   INTEGER,"
			."`match_table` TEXT,"
			."`tag`    INTEGER"
			.")");

sqlite_query("DELETE FROM `schedule`;");

foreach ($json->schedule as $match)
{
	sqlite_query("INSERT INTO `schedule` (`mat_id`, `white`, `blue`, `state`, `winner`, `score`, `time`, `match_table`, `tag`) VALUES ('$match->mat_id', '$match->white', '$match->blue', '$match->state', '$match->winner', '$match->score', '$match->time', '$match->match_table', '$match->tag');");
}

echo "ok";

?>