#!/usr/bin/perl

######################
use CGI qw(:standard);
use POSIX;
use locale;
use Pg;
use lib "./lib/";
use My_Sub;
$| = 1;
######################

#--- Local variables -------
my $query = new CGI;
my ($conn,$result,$error,$row,$temp,$cmd,$sec_conn);

#--- Init local variables ------
my ($selectlang,$switch,$pagenumber) = ('','','');
my ($unid,$accid,$phone,$nas,$port,$time) = ('','','','','','');

#--- Global variables -------
our %init = ();
do "./config/lpmx_config.pl" || die;

#--- Language ---
$selectlang = $query->cookie("my_language");                               # language from cookies
$selectlang = $init{DefaultLang} unless $selectlang;                       # default language
$selectlang = substr("$selectlang",0,100) if length("$selectlang") > 100;

#--- Global variables -------
our %dict = ();
do "./dictionary/$selectlang/my_common.pl"  || die;
my $lpath = "./dictionary/$selectlang/".(split(/\//,$ENV{SCRIPT_NAME}))[-1];
do "$lpath" if -e "$lpath";

#--- Page ---
$pagenumber  =  $query->param("p");
$pagenumber = 1 if  $pagenumber eq '' || $pagenumber=~ /[^0-9]/;
$pagenumber = substr("$pagenumber",0,4) if length("$pagenumber") > 4;

#--- Switch ---
$switch = $query->param("w"); # sWitch
$switch = 1 if $switch eq '' || $switch=~ /[^0-9]/;
$switch = substr("$switch",0,2) if length("$switch") > 2;

#--- Get variables ---
$unid = $query->param('unid');
$accid = $query->param('accid');
$phone = $query->param('phone');
$delflag = $query->param('delflag');
$nas = $query->param('nas');
$port = $query->param('port');
$time = $query->param('time');

#--- remove whitespace at the end and begin of the line, replace whitespaces by one whitespace  ---
$unid = join(" ", split " ", $unid);
$accid = join(" ", split " ", $accid);
$phone = join(" ", split " ", $phone);
$port = join(" ", split " ", $port);
$nas = join(" ", split " ", $nas);
$time = join(" ", split " ", $time);

#--- Null row ------
if ($accid eq ''){
    print $query->redirect("./acc.pl?p=$pagenumber&w=$switch") ; 
    exit;
}

$conn = Pg::connectdb("host=$init{prim_dbhost} dbname=$init{prim_dbname} user=$init{prim_dbuser} password=$init{prim_dbpassword}");
if (PGRES_CONNECTION_OK == $conn->status) {

    $sec_conn = Pg::connectdb("host=$init{sec_dbhost} dbname=$init{sec_dbname} user=$init{sec_dbuser} password=$init{sec_dbpassword}");
    if (PGRES_CONNECTION_OK == $sec_conn->status) {

        #--- new record -
        if ($unid eq ''){

            $result = $conn->exec("SELECT MAX(unid) FROM tacppd_v1_acc;");
            $error = $conn->errorMessage unless PGRES_TUPLES_OK eq $result->resultStatus;
            &print_save_error_and_exit("$dict{ERROR20}",$conn,$sec_conn) if $error ne '';
            $unid = $result->getvalue(0, 0);
            $unid = 0 if $unid eq '';
            $unid ++;

            ######>>> INSERT NEW USER ##############
            $cmd = "INSERT INTO tacppd_v1_acc(accid,phone,nas,port,time,unid) 
                    VALUES('$accid','$phone','$nas','$port','$time',$unid);";


        #--- edit record -
        }else{

            if($delflag == 1){
                ######>>> DELETE USER ##############
                $cmd = "DELETE FROM tacppd_v1_acc 
                          WHERE unid = $unid;";
         
            }else{
         
                ######>>> UPDATE USER ##############
                $cmd = "UPDATE tacppd_v1_acc 
                         SET accid='$accid', phone='$phone', nas='$nas', port='$port', time='$time'
                          WHERE unid = $unid;";
            }


        } #-- edit/new

        ######>>> COMMON ##############

        #--- Secondary ---
        $result = $sec_conn->exec("BEGIN WORK;");
        $error = $sec_conn->errorMessage unless PGRES_TUPLES_OK eq $result->resultStatus;
        &print_save_error_and_exit("$dict{ERROR17}",$conn,$sec_conn) if $error ne '';

        #--- Secondary ---
        $result = $sec_conn->exec("$cmd");
        $error = $sec_conn->errorMessage unless PGRES_TUPLES_OK eq $result->resultStatus;
        &print_save_error_and_exit("$dict{ERROR_5}"."$error",$conn,$sec_conn) if $error ne '';

        #--- Primary ---
        $result = $conn->exec("$cmd");
        $error = $conn->errorMessage unless PGRES_TUPLES_OK eq $result->resultStatus;
        if ($error ne ''){

            #--- Secondary ---
            ########################
            $sec_conn->exec("ROLLBACK WORK;");
            ########################
            &print_save_error_and_exit("$dict{ERROR_5}"."$error",$conn,$sec_conn);

        }

        #--- Secondary ---
        $result = $sec_conn->exec("COMMIT WORK;");
        $error = $sec_conn->errorMessage unless PGRES_TUPLES_OK eq $result->resultStatus;
        &print_save_error_and_exit("$dict{ERROR18}",$conn,$sec_conn) if $error ne '';


        $conn->reset;
        $sec_conn->reset;

        print $query->redirect("./acc.pl?p=$pagenumber&w=$switch"); 


    }else{
       $error = $sec_conn->errorMessage;
       &print_save_error_and_exit("$dict{SecConnectError}"."$error",$conn,$sec_conn);
    }

    $conn->reset;

}else{
    &print_my_header();
    $error = $conn->errorMessage;
    print"$dict{ConnectError} <font color=red>$error</font>";
}
print"</BODY></HTML>";


########################################################################
########################################################################
########################################################################

sub print_save_error_and_exit{
   my($message,$conn,$sec_conn) = @_;

   $conn->reset if $conn;
   $sec_conn->reset if $sec_conn;

   &print_my_header();
     &print_top_table();
          print"<p><font color=red><b>$dict{ERROR}!</b><br>";
          print"$message<p>";
          print"$dict{PRESS1} <input TYPE='button' VALUE=$dict{PRESS_CMD} ONCLICK='history.back(-1)' class=MiniCmd> $dict{PRESS2}";
     &print_bottom_table();
   exit;
}

sub print_top_table{
    print"<FORM>";
    print"<TABLE class=Reg>";
    print"<TR>";
     print"<TD class=TitleTable0></TD>";
     print"<TD VALIGN=TOP>";
        print"<br><SPAN class=big>$dict{TitlePages}</SPAN>";
        print"<p>";
}

sub print_bottom_table{
        print"<p>";
     print"</TD>";
    print"</TR>";
    print"</TABLE>";
   print"</font>";
    print"</FORM>";
   print"</BODY></HTML>";
}

sub print_my_header{
    print header();
    print "<HEAD><TITLE>$dict{TitlePages}</TITLE>
          <link rel='stylesheet' type='text/css' href='./lib/lpmx.css'>
          <SCRIPT language='JavaScript' src='./lib/lpmx.js'></SCRIPT>
          <META http-equiv='cache control' content='no-cache'></HEAD>";
    print"<BODY background='./images/bts.gif'>";
}

