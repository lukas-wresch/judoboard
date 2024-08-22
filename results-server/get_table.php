<?php

include "common.php";

$id = $_GET['id'];

$query = sqlite_query("SELECT * FROM `match_tables` WHERE `id` = '$id';");

while ($row = $query->fetchArray(SQLITE3_ASSOC))
{
	//var_dump($row);
	echo $row['html'];
}

?>