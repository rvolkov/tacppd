#!/usr/bin/perl

use locale;
use IO::Socket;

$| = 1;

do "./tci.conf";

#--------HTTP header-----------------
print "Content-type: text/html\n\n";

print <<JS_END;

<HTML>
<HEAD>
<META http-equiv='cache control' content='no-cache'>
</HEAD>
<BODY bgcolor='#FFFFFF' LINK='#000000' VLINK='#000000' ALINK='#A00040'>

JS_END

$socket = IO::Socket::INET->new(PeerAddr => $tci_host,
                                PeerPort => $tci_port,
                                Proto    => "tcp",
                                Type     => SOCK_STREAM)
        or (print "Couldn't connect to $tci_host:$tci_port<BR>$@<BR></BODY></HTML>" and die);

$socket->autoflush(1);
# send username/password
print $socket "$tci_login\n$tci_pwd\n";
my $answer = <$socket>;
(undef, $answer) = split("\t", $answer);

print '<font size=3><PRE>';
print '<A HREF=index.html>Back</A>';
print '<HR><BR>';

print "Connection status: $answer<BR>";
print '<HR><BR>';


my @row = ();
my @row2 = ();

#my ($device, $username, $device_port, $phone, $session_ip);

if (substr($answer, 0, 2) eq 'Ok') {

  print $socket "cf_write\n";
  shutdown($socket, 1);

  while ($answer = <$socket>) {

    @row = split("\t", $answer);
    @row2 = split("\n", $answer);
    if ($row[0] eq '#:') {
        if (substr($row[1], 0, 5) eq "Begin") {
          print '<TABLE border="1" cellpadding="5" cellspacing="2">';
          print "<TR><TD colspan=5 bgcolor=#F4A984><B>Write tacppd config.</B></TD></TR>";
	  print "<TR bgcolor=#EFD6E1><TD>";
        }
        if (substr($row[1], 0, 3) eq 'End') {
	  print "</TD></TR>";
          printf "<TR><TD colspan=5 bgcolor=#F4A984><B>Process completed</B></TD></TR>";
          print '</TABLE>';
        }
    }
    else {
      $outstr = $row2[0];
#     print "<TR bgcolor=#EFD6E1><TD align=right>$outstr</TD></TR>";
      print "$outstr<BR>";
    }
  }
}
close($socket);
print '<HR><BR>';
print '</PRE></body></html>';
