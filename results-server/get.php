<?php

include "common.php";

//$query = sqlite_query("SELECT * FROM `match_tables`;");
$query = sqlite_query("SELECT `schedule`.`white`, `schedule`.`blue`, `schedule`.`mat_id`, `schedule`.`match_table`, `schedule`.`state`, `mats`.`name` AS `mat_name`, `match_tables`.`name` AS `match_table_name` FROM `schedule` LEFT JOIN `mats` ON `schedule`.`mat_id` = `mats`.`id` LEFT JOIN `match_tables` ON `schedule`.`match_table` = `match_tables`.`id`;");

echo "<table rules='all' border='1'>";
echo "<tr><th>Weiss</th><th>Blau</th><th>Kampfliste</th><th>Matte</th><th>Status</th></tr>";

while ($row = $query->fetchArray(SQLITE3_ASSOC))
{
	//var_dump($row);
	echo "<tr><td>" . $row['white'] . "</td><td>" . $row['blue'] . '</td>';
	echo '<td><a href="#" onclick="function(event){event.preventDefault(); Update_Table(\'' . $row['match_table'] .'\');}">' . $row['match_table_name'] . '</a></td>';

	if ($row['mat_name'] != "")
		echo '<td>'.$row['mat_name'].'</td>';
	else
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