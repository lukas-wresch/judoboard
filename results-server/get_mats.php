<?php

header('Content-type: application/json');

include "common.php";

$query = sqlite_query("SELECT * FROM `mats`;");

$data = array();

while ($row = $query->fetchArray(SQLITE3_ASSOC))
{
	$new_data = array();
	$new_data['id']   = $row['id'];
	$new_data['name'] = $row['name'];

	$data[] = $new_data;
}

echo json_encode($data);

?>