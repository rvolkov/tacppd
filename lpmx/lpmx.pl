#!/usr/bin/perl
 
use Time::Local;
use Time::localtime;
use Pg;

my $parsertime = localtime(time);
$p_month = $parsertime->mon() + 1;
$p_day = $parsertime->mday();
$p_year = $parsertime->year() + 1900;
$p_hour = $parsertime->hour();
$p_min = $parsertime->min();
$p_sec = $parsertime->sec();

# Проверка повторного запуска
if (-e "jobstart.mx") {
  open (ERRLOG,">>lpmx.err") || die print "ERRLOG open:$!\n";
  print ERRLOG "Can't start at $p_day.$p_month.$p_year $p_hour:$p_min:$p_sec\n";
  close (ERRLOG) || die print "ERRLOG close:$!\n";
  exit;
}

open (CHKSTART,">>jobstart.mx") || die print "CHKSTART open:$!\n";
print CHKSTART time;
close (CHKSTART) || die print "CHKSTART close:$!\n";

# lpmx.conf
my ($server, $abspath, $mailto, $currentperiod);
open (CONF,"<lpmx.conf") || die print "lpmx.conf open:$!\n";                
while (<CONF>) {
  s/\r//g;
  s/\n//g;
  s/^\s+//;
  @conf_record = split(/\s+/);
  if ($conf_record[0] eq 'Server:') {
     $server = $conf_record[1];
    next;
  }
  if ($conf_record[0] eq 'S_Server:') {
     $s_server = $conf_record[1];
    next;
  }
  if ($conf_record[0] eq 'AbsPath:') {
    $abspath = $conf_record[1];
    next;
  }
  if ($conf_record[0] eq 'MailTo:') {
    $mailto = $conf_record[1];
    next;
  }
  if ($conf_record[0] eq 'CurrentPeriod:') {
    $currentperiod = $conf_record[1];
    next;
  }
  if ($conf_record[0] eq 'TrafficLimit:') {
    $traffic_limit = $conf_record[1];
    next;
  }
}
close (CONF) || die print "lpmx.conf close:$!\n";

# Check new log
my $nextperiod = GoYYYYMM($currentperiod, 1);
my $logNext = "$abspath/$nextperiod.log";
if (-e "$logNext") {
  open (CONF,">lpmx.conf") || die print "lpmx.conf open:$!\n";
  printf CONF "%14s $server\n", 'Server:';
  printf CONF "%14s $s_server\n", 'S_Server:';
  printf CONF "%14s $abspath\n", 'AbsPath:';
  printf CONF "%14s $mailto\n", 'MailTo:';
  printf CONF "%14s $nextperiod\n", 'CurrentPeriod:';
  printf CONF "%14s $traffic_limit\n", 'TrafficLimit:';
  close (CONF) || die print "lpmx.conf close:$!\n";
}

my $tstName = "$abspath/$currentperiod.mx.tst"; 
open (TSTLOG,">>$tstName") || die print "$currentperiod.mx.tst open:$!\n";

my $pos = 0;
my %users = ();
my %add_inf = ();
my ($bytes_recive, $mb_recive, $bytes_send, $mb_send, $conn_count, $sum_elapsed);
my ($tot_bytes_recive, $tot_mb_recive, $tot_bytes_send, $tot_mb_send, $tot_conn_count, 
    $tot_sum_elapsed, $tot_users);

my $totName = "$abspath/$currentperiod.tot"; 
if (-e "$abspath/$currentperiod.tot") {
  open (TOTLOG,"<$totName") || die print "$currentperiod.tot open:$!\n";
  $first_str = <TOTLOG>;
  $first_str =~ s/\r//g;
  $first_str =~ s/\n//g;
  $first_str = substr($first_str, 43);
  $first_str =~ s/#//g;
  $first_str =~ s/\s//g;
  $pos = $first_str;

  while (<TOTLOG>) {
    if (substr($_, 0, 1) ne '#') {
      ($username, $bytes, $mb_recive, $bytes_send, $mb_send, $conn_count, $sum_elapsed) 
       = split(/\s+/);
      $users{$username} = $bytes;
      push (@{$add_inf{$username}}, ($bytes_send, $conn_count, $sum_elapsed));
    }
  }
  close (TOTLOG) || die print TSTLOG "TOT close:$!\n";
}

