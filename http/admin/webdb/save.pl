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
my ($print_user,$save_time);

#--- Init local variables ------
my ($selectlang,$switch,$pagenumber,$search,$qr) = ('','','','','');
my ($username,$password,$dopen,$dclose,$avpid,$accid,$maxsess,$edituser,$action) = ('','','','','','',1,'','');

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

#--- Search ---
$search = $query->param("s");
$search =~ s/\s+$//;
$search = substr("$search",0,30) if length("$search") > 30;

#--- Quick return ---
$qr = $query->param("qr");

#--- Get variables ---
$username = $query->param('username');
$edituser = $query->param('en');
$password = $query->param('password');
$avpid = $query->param('avpid');
$accid = $query->param('accid');
$maxsess = $query->param('maxsess');
$dopen = $query->param('dopen');
$dclose = $query->param('dclose');
$action = $query->param('action');
$username =~ s/\s+$//;
$edituser =~ s/\s+$//;
$dclose =~ s/\s+$//;
$dopen =~ s/\s+$//;

$conn = Pg::connectdb("host=$init{prim_dbhost} dbname=$init{prim_dbname} user=$init{prim_dbuser} password=$init{prim_dbpassword}");
if (PGRES_CONNECTION_OK == $conn->status) {

      &print_save_error_and_exit("$dict{SAVE_ERROR4}",$conn,$sec_conn) if $password eq '';
      &print_save_error_and_exit("$dict{SAVE_ERROR5}",$conn,$sec_conn) if $password =~ /\s/;
      &print_save_error_and_exit("$dict{SAVE_ERROR6}",$conn,$sec_conn) if length("$password") > 50;
      &print_save_error_and_exit("$dict{SAVE_ERROR7}",$conn,$sec_conn) if $maxsess =~ /[^0-9]/;
      &print_save_error_and_exit("$dict{SAVE_ERROR7}",$conn,$sec_conn) if $maxsess < 0 || $maxsess > 99999;
      &print_save_error_and_exit("$dict{SAVE_ERROR8}",$conn,$sec_conn) if $action =~ /[^1|2]/;

      $dclose = '' if $action == 1;

      &print_save_error_and_exit("$dict{SAVE_ERROR9}",$conn,$sec_conn) unless $dopen =~ /^\d\d\/\d\d\/\d\d\d\d\ \d\d\:\d\d\:\d\d$/;
      $temp = get_num_date($dopen);
      &print_save_error_and_exit("$dict{SAVE_ERROR11}",$conn,$sec_conn) if $temp == -1;
      if ($dclose ne ''){
          &print_save_error_and_exit("$dict{SAVE_ERROR10}",$conn,$sec_conn) unless $dclose =~ /^\d\d\/\d\d\/\d\d\d\d\ \d\d\:\d\d\:\d\d$/;
          $temp = get_num_date($dclose);
          &print_save_error_and_exit("$dict{SAVE_ERROR12}",$conn,$sec_conn) if $temp == -1;
      }
      
      $result = $conn->exec("SELECT avpid FROM tacppd_v1_avp WHERE avpid = '$avpid' LIMIT 1 OFFSET 0");
      $error = $conn->errorMessage unless PGRES_TUPLES_OK eq $result->resultStatus;
      &print_save_error_and_exit($dict{ERROR_1},$conn,$sec_conn) if $error ne '';
      $row = $result->ntuples; 
      &print_save_error_and_exit("$dict{SAVE_ERROR13}",$conn,$sec_conn) if $row == 0;

      $result = $conn->exec("SELECT accid FROM tacppd_v1_acc WHERE accid = '$accid' LIMIT 1 OFFSET 0");
      $error = $conn->errorMessage unless PGRES_TUPLES_OK eq $result->resultStatus;
      &print_save_error_and_exit($dict{ERROR_1},$conn,$sec_conn) if $error ne '';
      $row = $result->ntuples; 
      &print_save_error_and_exit("$dict{SAVE_ERROR14}",$conn,$sec_conn) if $row == 0;

      $sec_conn = Pg::connectdb("host=$init{sec_dbhost} dbname=$init{sec_dbname} user=$init{sec_dbuser} password=$init{sec_dbpassword}");
      if (PGRES_CONNECTION_OK == $sec_conn->status) {

          #--- new user -
          if ($edituser eq ''){

              &print_save_error_and_exit("$dict{SAVE_ERROR1}",$conn,$sec_conn) if $username eq '';
              &print_save_error_and_exit("$dict{SAVE_ERROR2}",$conn,$sec_conn) if $username =~ /[^a-zA-Z0-9\-\.]/;
              &print_save_error_and_exit("$dict{SAVE_ERROR3}",$conn,$sec_conn) if length("$username") > 32;

              $result = $conn->exec("SELECT username FROM tacppd_v1_usr WHERE username='$username'");
              $error = $conn->errorMessage unless PGRES_TUPLES_OK eq $result->resultStatus;
              &print_save_error_and_exit("$dict{ERROR_1}"."$error",$conn,$sec_conn) if $error ne '';
              $row = $result->ntuples; 
              &print_save_error_and_exit("$dict{SAVE_ERROR15}",$conn,$sec_conn) if $row > 0;


              ######>>> INSERT NEW USER ##############
              $save_time = get_cur_datetime(time);
              $cmd = "INSERT INTO tacppd_v1_usr(username,password,dopen,";
              $cmd .= "dclose," if $dclose ne '';
              $cmd .= "avpid,accid,maxsess,dupdate) VALUES ('$username','\Q$password\E','$dopen',";
              $cmd .= "'$dclose'," if $dclose ne '';
              $cmd .= "'$avpid','$accid',$maxsess,'$save_time')";

             
          #--- edit user -
          }else{

              &print_save_error_and_exit("$dict{SAVE_ERROR3}",$conn,$sec_conn) if length("$edituser") > 32;

              $result = $conn->exec("SELECT date_part('epoch',dclose) FROM tacppd_v1_usr WHERE username='$edituser'");
              $error = $conn->errorMessage unless PGRES_TUPLES_OK eq $result->resultStatus;
              &print_save_error_and_exit("$dict{ERROR_1}"."$error",$conn,$sec_conn) if $error ne '';
              $row = $result->ntuples; 
              &print_save_error_and_exit("$dict{SAVE_ERROR16}",$conn,$sec_conn) if $row == 0;

              $temp = $result->getvalue(0, 1);
              $temp =~ s/\s+$//;
              if($dclose ne '' && $temp ne ''){
                  $temp = get_cur_datetime($temp);
                  $dclose = $temp;
              }

              ######>>> UPDATE USER ##############
              $save_time = get_cur_datetime(time);
              $cmd = "UPDATE tacppd_v1_usr SET password='\Q$password\E',";
              if ($dclose ne ''){
                  $cmd .= "dclose='$dclose',";
              }else{
                  $cmd .= "dclose=NULL,";
              }
              $cmd .= "avpid='$avpid',accid='$accid',maxsess=$maxsess,dupdate='$save_time' ";
              $cmd .= "WHERE username='$edituser'";

              #--- Secondary Check Username ---
              $result = $sec_conn->exec("SELECT username FROM tacppd_v1_usr WHERE username='$edituser'");
              $error = $sec_conn->errorMessage unless PGRES_TUPLES_OK eq $result->resultStatus;
              &print_save_error_and_exit("$dict{ERROR_4}"."[Secondary Server] $error",$conn,$sec_conn) if $error ne '';
              $row = $result->ntuples; 
              &print_save_error_and_exit("$dict{ERROR19}",$conn,$sec_conn) if $row == 0;

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


          #--- Exit ---
          if($qr eq ''){
              print $query->redirect("./edit.pl?p=$pagenumber&w=$switch&s=$search"); 
          }else{
              if ($edituser ne ''){
                  $username = $edituser;
                  &print_registration_status($dict{SAVE_REGEDIT});
              }else{
                  &print_registration_status($dict{SAVE_REGNEW});
              }
          }


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

sub print_registration_status{
   my ($message) = @_;

   &print_my_header();

     &print_top_table();
      print"<SPAN class=regmessag>$message</SPAN><p>";

      print"<TABLE>";
      print"<TR>";
      print"<TD VALIGN=TOP><b>$dict{USERNAME}</b>:</TD>";
      print"<TD VALIGN=TOP><INPUT TYPE='text' readonly=\"on\" name=r1 VALUE=\"$username\" class=PlaneTextBig></TD>";
      print"</TR>";

      print"<TR>";
      print"<TD VALIGN=TOP><b>$dict{EDIT_PASSWORD}</b>:</TD>";
      print"<TD VALIGN=TOP><INPUT TYPE='text' readonly=\"on\" name=r2 VALUE=\"$password\" class=PlaneTextBig></TD>";
      print"</TR>";

      print"<TR>";
      print"<TD VALIGN=TOP colspan=2>&nbsp;</TD>";
      print"</TR>";

      print"<TR>";
      print"<TD VALIGN=TOP><b>$dict{DOPEN}</b>:</TD>";
      print"<TD VALIGN=TOP><INPUT TYPE='text' readonly=\"on\" name=r3 VALUE=\"$dopen\" class=PlaneTextBig></TD>";
      print"</TR>";

      if ($dclose ne ''){
          print"<TR>";
          print"<TD VALIGN=TOP><b>$dict{DCLOSE}</b>:</TD>";
          print"<TD VALIGN=TOP><INPUT TYPE='text' readonly=\"on\" name=r4 VALUE=\"$dclose\" class=PlaneTextBig></TD>";
          print"</TR>";
      }

      print"<TR>";
      print"<TD VALIGN=TOP colspan=2>&nbsp;</TD>";
      print"</TR>";

      print"<TR>";
      print"<TD VALIGN=TOP><b>$dict{AVP}</b>:</TD>";
      print"<TD VALIGN=TOP><INPUT TYPE='text' readonly=\"on\" name=r5 VALUE=\"$avpid\" class=PlaneTextBig></TD>";
      print"</TR>";

      print"<TR>";
      print"<TD VALIGN=TOP><b>$dict{ACC}</b>:</TD>";
      print"<TD VALIGN=TOP><INPUT TYPE='text' readonly=\"on\" name=r6 VALUE=\"$accid\" class=PlaneTextBig></TD>";
      print"</TR>";

      print"<TR>";
      print"<TD VALIGN=TOP><b>$dict{EDIT_MAXSESS}</b>:</TD>";
      print"<TD VALIGN=TOP><INPUT TYPE='text' readonly=\"on\" name=r7 VALUE=\"$maxsess\" class=PlaneTextBig></TD>";
      print"</TR>";

      print"<TR>";
      print"<TD VALIGN=TOP colspan=2>&nbsp;</TD>";
      print"</TR>";

      print"<TR>";
      print"<TD VALIGN=TOP colspan=2>[<A HREF=./edit.pl?p=$pagenumber&w=$switch&s=$search class=Link>$dict{LINK_BACK}</A>]</TD>";
      print"</TR>";

      print"<TR>";
      print"<TD VALIGN=TOP colspan=2>&nbsp;</TD>";
      print"</TR>";

      print"<TR>";
      print"<TD VALIGN=TOP colspan=2>$dict{SAVE_PRINT1} [
              <IMG SRC='./images/print.gif' class='img' ALIGN='bottom' ALT='' WIDTH='24' HEIGHT='22' BORDER='0' onClick='window.print();' STYLE='filter: gray();' onmouseover='this.style.filter=\"none\";' onmouseout='this.style.filter=\"gray()\";'>
                                     ] $dict{SAVE_PRINT2}</TD>";
      print"</TR>";

      print"<TR>";
      print"<TD VALIGN=TOP colspan=2><small><SPAN class=Link>$dict{SAVE_CONF}</SPAN></small></TD>";
      print"</TR>";

      print"<TR>";
      print"<TD VALIGN=TOP colspan=2>&nbsp;</TD>";
      print"</TR>";

      print"</TR>";
      print"</TABLE>";

      &print_bottom_table();
   exit;
}


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

