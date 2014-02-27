#!/usr/bin/perl
print "Content-Type: text/html\n";
print "\n";
print "<HTML>";
print "<HR>This is Perl script output<HR>\n";

$num = @ARGV;
print "Get $num arguments<BR>\n";

if($num > 0) {
  print "This is GET method<BR>\n";
} else {
  print "This is POST method<BR>\n";
  $sss = <STDIN>;
  print "Input from stdin = $sss";
}

foreach $aaa (@ARGV) {
	print "ARGUMENT: $aaa<BR>\n";
	$aaa = "bbb";
}

print "<P>\n";

foreach $aaa (@ENV) {
	print "ENV: $aaa<BR>\n";
	$aaa = "bbb";
}


print "<HR>\n";

print "</HTML>\n\n";