my %month_num = qw(Jan 01
                   Feb 02
                   Mar 03
                   Apr 04
                   May 05
                   Jun 06
                   Jul 07
                   Aug 08
                   Sep 09
                   Oct 10
                   Nov 11
                   Dec 12);

my $total_row = 0;
my $stop_row = 0;
my $update_row = 0;
my $start_row = 0;

my $unknown_row = 0;
my $shell_row = 0;

my $insert_row = 0;
my $error_row = 0;

# my $backup_count = 0;
# ?????????????????
my $double_row = 0;


my @dup_record;

my $logName = "$abspath/$currentperiod.log";
open (DUPLOG,"<$logName") || die print TSTLOG "DUP open:$!\n";                

print TSTLOG "- START in $p_day.$p_month.$p_year $p_hour:$p_min:$p_sec $server  -\n";
print TSTLOG "- START on $logName at $pos -\n";

seek(DUPLOG, $pos, 0);

while (<DUPLOG>) {
  
  $pos = tell(DUPLOG);
  
  @dup_record = split(/\t/);
  
  $stop_row++ if $dup_record[5] eq 'stop';
  $update_row++ if $dup_record[5] eq 'update';
  $start_row++ if $dup_record[5] eq 'start';

  $total_row++;

  if ($dup_record[5] eq 'stop') {
    if ($dup_record[2] eq 'unknown') {
      $unknown_row++;
      next;
    }

    $start_time = '';
    $service = '';
    $task_id = 0;
    $ip = '';
    $timezone = '';
    $protocol = '';
    $elapsed = 0;
    $bytes_in = 0;
    $bytes_out = 0;
    $cause = 0;
    $cause_ext = 0;
    $rx_speed = 0;
    $tx_speed = 0;
    $phone = '';
    $town_from = '';
    $timefrom = 'S';
    $ciscotime = 0;

    $currenttime = $dup_record[0];

    $nas = $dup_record[1];
    $username = $dup_record[2];
    $port = $dup_record[3];
    $phone = $dup_record[4];
      
    for ($count = 5; $count < @dup_record; $count++) {

      if ($dup_record[$count] =~ m/task_id=/) {
        $task_id = $dup_record[$count];
        $task_id =~ s/task_id=//;
        next;
      }
      if ($dup_record[$count] =~ m/service=/) {
        $service = $dup_record[$count];
        $service =~ s/service=//;

        if ($service eq 'shell') {
          $shell_row++;
          goto SKIP_SHELL;
        }
        $service = substr($service,0,3);
        next;
      }
      if ($dup_record[$count] =~ m/protocol=/) {
        $protocol = $dup_record[$count];
        $protocol =~ s/protocol=//;
        next;
      }
      if ($dup_record[$count] =~ m/addr=/) {
        $ip = $dup_record[$count];
        $ip =~ s/addr=//;
        next;
      }
      if ($dup_record[$count] =~ m/timezone=/) {
        $timezone = $dup_record[$count];
        $timezone =~ s/timezone=//;
        next;
      }
      if ($dup_record[$count] =~ m/elapsed_time=/) {
        $elapsed = $dup_record[$count];
        $elapsed =~ s/elapsed_time=//;
        next;
      }
      if ($dup_record[$count] =~ m/bytes_in=/) {
        $bytes_in = $dup_record[$count];
        $bytes_in =~ s/bytes_in=//;
        next;
      }
      if ($dup_record[$count] =~ m/bytes_out=/) {
        $bytes_out = $dup_record[$count];
        $bytes_out =~ s/bytes_out=//;
        next;
      }

      if ($dup_record[$count] =~ m/start_time=/) {
        $ciscotime = $dup_record[$count];
        $ciscotime  =~ s/start_time=//;
        next;
      }
      if ($dup_record[$count] =~ m/disc-cause=/) {
        $cause = $dup_record[$count];
        $cause =~ s/disc-cause=//;
        next;
      }
      if ($dup_record[$count] =~ m/disc-cause-ext=/) {
        $cause_ext = $dup_record[$count];
        $cause_ext =~ s/disc-cause-ext=//;
        next;
      }
      if ($dup_record[$count] =~ m/nas-rx-speed=/) {
        $rx_speed = $dup_record[$count];
        $rx_speed =~ s/nas-rx-speed=//;
        $rx_speed = 0 if $rx_speed > 115200;
        next;
      }
      if ($dup_record[$count] =~ m/nax-tx-speed=/) {
        $tx_speed = $dup_record[$count];
        $tx_speed =~ s/nax-tx-speed=//;
        $tx_speed = 0 if $tx_speed > 115200;
        next;
      }
    }

    $year = substr($currenttime, 20, 4);
    $month_eng = substr($currenttime, 4, 3);
    $mon = $month_num{$month_eng};
    $mday = substr($currenttime, 8, 2);
    $mday =~ s/\s//; 
    
    $hour = substr($currenttime, 11, 2);
    $min = substr($currenttime, 14, 2);
    $sec = substr($currenttime, 17, 2);
    
    $currenttime =  "$year-$mon-$mday $hour:$min:$sec";

    if ($ciscotime == 0) {
      $stop_time = $currenttime;

      $mon -= 1;
      $year -= 1900;
      $st_time = timelocal($sec,$min,$hour,$mday,$mon,$year);
      $st_time -= $elapsed;

    }
    else {
      $timefrom = 'C';

      $st_time = $ciscotime;

      $stop_time = $st_time + $elapsed;
      $stop_time = localtime($stop_time);

      $p_year = $stop_time->year() + 1900;
      $p_month = $stop_time->mon() + 1;
      $p_day = $stop_time->mday();
      $p_hour = $stop_time->hour();
      $p_min = $stop_time->min();
      $p_sec = $stop_time->sec();
      $stop_time =  "$p_year-$p_month-$p_day $p_hour:$p_min:$p_sec";
    }

    $start_time = localtime($st_time);

    $p_year = $start_time->year() + 1900;
    $p_month = $start_time->mon() + 1;
    $p_day = $start_time->mday();
    $p_hour = $start_time->hour();
    $p_min = $start_time->min();
    $p_sec = $start_time->sec();

    $start_time =  "$p_year-$p_month-$p_day $p_hour:$p_min:$p_sec";

    $DoubleRecord = 0;
    if ($DoubleRecord == 0) {

      $username =~ s/\s+$//;
         
      if (exists($users{$username})) {
        $users{$username} += $bytes_out / 1024;

        $add_inf{$username}[0] += $bytes_in / 1024;
        $add_inf{$username}[1]++;
        $add_inf{$username}[2] += $elapsed;
      }
      else {
        $users{$username} = $bytes_out / 1024;
        push (@{$add_inf{$username}}, ($bytes_in / 1024, 1, $elapsed));
      }

# my ($mb_recive, $bytes_send, $mb_send, $conn_count, $sum_elapsed);
# my ($tot_bytes_recive, $tot_mb_recive, $tot_bytes_send, $tot_mb_send, $tot_conn_count, 
#    $tot_sum_elapsed, $tot_users);

      #'$stop_time','$start_time','$service',$task_id,'$ip',
      #'$protocol',$elapsed,$bytes_in,$bytes_out,$cause,$cause_ext,$rx_speed,$tx_speed,
      #'$phone','$currenttime','$timefrom','$timezone'

      #'$nas','$port',$ciscotime
      # $double_row++;
      # print TSTLOG "Double $currenttime $nas $port $username\n";

    } # !Double record

  } # Stop record
  SKIP_SHELL:

} # !Eof(DUPLOG)
close (DUPLOG) || die print TSTLOG "TST close:$!\n";

