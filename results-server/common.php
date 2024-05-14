<?php


$db = new SQLite3("database.db");
if (!$db)
	die("Error: Could not connect to database");


function sqlite_query($query)
{
	global $db;
    return $db->query($query);
	//return $result->fetchArray(SQLITE3_ASSOC);
}

?>