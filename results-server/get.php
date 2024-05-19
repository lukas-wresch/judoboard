<?php

include "common.php";

//$setup = sqlite_query("SELECT * FROM `setup`;")->fetchArray(SQLITE3_ASSOC);

$query = sqlite_query("SELECT * FROM `schedule`;");

echo "<table rules='all' border='1'>";
echo "<tr><th>white</th><th>blue</th><th>Kampfliste</th><th>Matte</th><th>Status</th></tr>";

while ($row = $query->fetchArray(SQLITE3_ASSOC))
{
	//var_dump($row);
	echo "<tr><td>" . $row['white'] . "</td><td>" . $row['blue'] . '</td><td>' . $row['match_table'] . '</td>';

	echo '<td>Matte '.$row['mat_id'].'</td>';

	if ($row['state'] == 0)
		echo '<td>In Vorbereitung</td>';
	else if ($row['state'] == 1)
		echo '<td>Am K&auml;mpfen</td>';
	else if ($row['state'] == 2)
		echo '<td>Beendet</td>';
	else
		echo '<td>- - -</td>';

	echo '</tr>';
}

echo "</table>"

?>