print TSTLOG '-' x 60, "\n";
printf TSTLOG "  Start: %10d\n", $start_row;
printf TSTLOG " Update: %10d\n", $update_row;
print TSTLOG '-' x 60, "\n";
printf TSTLOG "   Stop: %10d\n", $stop_row;
printf TSTLOG "Unknown: %10d\n", $unknown_row;
printf TSTLOG "  Shell: %10d\n", $shell_row;

# printf TSTLOG " Double: %10d\n", $double_row;
print TSTLOG '-' x 60, "\n";
printf TSTLOG "  Total: %10d\n", $total_row;
print TSTLOG '-' x 60, "\n";

## Почитаем из базы данных
ReadFromPg($server);
ReadFromPg($s_server);
#

my $tmpName = "$abspath/$currentperiod.tot.tmp"; 
open (TOTTMP,">$tmpName") || die print "$currentperiod.tot.tmp open:$!\n";
printf TOTTMP "# LastRead = %02d.%02d.%4d %02d:%02d:%02d # Shift = $pos #\n", 
              $p_day,$p_month,$p_year, $p_hour,$p_min,$p_sec;
printf TOTTMP "## %-29s %20s %10s %20s %10s %5s %20s ##\n",
              'USERNAME', 'KB(Recive)', 'MB(Recive)', 'KB(Send)', 'MB(Send)', 'CONN', 'ELAPSED';
