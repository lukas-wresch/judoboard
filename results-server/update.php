<?php
//phpinfo();

//$parsed = yaml_parse(file_get_contents("php://input"));
$parsed = json_decode(file_get_contents("php://input"));

var_dump($parsed);


$db = new SQLite3("database.db");


function sqlite_query($query)
{
	global $db;
    return $db->query($query);
	//return $result->fetchArray(SQLITE3_ASSOC);
}


sqlite_query("CREATE TABLE schedule ("
			."id     INTEGER PRIMARY KEY,"
			."mat_id INTEGER,"
			."white  TEXT,"
			."blue   TEXT,"
			."state  INTEGER,"
			."winner INTEGER,"
			."score  INTEGER,"
			."time   INTEGER,"
			."match_table TEXT,"
			."tag    INTEGER"
			.")";

sqlite_query("DELETE FROM `schedule`;");

echo "ok";

?>