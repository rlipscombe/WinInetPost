<html>
<?
foreach ($_FILES as $key=>$value) {
	print "<h1>" . $key . "</h1>\n";
	$files = $value;
	
	print "<table border=\"1\">\n";
	foreach ($files as $k=>$v) {
		print "<tr><td>" . $k . "</td>\n";
		print "    <td>" . $v . "</td></tr>\n";
	}
	print "</table>\n";
}
?>
</html>