my $over_limit = 0;
my $limit_count = 1;
foreach $key (sort {$users{$b} <=> $users{$a}}(keys %users)) {
  $mb = $users{$key}/1024;
  if ($limit_count) {
    if ($mb < $traffic_limit) {
      print TOTTMP "# Limit = $traffic_limit Mb # Users over limit = $over_limit #\n";
      $limit_count = 0;
    }
    $over_limit++;
  }
  printf TOTTMP "%-32s %20.4f %10.2f", $key, $users{$key}, $mb ;
  $tot_users++;
  # $tot_bytes_recive += $users{$key}; Очень большое число
  $tot_mb_recive += $mb;

  $mb = $add_inf{$key}[0]/1024;
  printf TOTTMP " %20.4f %10.2f %5d %20d\n", $add_inf{$key}[0], $mb, 
                $add_inf{$key}[1], $add_inf{$key}[2] ;

  # $tot_bytes_send += $add_inf{$key}[0]; Очень большое число
  $tot_mb_send += $mb;
  $tot_conn_count += $add_inf{$key}[1]; 
  $tot_sum_elapsed += $add_inf{$key}[2]; 

}
printf TOTTMP "%-32s %20s %10.2f %20s %10.2f %5d %20d#\n", "# Total: users $tot_users ",
              ' ', $tot_mb_recive, ' ', $tot_mb_send,
              $tot_conn_count, $tot_sum_elapsed;

close (TOTTMP) || die print TSTLOG "$currentperiod.tot.tmp close:$!\n";
rename ("$tmpName","$totName");
close (TSTLOG) || die print "$currentperiod.tst close:$!\n";

unlink("jobstart.mx");

#--- End of mainproc ---

sub GoYYYYMM {
  my $month_count = substr($_[0], 0, 4) * 12 + substr($_[0], 4, 2) + $_[1];
  my $month_part = sprintf("%02s", $month_count % 12);
  
  if ($month_part eq '00') {
    $month_part = '12';
    $month_count -= 12;
  }

  return int($month_count / 12).$month_part;
}

sub ReadFromPg {
  my ($conn, $cmd, $result, $updt_cmd, $updt_result, $n_checked_record);
  $conn = Pg::connectdb("host=$_[0] dbname=lpmx user=tacacs password=TacacS+");

  print TSTLOG "Read from PostgreSQL($_[0])\n";
  if ($conn->status == PGRES_CONNECTION_OK) {
    $cmd = "SELECT OID, username, bytes_out, bytes_in, elapsed FROM tacppd_v1_log_$currentperiod where dcheck IS NULL;";
    $result = $conn->exec($cmd);
    $n_checked_record = $result->ntuples;
    print TSTLOG "Number of checked records: $n_checked_record\n";
    while (@pg_row = $result->fetchrow) {

      $username = $pg_row[1];
      $bytes_out = $pg_row[2];
      $bytes_in = $pg_row[3];
      $elapsed = $pg_row[4];

      if (exists($users{$username})) {
        $users{$username} += $bytes_out / 1024;

        $add_inf{$username}[0] += $bytes_in / 1024;
        $add_inf{$username}[1]++;
        $add_inf{$username}[2] += $elapsed;
      }
      else {
        $users{$username} = $bytes_out / 1024;
        push (@{$add_inf{$username}}, ($bytes_in / 1024, 1, $elapsed));
      }

      $updt_cmd = "UPDATE tacppd_v1_log_$currentperiod SET dcheck = now() where OID = $pg_row[0];"; 
      $updt_result = $conn->exec($updt_cmd);
    }
  }
  else {print TSTLOG "Connection error!\n";}
  print TSTLOG '-' x 60, "\n";
}
