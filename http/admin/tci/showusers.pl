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
        or (print "Couldn't connect to $tci_host:$tci_port<BR>$@<BR>" and die);

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

## my $row_count = 0;
my $dev_count = 0;
my $usr_count = 0;
my $usr_tot_count = 0;
my @row = ();

my ($device, $username, $device_port, $phone, $session_ip);

if (substr($answer, 0, 2) eq 'Ok') {

  print $socket "showusers\n";
  shutdown($socket, 1);

  while ($answer = <$socket>) {

    ## Save for debuging
    ## print "$answer\n";
    ## $row_count++;

    @row = split("\t", $answer);
    if ($row[0] eq '#:') {
      if ($row[1] eq 'Device') {
        $dev_count++;
        $usr_count = 0;
        print "<TR><TD colspan=5 bgcolor=#F5D2AF<font color=#099D3D><B>$row[1] $row[2]</B></font></TD></TR>";
      }
      else {
        if (substr($row[1], 0, 5) eq "Begin") {
          print '<TABLE border="1" cellpadding="5" cellspacing="2">';
          print "<TR><TD colspan=5 bgcolor=#F4A984><B>List of active users.</B></TD></TR>";
        }
        if (substr($row[1], 0, 3) eq 'End') {
          printf "<TR><TD colspan=5 bgcolor=#F4A984><B>Total:</B> %d user(s) on %d device(s).</TD></TR>", $usr_tot_count, $dev_count;
          print '</TABLE>';
        }
      }
    }
    else {
      $usr_count++;
      $usr_tot_count++;
      $username = $row[0];
      $device_port = $row[1];
      $phone = $row[2];
      $session_ip = $row[3];
      print "<TR bgcolor=#EFD6E1><TD align=right>$usr_count</TD><TD>$username</TD><TD>$device_port</TD><TD>$phone</TD><TD>$session_ip</TD></TR>";
    }
  }
}
close($socket);
print '<HR><BR>';
print '</PRE></body></html>';
