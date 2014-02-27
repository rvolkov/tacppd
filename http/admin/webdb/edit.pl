#!/usr/bin/perl

######################
use CGI qw(:standard);
use POSIX;
use locale;
use Pg;
use Time::Local;
use Time::localtime;
use lib "./lib/";
use My_Sub;
$| = 1;
######################

#--- Local variables -------
my $query = new CGI;
my ($conn,$result,$error,$row,$temp);
my ($editname,$print_user);
my ($print_dclose,$print_username);

#--- Init local variables ------
my ($selectlang,$switch,$pagenumber,$search,$qr) = ('','','','','');
my ($username,$password,$dopen,$dclose,$avpid,$accid,$maxsess) = ('','','','','','',1);


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

#--- Username ---
$editname = $query->param("en");
$editname =~ s/\s+$//;
$editname = substr("$editname",0,100) if length("$editname") > 100;
if($editname eq ''){
    $print_user = "$dict{EDIT_NEW}";
}else{ 
    $print_user = $editname; 
}

#--- Quick return ---
$qr = $query->param("qr"); 


print header();
print "<HEAD><TITLE>[$print_user] $dict{TitlePages}</TITLE>
      <link rel='stylesheet' type='text/css' href='./lib/lpmx.css'>
      <SCRIPT language='JavaScript' src='./lib/lpmx.js'></SCRIPT>
      <META http-equiv='cache control' content='no-cache'></HEAD>";
print"<BODY background='./images/bts.gif'>";


#--- Top table ------
##################################################################
print"<TABLE class=SiteTop>";
print"<TR>";
print"<TD VALIGN=TOP class=SiteTop1>";
         #--- Selected language ---
         print"<DIV class=SelectLang>";
         print"<SPAN class=SelectLang>$dict{lblSwitchLang}</SPAN><br>";
         print"$selectlang";
         print"</DIV>";

         
print"</TD>";
print"<TD VALIGN=TOP class=SiteTop2>
         <SPAN class=big>$dict{TitlePages}</SPAN>
         <br>";
         if($editname eq ''){
             print"<SPAN class=SelectLang>$dict{NEW_BIG}</SPAN>";
         }else{
             print"<SPAN class=SelectLang>$dict{EDIT_BIG}</SPAN>";
         }
     print"</TD>";

print"<TD VALIGN=TOP class=SiteTop3>
         <TABLE><TR>
          <TD WIDTH=70 class=inner><A HREF=.. class=Link>$dict{EXIT}</A></TD>
          <TD WIDTH=70 class=inner> </TD>
          <TD WIDTH=140 colspan=2 class=inner><A HREF=./index.pl?p=$pagenumber&w=$switch&s=$search class=Link TITLE=\"$dict{LIST_BIG}\">$dict{LIST}</A></TD>";
          if($editname eq ''){
              print"<TD WIDTH=140 colspan=2 class=inner><SPAN class=Link>$dict{NEW}</SPAN></TD>";
          }else{
              print"<TD WIDTH=140 colspan=2 class=inner><SPAN class=Link>$dict{EDIT}</SPAN></TD>";
          }
         print"</TR><TR>
          <TD WIDTH=140 colspan=2 class=inner><A HREF=./avp.pl class=Link TITLE=\"$dict{AVP_BIG}\">\L$dict{AVP}\E</A></TD>
          <TD WIDTH=140 colspan=2 class=inner><A HREF=./acc.pl class=Link TITLE=\"$dict{ACC_BIG}\">\L$dict{ACC}\E</A></TD>
          <TD WIDTH=140 colspan=2 class=inner><A HREF=./cmd.pl class=Link TITLE=\"$dict{CMD_BIG}\">\L$dict{CMD}\E</A></TD>
         </TR></TABLE>
      </TD>";
print"</TR></TABLE>";
##################################################################


