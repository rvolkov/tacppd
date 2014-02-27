#!/usr/bin/perl

######################
use CGI qw(:standard);
use Pg;
use Time::Local;
use Time::localtime; 
use lib "./lib/";
use My_Sub;
$| = 1;
######################

#--- Local variables -------
my $query = new CGI;
my ($conn,$result,$error,$row,$cmd);

#--- Global variables -------
our %init = ();
do "./config/lpmx_config.pl"  || die;

#--- Language ---
my $selectlang = $query->param("SelectLang");                                 # set language
$selectlang = $query->cookie("my_language") unless $selectlang;          # language from cookies
$selectlang = $init{DefaultLang} unless $selectlang;                     # default language
$selectlang = substr("$selectlang",0,100) if length("$selectlang") > 100;

#--- Global variables -------
our %dict = ();
do "./dictionary/$selectlang/my_common.pl"  || die;

my $ip  =  $query->param("ip");
my $start_date  =  $query->param("d");
my $stop_date  = $start_date + 86399;
my $tablename = get_tablename_from_date($start_date);
my ($sp_yyyy,$sp_mm,$sp_dd,$sp_hh) = get_year_month_etc_from_date($start_date);
my $dd = $sp_dd;

print header();
print "<HEAD><TITLE>$ip [$sp_yyyy-$sp_mm-$sp_dd]</TITLE>
      <link rel='stylesheet' type='text/css' href='./lib/lpmx.css'>
      <SCRIPT language='JavaScript' src='./lib/lpmx.js'></SCRIPT>
      <META http-equiv='cache control' content='no-cache'></HEAD>";
print"<BODY background='./images/bts.gif'>";

my ($bytes_in,$bytes_out,$name,$formprint);


$conn = Pg::connectdb("host=$init{prim_dbhost} dbname=$init{prim_dbname} user=$init{prim_dbuser} password=$init{prim_dbpassword}");
if (PGRES_CONNECTION_OK == $conn->status) {

    $result = $conn->exec("SELECT name, SUM(bytes_in), SUM(bytes_out) FROM $tablename WHERE  ip='$ip' AND currenttime BETWEEN $start_date AND $stop_date GROUP BY name");
    $name = $result->getvalue(0, 0);
    $bytes_in = $result->getvalue(0, 1);
    $bytes_out = $result->getvalue(0, 2);

    $bytes_in = sprintf("%2.2f",$bytes_in/1024/1024); 
    $bytes_out = sprintf("%2.2f",$bytes_out/1024/1024); 
    print"DATE: <b>$sp_yyyy-$sp_mm-$sp_dd</b> &nbsp; ";
    print"IP: <b>$ip</b>";
    print"<br>";
    print"$dict{USERNAME}: <b>$name</b> &nbsp; ";
    print"MBytes IN: <b><font color=blue>$bytes_in</font></b> &nbsp; ";
    print"MBytes OUT: <b><font color=red>$bytes_out</font></b> <br> ";
    #------------------------------------
    print"<TABLE class=listtable>";
    print"<TR class=listtable>";
     print"<TD class=nocolor><center>Hour/Traffic</TD>";
     print"<TD class=blue><center>Bytes IN</TD>";
     print"<TD class=blue><center>MBytes IN</TD>";
     print"<TD class=red><center>Bytes OUT</TD>";
     print"<TD class=red><center>MBytes OUT</TD>";
    print"</TR>";
    #------------------------------------
    $result = $conn->exec("SELECT SUM(bytes_in) AS bytes_in,SUM(bytes_out) AS bytes_out, date_part('hour',currenttime) AS currenttime FROM $tablename WHERE ip='$ip' AND currenttime BETWEEN $start_date AND $stop_date  GROUP BY date_part('hour',currenttime) ORDER BY currenttime");

    my $spider = 0;
    my $flag = 0;
    my $hh = '';
    for(;;){
        $spider++;
        ($sp_yyyy,$sp_mm,$sp_dd,$sp_hh) = get_year_month_etc_from_date($start_date);
        #--- вывал, если пошел другой день -----
        last if $sp_dd ne $dd; 

        print"<TR class=listtable>";
        print"<TD class=nocolor>$sp_yyyy-$sp_mm-$sp_dd : $sp_hh</TD>";

          if($flag == 0){
              ($bytes_in,$bytes_out,$hh) = $result->fetchrow;
              $hh = sprintf("%02d", $hh);
          }

          if($hh eq $sp_hh){
              print"<TD class=blue align=right>$bytes_in</TD>";
              $formprint = sprintf("%2.2f",$bytes_in/1024/1024); 
              print"<TD class=blue align=right>$formprint</TD>";

              print"<TD class=red align=right>$bytes_out</TD>";
              $formprint = sprintf("%2.2f",$bytes_out/1024/1024); 
              print"<TD class=red align=right>$formprint</TD>";
              $flag = 0;
          }else{
              print"<TD class=blue align=right>-</TD>";
              print"<TD class=blue align=right>-</TD>";
              print"<TD class=red align=right>-</TD>";
              print"<TD class=red align=right>-</TD>";
              $flag = 1;
          }

        print"</TR>";

        $start_date += 3600;
        last if $spider > 25;
    }
    print"</TABLE><br>";
    print"<form><center><input type='button' name='submit' value=$dict{cmdClose} class='MiniCmdGr'  onClick='window.close()'></form>";

    $conn->reset;

}else{
    $error = $conn->errorMessage;
    print"$dict{ConnectError} <font color=red>$error</font>";
}
print"</BODY></HTML>";



##################################################
sub get_tablename_from_date{
   my ($time) = @_;
   my($obj_time) = localtime($time);
   my $yyyymm = sprintf("nf_tot_%04d%02d", 
                     $obj_time->year() + 1900,
                     $obj_time->mon() + 1
              );
   return($yyyymm); 
}
