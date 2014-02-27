#!/usr/bin/perl
 
use Time::Local;
use Time::localtime;
use Pg;

#- CONFIG -#
my $LogPath = '/app/var/tacacs/lpmx';

my $nf_server = 'server1.net.com';
my $nf_dbname = 'lpmx';
my $nf_dbuser = 'tacuser';
my $nf_dbpswd = 'tacpwd';

my $nf_tnmask = 'tacppd_v1_nf_';
my $ck_tnmask = 'tacppd_v1_ck_';
#----------#

my $parsertime = localtime(time);
my $p_month = $parsertime->mon() + 1;
my $p_day = $parsertime->mday();
my $p_year = $parsertime->year() + 1900;
my $p_hour = $parsertime->hour();
my $p_min = $parsertime->min();
my $p_sec = $parsertime->sec();

if (-e "nfstart.mx") {
  open (ERRLOG,">>lpmx.err") || die print "ERRLOG open:$!\n";
  print ERRLOG "Can't start lpmx_nf at $p_day.$p_month.$p_year $p_hour:$p_min:$p_sec\n";
  close (ERRLOG) || die print "ERRLOG close:$!\n";
  exit;
}

open (CHKSTART,">>nfstart.mx") || die print "CHKSTART open:$!\n";
print CHKSTART $$;
close (CHKSTART) || die print "CHKSTART close:$!\n";

my $LogName = "$LogPath/nf_$p_year$p_month.tst"; 
open (LOG,">>$LogName") || die print "$LogName open:$!\n";

my $ConnString = "host=$nf_server dbname=$nf_dbname user=$nf_dbuser password=$nf_dbpswd";
ReadFromPg($ConnString);

close (LOG) || die "$LogName close:$!\n";
unlink("nfstart.mx");

#- End of mainproc -

sub ReadFromPg {
  my ($conn, $cmd, $result, $n_record);
  my $table_count = 0;
  my ($tn, $tn_year, $tn_month, $tn_day, $tn_hour);
  my $ck_result; 

  $conn = Pg::connectdb($_[0]);

  print LOG "Read from PostgreSQL($nf_server) at $p_day.$p_month.$p_year $p_hour:$p_min:$p_sec\n";
  if ($conn->status == PGRES_CONNECTION_OK) {
    $cmd = "SELECT tablename FROM pg_tables WHERE tablename LIKE '$nf_tnmask%' ORDER BY tablename";
    $result = $conn->exec($cmd);

    $n_record = $result->ntuples;

    print LOG "Number of tables: $n_record\n";
		++$table_count;
    while (@pg_row = $result->fetchrow) {
      if (++$table_count >= $n_record) {last;}
      $tn = unpack("x".length($nf_tnmask)." A10", $pg_row[0]);
      ($tn_year, $tn_month, $tn_day, $tn_hour) = unpack("A4 A2 A2 A2", $tn);
      $TotName = 'nf_tot_'.$tn_year.$tn_month;
      $cmd = "SELECT count(*) FROM pg_tables WHERE tablename = '$TotName'";
      $ck_result = $conn->exec($cmd);
      if ($ck_result->getvalue(0,0)) {
        $cmd = "INSERT INTO $TotName ";
      }
      else {
        $cmd = "CREATE TABLE $TotName AS ";
      }
      $cmd .= "SELECT name, ip, Sum(bytes_in) AS bytes_in, Sum(bytes_out) AS bytes_out, 
                      to_timestamp('$tn','YYYYMMDDHH24') AS currenttime
               FROM (SELECT u.name, t.destination AS ip, Sum(t.octets) AS bytes_in, 0 AS bytes_out
               FROM $pg_row[0] t, tacppd_v1_nfuser u 
               WHERE inet(t.destination) <<= u.usernet 
               GROUP BY u.name, t.destination
               UNION
               SELECT u.name, t.source AS ip, 0 AS bytes_in, Sum(t.octets) AS bytes_out
               FROM $pg_row[0] t, tacppd_v1_nfuser u 
               WHERE inet(t.source) <<= u.usernet
               GROUP BY u.name, t.source
               ) AS temptable GROUP BY name, ip;";
      $ck_result = $conn->exec($cmd);
      $cmd = "ALTER TABLE $pg_row[0] RENAME TO $ck_tnmask$tn";
      $ck_result = $conn->exec($cmd);
    }
  }
  else {print TSTLOG "Connection error!\n";}
  print LOG '-' x 60, "\n";
}
