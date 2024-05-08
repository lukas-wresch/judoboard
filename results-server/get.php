<?php

include "common.php";

$query = sqlite_query("SELECT * FROM `schedule`;");

echo "<table rules='all' border='1'>";
echo "<tr><th>white</th><th>blue</th><th>table</th><th>mat</th><th>state</th></tr>";

while ($row = $query->fetchArray(SQLITE3_ASSOC))
{
	//var_dump($row);
	echo "<tr><td>" . $row['white'] . "</td><td>" . $row['blue'] . '</td><td>' . $row['match_table'] . '</td><td>' . $row['mat_id'] . '</td><td>' . $row['state'] . '</td></tr>';
}

echo "</table>"

?>