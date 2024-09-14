<?php

include "common.php";

//$parsed = yaml_parse(file_get_contents("php://input"));
$json = json_decode(file_get_contents("php://input"));

var_dump($json);


sqlite_query("CREATE TABLE `setup` ("
			."`name`     TEXT,"
			."`language` INTEGER"
			.")");


sqlite_query("CREATE TABLE `mats` ("
			."`id`    INTEGER PRIMARY KEY,"
			."`name`  TEXT"
			.")");


sqlite_query("CREATE TABLE `match_tables` ("
			."`id`    TEXT PRIMARY KEY,"
			."`name`  TEXT,"
			."`html`  TEXT"
			.")");


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


sqlite_query("DELETE FROM `setup`;");

sqlite_query("INSERT INTO `setup` (`name`, `language`) VALUES ('$json->name', '1');");


sqlite_query("DELETE FROM `mats`;");

foreach ($json->mats as $mat)
{
	sqlite_query("INSERT INTO `mats` (`id`, `name`) VALUES ('$mat->id', '$mat->name');");
}


sqlite_query("DELETE FROM `match_tables`;");

foreach ($json->match_tables as $table)
{
	sqlite_query("INSERT INTO `match_tables` (`id`, `name`, `html`) VALUES ('$table->uuid', '$table->name', '$table->html');");
	echo "INSERT INTO `match_tables` (`id`, `name`, `html`) VALUES ('$table->uuid', '$table->name', '$table->html');\n";
}


sqlite_query("DELETE FROM `schedule`;");

foreach ($json->schedule as $match)
{
	sqlite_query("INSERT INTO `schedule` (`mat_id`, `white`, `blue`, `state`, `winner`, `score`, `time`, `match_table`, `tag`) VALUES ('$match->mat_id', '$match->white', '$match->blue', '$match->state', '$match->winner', '$match->score', '$match->time', '$match->match_table', '$match->tag');");
}

echo "ok";

?>