$conn = Pg::connectdb("host=$init{prim_dbhost} dbname=$init{prim_dbname} user=$init{prim_dbuser} password=$init{prim_dbpassword}");
if (PGRES_CONNECTION_OK == $conn->status) {

    print"<br><FORM METHOD=POST ACTION=./save.pl NAME=EditForm>";
    print"<INPUT TYPE=hidden NAME='p' VALUE=\"$pagenumber\">";
    print"<INPUT TYPE=hidden NAME='w' VALUE=\"$switch\">";
    print"<INPUT TYPE=hidden NAME='s' VALUE=\"$search\">";
    print"<TABLE class=SiteTop><TR><TD VALIGN=TOP>";
     print"<TABLE class=InnerTable>";
     print"<TR>";
     #-- user+passwd ---
     print"<TD WIDTH=150 VALIGN=TOP>";
      #- new -
      if ($editname eq ''){
          print"<SPAN class=InputText>$dict{EDIT_USERNAME}</SPAN>:<br>";
          print"<INPUT TYPE=text AUTOCOMPLETE=\"off\" SIZE=20 MAXLENGTH=32 NAME=username class=PlaneText>";
      #- edit -
      }else{
          print"<SPAN class=InputText>$dict{EDIT_USER}</SPAN>:<br>";
          print"<SPAN class=ColorText>$editname</SPAN>";
          print"<INPUT TYPE=hidden NAME='en' VALUE=\"$editname\">";
          $result = $conn->exec("SELECT username,date_part('epoch',dopen),date_part('epoch',dclose),avpid,accid,maxsess,password
                                 FROM tacppd_v1_usr
                                 WHERE username = '$editname'");
          $error = $conn->errorMessage unless PGRES_TUPLES_OK eq $result->resultStatus;
          print_error_and_exit($dict{ERROR_1},$conn,$error,$dict{ERROR},$dict{CAUSE}) if $error ne '';
          $row = $result->ntuples;
          if($row > 0){
             $username = $result->getvalue(0, 0);
             $dopen = $result->getvalue(0, 1);
             $dclose = $result->getvalue(0, 2);
             $avpid = $result->getvalue(0, 3);
             $accid = $result->getvalue(0, 4);
             $maxsess = $result->getvalue(0, 5);
             $password = $result->getvalue(0, 6);
             $username =~ s/\s+$//;
             $avpid =~ s/\s+$//;
             $accid =~ s/\s+$//;
          }else{
              $error = "$editname not found!";
              print_error_and_exit($dict{ERROR_4},$conn,$error,$dict{ERROR},$dict{CAUSE});
          }
      }
      print"<p>";
      print"<SPAN class=InputText>$dict{EDIT_PASSWORD}</SPAN>:<br>";
      print"<INPUT TYPE=text AUTOCOMPLETE=\"off\" SIZE=20 MAXLENGTH=50 NAME=password class=PlaneText value=\"$password\">";
      print"<br><br><br>";
      print"<INPUT TYPE='submit' VALUE=$dict{cmdSave} class=SaveCmd> ";
      print"<INPUT TYPE='reset' VALUE=$dict{cmdClear} class=MiniCmd>";
     print"</TD>";

     #-- random passwd ---
     print"<TD WIDTH=150 VALIGN=TOP>";
      print"<SPAN class=InputText>$dict{EDIT_SELECTPASS}</SPAN><br>";
      my $ex;
      for(;;){ $ex = get_random_pass(); last if $ex=~ /[a-z]/ && $ex=~ /[A-Z]/; }
      print"<INPUT TYPE='text' AUTOCOMPLETE=\"off\" readonly=\"on\" name=ex1 VALUE=\"$ex\" class='SelectPassT' onFocus='document.EditForm.password.value=\"$ex\"; '><br>";
      for(;;){ $ex = get_random_pass(); last if $ex=~ /[a-z]/ && $ex=~ /[A-Z]/; }
      print"<INPUT TYPE='text' AUTOCOMPLETE=\"off\" readonly=\"on\" name=ex2 VALUE=\"$ex\" class='SelectPassB' onFocus='document.EditForm.password.value=\"$ex\"; '><br>";
      for(;;){ $ex = get_random_pass(); last if $ex=~ /[a-z]/ && $ex=~ /[A-Z]/; }
      print"<INPUT TYPE='text' AUTOCOMPLETE=\"off\" readonly=\"on\" name=ex3 VALUE=\"$ex\" class='SelectPassB' onFocus='document.EditForm.password.value=\"$ex\"; '><br>";
      for(;;){ $ex = get_random_pass(); last if $ex=~ /[a-z]/ && $ex=~ /[A-Z]/; }
      print"<INPUT TYPE='text' AUTOCOMPLETE=\"off\" readonly=\"on\" name=ex4 VALUE=\"$ex\" class='SelectPassB' onFocus='document.EditForm.password.value=\"$ex\"; '><br>";
     print"</TD>";

     #-- split column --
     print"<TD WIDTH=30>&nbsp;</TD>";

     #--- attributes ------
     print"<TD WIDTH=150 VALIGN=TOP>";
      print"<SPAN class=InputText>$dict{EDIT_TYPE_AVP}</SPAN>:<br>";
      $result = $conn->exec("SELECT DISTINCT avpid
                             FROM tacppd_v1_avp
                             ORDER BY avpid");
      $error = $conn->errorMessage unless PGRES_TUPLES_OK eq $result->resultStatus;
      print_error_and_exit($dict{ERROR_1},$conn,$error,$dict{ERROR},$dict{CAUSE}) if $error ne '';
      $row = $result->ntuples; 
      print"<SELECT NAME='avpid' class=MySelect>";
      $avpid = $init{EDIT_DEF_AVP} if $avpid eq '';
      for(my $i=0; $i<$row; $i++){
          $temp = $result->getvalue($i, 0);
          if($temp eq $avpid){
              print"<OPTION SELECTED VALUE=$temp>$temp";
          }else{
              print"<OPTION VALUE=$temp>$temp";
          }
      }
      print"</SELECT>";

      print"<p>";
      print"<SPAN class=InputText>$dict{EDIT_TYPE_ACC}</SPAN>:<br>";
      $result = $conn->exec("SELECT DISTINCT accid
                             FROM tacppd_v1_acc
                             ORDER BY accid");
      $error = $conn->errorMessage unless PGRES_TUPLES_OK eq $result->resultStatus;
      print_error_and_exit($dict{ERROR_1},$conn,$error,$dict{ERROR},$dict{CAUSE}) if $error ne '';
      $row = $result->ntuples; 
      print"<SELECT NAME='accid' class=MySelect>";
      $accid = $init{EDIT_DEF_ACC} if $accid eq '';
      for(my $i=0; $i<$row; $i++){
          $temp = $result->getvalue($i, 0);
          if($temp eq $accid){
              print"<OPTION SELECTED VALUE=$temp>$temp";
          }else{
              print"<OPTION VALUE=$temp>$temp";
          }
      }
      print"</SELECT>";
      print"<p>";
      print"<SPAN class=InputText>$dict{EDIT_MAXSESS}</SPAN>:<br>";
      print"<INPUT TYPE=text AUTOCOMPLETE=\"off\" SIZE=10 MAXLENGTH=5 NAME=maxsess class=PlaneText VALUE=$maxsess>";
     print"</TD>";

     #--- date ----
     print"<TD WIDTH=150 VALIGN=TOP>";

      $dopen = time if $dopen eq '';
      $dopen = get_cur_datetime($dopen);
      print"<SPAN class=InputText>$dict{DOPEN}</SPAN>:<br>";
      print"<INPUT TYPE=text AUTOCOMPLETE=\"off\" SIZE=20 MAXLENGTH=20 NAME=dopen class=PlaneText value=\"$dopen\">";
      print"<p>";
      print"<SPAN class=InputText>$dict{EDIT_DCLOSE}</SPAN>:<br>";
      print"<SPAN class=InputText>mm/dd/yyyy hh:mm:ss</SPAN><br>";

      $dclose = get_cur_datetime($dclose) if $dclose ne '';
      print"<INPUT TYPE=text AUTOCOMPLETE=\"off\" SIZE=20 MAXLENGTH=20 NAME=dclose class=PlaneTextRed value=\"$dclose\">";
      print"<p>";

      my $js_dclose;
      if ($dclose eq ''){
          $js_dclose = get_cur_datetime(time);
      }else{
          $js_dclose = $dclose;
      }
      my($js_date,$js_time) = split(/\ /,$js_dclose);
      print"<SELECT NAME='action' class=MySelectColor onChange=SetCloseTime(EditForm,\"$js_date\",\"$js_time\");>";
           if($dclose eq ''){
               print"<OPTION SELECTED VALUE=1> $dict{EDIT_OPEN1} ";
               print"<OPTION VALUE=2> $dict{EDIT_CLOSE} ";
           }else{
               print"<OPTION VALUE=1> $dict{EDIT_OPEN} ";
               print"<OPTION SELECTED VALUE=2> $dict{EDIT_CLOSE1} ";
           }
      print"</SELECT>";
     print"</TD>";

     #--- note ---
     print"<TD VALIGN=TOP>";
      print"<SPAN class=InputText>";
        print"<li>$dict{EDIT_RULE1}";
        print"<li>$dict{EDIT_RULE2}";
      print"<br><br>";
      if($qr == 1){
         print"<INPUT TYPE=checkbox NAME=qr VALUE=1 checked> $dict{EDIT_QR}</SPAN>"; # Quick Return
      }else{
         if($editname eq ''){
             print"<INPUT TYPE=checkbox NAME=qr VALUE=1 checked> $dict{EDIT_QR}</SPAN>"; # Quick Return
         }else{ 
             print"<INPUT TYPE=checkbox NAME=qr VALUE=1> $dict{EDIT_QR}</SPAN>"; # Quick Return
         }
      }
     print"</TD>";

     print"</TR>";
     print"</TABLE>";
    print"</TD></TR></TABLE>";
    print"</FORM>";

    #---------------------------------------------------------------------------------
    print"<br><small>[<A HREF=./edit.pl?p=$pagenumber&w=$switch&s=$search class=Link>$dict{RETURN}</A>]</small>" if $editname ne '';
    #---------------------------------------------------------------------------------


    #######################################################
    #--- List of last users ----------
    print"<br><br><hr>";
    print"<SPAN class=SelectLang>$dict{EDIT_LIST} ";
    print"[ $init{LIST_LAST_USERS} ]</SPAN>";
    print"<p>";

    $result = $conn->exec("SELECT username,date_part('epoch',dopen),date_part('epoch',dclose),avpid,accid,maxsess 
                           FROM tacppd_v1_usr ORDER BY dupdate DESC
                           LIMIT $init{LIST_LAST_USERS} OFFSET 0 ");
    $error = $conn->errorMessage unless PGRES_TUPLES_OK eq $result->resultStatus;
    print_error_and_exit($dict{ERROR_1},$conn,$error,$dict{ERROR},$dict{CAUSE}) if $error ne '';
    $row = $result->ntuples; 

    #--- title table ---
    &print_table_header();

    #-- Ok ---
    if($row > 0){

        print"<PRE>";
        #--- list ----
        for(my $i=0; $i<$row; $i++){
             $username = $result->getvalue($i, 0);
             $dopen = $result->getvalue($i, 1);
             $dclose = $result->getvalue($i, 2);
             $avpid = $result->getvalue($i, 3);
             $accid = $result->getvalue($i, 4);
             $maxsess = $result->getvalue($i, 5);
             $username=~ s/\s+$//;

             $print_username = $username;
             $print_username = substr("$print_username",0,22).'..' if length("$print_username") > 22; 
             $print_username = sprintf("%-24s",$print_username);
             $avpid = sprintf("%-17s",$avpid);
             $accid = sprintf("%-17s",$accid);
             $maxsess = sprintf("%12s",$maxsess);
             $dopen = get_cur_datetime($dopen);
             $print_dclose = '';
             $print_dclose = get_cur_datetime($dclose) if $dclose ne '';
             $print_dclose = sprintf("%19s",$print_dclose);

             #************
             if($dclose < time){
                 print"   <A HREF=./edit.pl?p=$pagenumber&w=$switch&en=$username&s=$search class=Link TITLE=\"$username\">$print_username$dopen</A>     <font color=red>$print_dclose</font>     $avpid $accid $maxsess<br>";
             }else{
                 print"   <A HREF=./edit.pl?p=$pagenumber&w=$switch&en=$username&s=$search class=Link TITLE=\"$username\">$print_username$dopen</A>     $print_dclose     $avpid $accid $maxsess<br>";
             }
             #************
        } #for
        print"</PRE>";



    #--- error ----
    }else{
        $error = "\$row = $row";
        print_error_and_exit($dict{ERROR_2},$conn,$error,$dict{ERROR},$dict{CAUSE});
    }

    $conn->reset;

}else{
    $error = $conn->errorMessage;
    print"$dict{ConnectError} <font color=red>$error</font>";
}
print"</BODY></HTML>";


########################################################################
########################################################################
########################################################################

#--- title table ---
sub print_table_header{

    print"<TABLE class=TitleTable>";
    print"<TR>";
     print"<TD class=TitleTable0></TD>";

     #--- username ---
     print"<TD class=TitleTable1 VALIGN=TOP>";
          print"$dict{USERNAME}"; 
     print"</TD>";
     print"<TD class=TitleTable0></TD>";

     #--- dopen ---
     print"<TD class=TitleTable1 VALIGN=TOP>";
         print"$dict{DOPEN}"; 
     print"</TD>";
     print"<TD class=TitleTable0></TD>";

     #--- dclose ---
     print"<TD class=TitleTable1 VALIGN=TOP>";
         print"$dict{DCLOSE}"; 
     print"</TD>";
     print"<TD class=TitleTable0></TD>";

     #--- avpid ---
     print"<TD class=TitleTable2 VALIGN=TOP>";
         print"$dict{AVP}"; 
     print"</TD>";
     print"<TD class=TitleTable0></TD>";

     #--- accid ---
     print"<TD class=TitleTable2 VALIGN=TOP>";
         print"$dict{ACC}"; 
     print"</TD>";
     print"<TD class=TitleTable0></TD>";

     #--- maxsess ---
     print"<TD class=TitleTable2 VALIGN=TOP>";
         print"$dict{MAXSESS}"; 
     print"</TD>";
     print"<TD></TD>";
    print"</TR>";
    print"</TABLE>";
}



sub get_random_pass{
    my @arr = () ;
    my($row,$split,$value,$left,$right,$digit);

    my $obj_time  =  localtime(time); 
    my $sec = $obj_time->sec();
    if($sec == 1){
        @arr = qw(% ^ & ? a @ $ b h N P m n o i k L M p Z 1 2 W X Y u v w 5 6 x y z A B f g G C D E F b a d a c d e H J T U V 3 4 7 8 9 0 K q r s t Q R S);#
    }elsif($sec == 2){
        @arr = qw(@ & $ % h i k L M N P m n K q r s t 4 Q R S T U V 3 4 7 o p W D E F c d e f g X Y Z 1 2 u v w x y z A B C 5 6 G H J 8 9 0 ^ ? a b);#
    }elsif($sec == 3){
        @arr = qw(L M % ^ & ? a b h i N P W X Y Z 1 2 u v w x y m n o p @ E F c d e f g $ k z A K q r s t Q R 8 9 0 S T B C 5 6 D G H J U V 3 4 7);#
    }elsif($sec == 4){
        @arr = qw(g G H J K q r s t m n o p W X Y Z Q R S T U V 3 4 7 8 9 0 @ $ % ^ & C 5 6 / D E F c d ? a b h i k L + M N P 1 2 u v w x y z A B e f);#
    }elsif($sec == 5){
        @arr = qw(k L M N P m n @ $ % ^ & ? a b h i o p / W X Y Z C 5 6 D + E F c d e f 1 2 u v w x y z A B g G H J K q r s t Q R S U V 3 4 7 8 9 0 T);#
    }elsif($sec == 6){
        @arr = qw(a b h i ^ & ? M N P m n o p W X Y Z 1 2 u v w x y z A g G H J K q r s t Q R S T U B C 5 6 D E F c d e f V 3 4 7 8 9 0 k L @ $ %);#
    }elsif($sec == 7){
        @arr = qw(R S T U V 3 4 7 8 N P m n o p W X Y Z 1 2 u v w 9 0 @ $ % ^ & ? a b h i k L M x y z A B C 5 6 D E F / c d e f g G H J K q r s t Q);#
    }elsif($sec == 8){
        @arr = qw(d H J K e f g % ^ & G q r s t Q R S T @ K n o p W X $ ? a b h i k L M N P * m Y Z 1 2 u v w 6 D E x y z A B C 5 F c U V 3 4 8 7 9 0);#
    }elsif($sec == 9){
        @arr = qw(@ h i k L $ % ^ & ? a p W X b M N P m w x y = z A n o Y Z 1 2 u v B C 5 6 D E F c d e f g G H J K q r s n o Y Z S T U V 3 + 4 7 8 9 0 1 2 u v t Q R);#
    }elsif($sec == 10){
        @arr = qw(@ $ % ^ W X Y Z 1 2 u v w x y z A B C 5 6 D & ? a b h i k L M G N P m n o p E F c d e f g G Q R S T U V 3 4 7 8 9 0 H J K q r s t);#
    }elsif($sec == 11){
        @arr = qw(W X Y Z 1 2 u v @ M N P $ % ^ & ? a b h i k L m n o p 6 D E F c d e f g G w x y z A B C 5 H J K q U V 3 4 7 8 9 0 r s t Q R S T);#
    }elsif($sec == 12){
        @arr = qw(q r s t ? a b h i k L M N P m n o p W X Y Q R S 7 8 9 0 T U x y z A B C 5 @ $ % ^ & Z 1 2 u v w 6 D E F c d e f g G H J K V 3 4);#
    }elsif($sec == 13){
        @arr = qw(* p W @ $ % ^ & ? a b h i k L M N P m n o X Y = Z C 5 6 D E 1 2 u v w x y z A B F J K q r 7 8 9 0 s t Q R S c d e f g G H b a b a T U V 3 4);#
    }elsif($sec == 14){
        @arr = qw(N @ $ 2 u v w x % ^ & ? a b h i k L M P y z A B C 5 6 D E F c d Q R S T U m * n o p W X Y Z 1 V 3 4 7 8 9 0 e f g G H J K q r s t);#
    }elsif($sec == 15){
        @arr = qw(v w x y z A B n o p C 5 6 G H @ $ % ^ & ? a b h i k L M N P m / W X Y Z 1 2 u J K q r s t Q R S T U D E F c d e f g V 3 4 7 8 9 0);#
    }elsif($sec == 16){
        @arr = qw(@ $ P m n o p W X Y % ^ & ? a b h i k L M N Z 1 2 u v w x y z A B C v w K q r s t Q R x y z 5 6 D E F < c d e f g G H J S T U V 3 4 7 > 8 9 0);#
    }elsif($sec == 17){
        @arr = qw(t Q R S T U V 3 M N P m n o 4 7 8 9 0 @ $ % ^ & ? 2 u v w x a b h i k L p 5 6 D E F W X Y Z 1 y z A B C c d e f g G H J K q r s);#
    }elsif($sec == 18){
        @arr = qw(@ L M N P m n $ % ^ & ? a b w x y z A B h i k o p W D E F c d e f X Y Z 1 2 u v C 5 6 g G H Q R S T U V 3 4 7 8 9 0 X Y Z 1 J K q r s t 2 u v C 5 6);#
    }elsif($sec == 19){
        @arr = qw(@ $ L M % ^ & m n o p ? a b u v w > h i k N P 5 6 D E W X Y Z 1 2 x H J K y z A B C F c t Q R d < e f V 3 g G q r 9 0 s S T U 4 7 8);#
    }elsif($sec == 20){
        @arr = qw(@ $ & ? a M N P b h % o p ^ i k 2 u v w x y z A B C 5 6 D E F c d e f 8 9 0 L m n W g G H J K q r s t Q R S T U V 3 4 7 X Y Z 1);#
    }elsif($sec == 21){
        @arr = qw($ N % h i k @ W X L M P m n ^ B C 5 6 6 D E F c d e f g G H J K Y Z 1 2 u v 3 4 q r s t Q & ? a b o p 7 8 9 0 w x y z A R S T U V);#
    }elsif($sec == 22){
        @arr = qw(a C 5 6 D 1 2 u v w x y z A B b a b f g E F % ^ c d e @ $ M m N P m n o p W X Y Z G H J K q & ? a b h i k L r s t Q R S T U V 3 4 7 8 9 0);#
    }elsif($sec == 23){
        @arr = qw(@ $ % ^ p W X Y Z 1 2 u v w x & ? a b h i k L M N P m n o y z A B C 5 6 D T U V 3 4 7 8 9 0 E F c d e f g G H J K q r s t Q R S);#
    }elsif($sec == 24){
        @arr = qw(5 6 D E F c d e f g G H J K q r s t Q R S T U V 3 4 7 8 9 0 @ $ % ^ & ? a b h i k L M N P m n o p W X Y Z 1 2 u v w x y z A B C);#
    }elsif($sec == 25){
        @arr = qw(g G H J K q r s t Q R S T U V @ $ % ^ & ? a b h i k L M N P m n o p W X Y Z 1 2 u v w x y z A B C 5 6 D E F c d e f 3 4 7 8 9 0);#
    }elsif($sec == 26){
        @arr = qw($ % b h i g G H J K q r s t N P m n o p W X Y Z 1 2 Q R S 6 D E F c + k L M u v w x y z A B C 5 = d e f T U V 3 4 7 ^ & ? a 8 9 0);#
    }elsif($sec == 27){
        @arr = qw(a b 1 2 u v w d e f g G H J K q r s t Q R S T x y z A B C 5 6 D E F c U V 3 4 7 8 9 0 h i k L M N P m n o p W X Y Z);#
    }elsif($sec == 28){
        @arr = qw(@ $ M N P m n o p W X Y Z 1 2 u v w x y z A B C 5 6 D E F c d m n o p W X Y Z 1 2 u v w x y z A B e f g G H J K q r s t Q R S T U V 3 4 7 8 9 0);#
    }elsif($sec == 29){
        @arr = qw(@ k L M N P Y Z 1 2 u v w x y z A B C m n $ % ^ & ? a b h i o p W X 5 6 D E F m n $ % ^ & ? a G H J K q r s t Q R S T U V 3 b h i o p W X c d e f g 4 7 8 9 0 e f g G H J);#
    }elsif($sec == 30){
        @arr = qw(@ A B C 5 6 D E F c d e f g G H J K q r $ % ^ & ? a b h i k L M N P m n o p W X Y Z 1 2 u / > < + * v w x y z s t 8 9 0 Q R S T U V 3 4 7);#
    }elsif($sec == 31){
        @arr = qw(Z 1 ? a 2 u v k L M ^ A B % C E Q 6 D $ h F c d s t Q R S T U V 3 4 7 8 9 0 & b e f g G H J K q r N P m W X Y i n o p z y w x @ 5);#
    }elsif($sec == 32){
        @arr = qw(P m n 6 D E F c d e f g G H J K q r y z A B C 5 * t Q R S T U V 3 4 7 8 s o p W X Y Z 1 @ $ % ^ & ? a b h i k L M N 2 u v w x 9 0);#
    }elsif($sec == 33){
        @arr = qw(a @ $ i k o p W X Y Z 1 L M % ^ & ? b h N P y z * A B m n 2 u U V 3 4 7 e f g G H J K q v E F c d R S T r s t Q 8 w x C 5 6 D 9 0);#
    }elsif($sec == 34){
        @arr = qw(L M ^ & ? a b h i k o p n @ $ % 2 u v w x y c d e f g G H z W X Y Z R S T U V 3 4 1 N P m A B C 5 6 D E F J K q r s t Q 7 8 9 0);#
    }elsif($sec == 35){
        @arr = qw(i k L n @ $ % ^ & ? a P m o p N X M Z 1 2 Y W y u v w b h C 5 x F c d B z A 6 D E e f g G Q R 9 H J K q r s t 4 S T U 0 V 3 8 7);#
    }elsif($sec == 36){
        @arr = qw(@ B C 5 6 D E F c 7 8 9 0 $ % ^ & d e f g G H J K m n o p q r s t Q R S T U V 3 4 ? a b h i k L M N P W X Y Z 1 2 u v w x y z A);#
    }elsif($sec == 37){
        @arr = qw(? a b h i % ^ & P m n o p L M N @ $ 1 2 u k x y z W X Y Z C 5 A B v F c E e f g d w K q r s R t 4 7 8 9 0 D G H Q 6 V 3 J S T U);#
    }elsif($sec == 38){
        @arr = qw(u v x z A B C @ & b ? i $ L % N ^ n a p h X k Z M 2 P w m y 5 o E W Y d s t Q 0 F 1 6 D c * e R S T 9 3 U V 7 8 4 f g G H J K q r);#
    }elsif($sec == 39){
        @arr = qw(u @ % N ^ n a p v x z A k Z M 2 B C & b o E W Y d s t ? i 0 F 1 $ L h X P 3 U V w m y 5 Q 6 7 8 4 D c * e R S T 9 f H J K q r g G);#
    }elsif($sec == 40){
        @arr = qw(i $ L u v x z A B C a p h @ & b ? 2 P w % N W Y d s t Q m y 5 o E H J K q r 0 F 1 6 D c e R S T 9 3 U V 7 8 4 f g G ^ n X k Z M);#
    }elsif($sec == 41){
        @arr = qw(u v x Z M 2 P w m y 5 o z A B C @ & b ? i $ 0 F 1 6 4 f g G H J K q r L % N ^ n a p D c * e R S T 9 3 U V 7 8 h X k E W Y d s t Q);#
    }elsif($sec == 42){
        @arr = qw(x h X k Z M 2 K q r w m P 8 4 f z A B C @ & b ? u v i $ L % N ^ n a p g G H J y 5 o E W Y d s t Q 0 F 1 6 D c * e R S T 9 3 U V 7);#
    }elsif($sec == 43){
        @arr = qw(u v N ^ n a p x z A B C Y d s t Q 0 F 1 6 D c @ & b ? i m y 5 o E W $ L % h X k Z M 2 8 4 f g P w e R S T 9 q 3 U V 7 G H J K r);#
    }elsif($sec == 44){
        @arr = qw(u a p h X k Z M 2 P w m y 5 o E W Y d s v x z A B C R S T 9 3 U V 7 8 4 f @ & b ? i $ L % N ^ n t Q 0 F 1 6 D c * e g G H J K q r);#
    }elsif($sec == 45){
        @arr = qw(h @ i $ ? a b k M N X Y Z 1 C 5 6 D E F c P m n o p W d e f Q R S 2 u v w x y z A B T U V 3 4 7 8 9 g G H J K q r s t 0 L % ^ &);#
    }elsif($sec == 46){
        @arr = qw(@ X Y Z 1 2 M N P m n o p W w x y z A u v $ B g G H J ? a b h U V C 5 6 D E % ^ & e f 3 4 7 8 9 0 i k L t Q R S T F c d K q r s);#
    }elsif($sec == 47){
        @arr = qw(N P m n 1 2 1 2 u v Z w x y o p @ $ % ^ & ? C 5 W X Y Z z A B F 6 D E a q r s t b h i k L M c 9 d e f g G H J K Q R S T U V 3 4 7 8 0);#
    }elsif($sec == 48){
        @arr = qw(@ $ L M % ^ & m n k N P X Y Z 1 A B C w x y 2 u v H 6 D z e f g G 5 W ? R S E F c d 9 0 o p J K q r s t Q 7 8 T U V 3 4 a b h i);#
    }elsif($sec == 49){
        @arr = qw(@ p W X Y Z 1 $ % ^ & ? a v w N P m n o 2 u B C x y E F c d z A b h i H J K q r k L M 5 / S 6 D e f + U V 3 4 7 8 = g G s t Q R k * T 9 0);#
    }elsif($sec == 50){
        @arr = qw(6 D E F c d e @ $ % ^ & ? a b h i k L M g G H J K Y Z 1 2 u N P w x y z A B C 5 f v t Q R S T U V 3 q r s m n o p W X 4);#
    }elsif($sec == 51){
        @arr = qw(m n o p W X Y @ $ % 2 u v w ^ & ? a b h i k L 6 D E F c d e M N 4 7 z A B C 5 f g G H P Z 1 x y V 3 J K q r s t Q R S T U);#
    }elsif($sec == 52){
        @arr = qw(h i @ $ % ^ & W X Y Z 1 2 u v w x y z A B C k L M N P m n o 5 6 D E F c d s t Q R S 7 8 e f g G T U V 3 4 H ? a b p J K q r);#
    }elsif($sec == 53){
        @arr = qw( 2 3 4 u v @ $ % ^ & ? a b h W W X i k L M z A B C 5 N P m n E F c w x y 6 D d H J K S T U V 3 4 7 8 e o q r s t Q R p Y Z 1 f g G 9);#
    }elsif($sec == 54){
        @arr = qw(& f g G H 9 0 J K q r T ? a b h i k L M N P m n $ % ^ x y z A o p W X Y Z E F c d 1 2 u v w @ B C s t Q R S 5 6 D e U ? V 3 4 7 8);#
    }elsif($sec == 55){
        @arr = qw(@ $ P m n % ^ & ? a Y Z 1 2 u v w x y z A b h o p W X Q R S T U V 3 4 B C 5 6 D g 9 i k L E F c d e f M N 0 G H J K q r s t 7 8);#
    }elsif($sec == 56){
        @arr = qw(5 6 D E F c d @ $ % ^ & ? a b h i k L M N P 8 9 0 p W X Y Z 1 2 u v w x m n o R S T U V 3 4 7 y z A B C e f g G H J K q r s t Q);#
    }elsif($sec == 57){
        @arr = qw(W X Y Z 1 2 u v w x y z @ $ % ^ & ? a b h c d e f g G H J K 4 i k L E F 7 8 9 0 M N P m n q r s t Q R S T U V 3 o p A B C 5 6 D);#
    }elsif($sec == 58){
        @arr = qw(X Y Z 1 2 u v w x y z A @ $ % ^ & ? a b h i k L b a B C 5 6 M N P m n o p W a q r s t Q b a D E F c d e f g G H 8 9 0 v J K Q R S T U V 3 4 7);#
    }else{
        @arr = qw(3 4 D E F h i 8 C 5 6 b G H J k L M N P o p W X y z A m n g Y Z a 7 K c d e f Q R S 1 2 u v q r s t B % ^ & ? T U V w x @ $ 9 0);#
    }

    $row = $init{GENPASS_MIN} + int(rand $init{GENPASS_FLUCT});
    for (my $i = 0; $i < $row; $i++){ $value .= $arr[int(rand ($#arr + 1))]; } 

    #-- no digit --
    unless ($value=~ /[0-9]/){
        $split = int(rand $row) + 1;
        $left = substr($value,0,$split); 
        $right = substr($value,$split,$row-$split); 
        chop $left;
        $digit = int(rand 9) + 1; # случайная цифра
        $value = $left.$digit.$right; # в случайной позиции
    }
    @arr = () ;
    return($value);
}
