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
my ($conn,$result,$error,$row,$cmd);
my ($min_row,$max_row,$cicle,$selectpage,$prepage,$nextpage);
my ($accid,$phone,$nas,$port,$time,$unid);

#--- Init local variables ------
my ($selectlang,$usercount,$pagenumber,$switch) = ('','','','');
$unid = '';

#--- Global variables -------
our %init = ();
do "./config/lpmx_config.pl"  || die;

#--- Language ---
$selectlang = $query->cookie("my_language");         # language from cookies
$selectlang = $init{DefaultLang} unless $selectlang; # default language

#--- Global variables -------
our %dict = ();
do "./dictionary/$selectlang/my_common.pl"  || die;
my $lpath = "./dictionary/$selectlang/".(split(/\//,$ENV{SCRIPT_NAME}))[-1];
do "$lpath" if -e "$lpath";

#--- Page ---
$pagenumber  =  $query->param("p");
$pagenumber = 1 unless  $pagenumber || $pagenumber=~ /[^0-9]/;
$pagenumber = substr("$pagenumber",0,4) if length("$pagenumber") > 4;

#--- Switch ---
$switch = $query->param("w"); # sWitch
$switch = 1 unless $switch || $switch=~ /[^0-9]/;
$switch = substr("$switch",0,2) if length("$switch") > 2;

#--- unid ---
$unid = $query->param("unid");
exit if $unid > 4294967294;

print header();
print "<HEAD><TITLE>$dict{TitlePages}</TITLE>
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
         <br>
         <SPAN class=SelectLang>$dict{ACC_BIG}</SPAN>
      </TD>";

print"<TD VALIGN=TOP class=SiteTop3>
         <TABLE><TR>
          <TD WIDTH=70 class=inner><A HREF=.. class=Link>$dict{EXIT}</A></TD>
          <TD WIDTH=70 class=inner> </TD>
          <TD WIDTH=140 colspan=2 class=inner><A HREF=./index.pl class=Link TITLE=\"$dict{LIST_BIG}\">$dict{LIST}</A></TD>
          <TD WIDTH=140 colspan=2 class=inner><A HREF=./edit.pl class=Link TITLE=\"$dict{NEW_BIG}\">$dict{NEW}</A></TD>
         </TR><TR>
          <TD WIDTH=140 colspan=2 class=inner><A HREF=./avp.pl class=Link TITLE=\"$dict{AVP_BIG}\">\L$dict{AVP}\E</A></TD>
          <TD WIDTH=140 colspan=2 class=inner><SPAN class=Link>\L$dict{ACC}\E</SPAN></TD>
          <TD WIDTH=140 colspan=2 class=inner><A HREF=./cmd.pl class=Link TITLE=\"$dict{CMD_BIG}\">\L$dict{CMD}\E</A></TD>
         </TR></TABLE>
      </TD>";
print"</TR></TABLE>";
##################################################################

$conn = Pg::connectdb("host=$init{prim_dbhost} dbname=$init{prim_dbname} user=$init{prim_dbuser} password=$init{prim_dbpassword}");
if (PGRES_CONNECTION_OK == $conn->status) {

    if($unid ne ''){
        $result = $conn->exec("SELECT accid,phone,nas,port,time FROM tacppd_v1_acc WHERE unid=$unid");
        $error = $conn->errorMessage unless PGRES_TUPLES_OK eq $result->resultStatus;
        print_error_and_exit($dict{ERROR21},$conn,$error,$dict{ERROR},$dict{CAUSE}) if $error ne '';
        $accid = $result->getvalue(0, 0);
        $phone = $result->getvalue(0, 1);
        $nas = $result->getvalue(0, 2);
        $port = $result->getvalue(0, 3);
        $time = $result->getvalue(0, 4);
    }

    print"<br><FORM METHOD=POST ACTION=./acc_save.pl NAME=AccForm>";
    print"<TABLE class=SiteTop><TR><TD VALIGN=TOP>";
     print"<INPUT TYPE=hidden NAME='p' VALUE=\"$pagenumber\">";
     print"<INPUT TYPE=hidden NAME='w' VALUE=\"$switch\">";

     print"<TABLE class=InnerTable>";
     print"<TR>";
      print"<INPUT TYPE=hidden NAME='unid' VALUE=\"$unid\">";

     #-- accid [16] ---
     print"<TD WIDTH=150 VALIGN=TOP>";
      print"<SPAN class=InputText>$dict{ACC}</SPAN>:<br>";
      print"<INPUT TYPE=text AUTOCOMPLETE=\"off\" SIZE=20 MAXLENGTH=16 NAME=accid class=PlaneText VALUE=\"$accid\">";
      &print_examples_textarea('accid','ex1',15,$conn);

      print"<br><br>";

      if($unid ne ''){
          print"<INPUT TYPE='submit' NAME=SaveCmd VALUE=$dict{cmdEdit} class=SaveCmd> ";
      }else{
          print"<INPUT TYPE='submit' NAME=SaveCmd VALUE=$dict{cmdSave} class=SaveCmd> ";
      }

     print"</TD>";

     #-- phone [64] ---
     print"<TD WIDTH=180 VALIGN=TOP>";
      print"<SPAN class=InputText>$dict{ACC_PHONE}</SPAN>:<br>";
      print"<INPUT TYPE=text AUTOCOMPLETE=\"off\" SIZE=28 MAXLENGTH=64 NAME=phone class=PlaneText VALUE=\"$phone\">";
      &print_examples_textarea('phone','ex2',20,$conn);

      print"<br><br>";
      print"<INPUT TYPE=checkbox NAME=delflag VALUE=1 onClick=SetDeleteMark(AccForm,\"$dict{cmdDel}\",\"$dict{cmdEdit}\");>";
      print"<SPAN class=Link>$dict{DELFLAG}</SPAN>";

     print"</TD>";

     #-- nas [64] ---
     print"<TD WIDTH=180 VALIGN=TOP>";
      print"<SPAN class=InputText>$dict{ACC_NAS}</SPAN>:<br>";
      print"<INPUT TYPE=text AUTOCOMPLETE=\"off\" SIZE=28 MAXLENGTH=64 NAME=nas class=PlaneText VALUE=\"$nas\">";
      &print_examples_textarea('nas','ex3',20,$conn);
     print"</TD>";

     #-- port [64] ---
     print"<TD WIDTH=180 VALIGN=TOP>";
      print"<SPAN class=InputText>$dict{ACC_PORT}</SPAN>:<br>";
      print"<INPUT TYPE=text AUTOCOMPLETE=\"off\" SIZE=28 MAXLENGTH=64 NAME=port class=PlaneText VALUE=\"$port\">";
      &print_examples_textarea('port','ex4',20,$conn);
     print"</TD>";

     #-- time [64] ---
     print"<TD WIDTH=180 VALIGN=TOP>";
      print"<SPAN class=InputText>$dict{ACC_TIME}</SPAN>:<br>";
      print"<INPUT TYPE=text AUTOCOMPLETE=\"off\" SIZE=28 MAXLENGTH=64 NAME=time class=PlaneText VALUE=\"$time\">";
      &print_examples_textarea('time','ex5',20,$conn);
     print"</TD>";

     print"</TR>";
     print"</TABLE>";

    print"</TD></TR></TABLE>";
    print"</FORM>";

    $result = $conn->exec("SELECT count(accid) FROM tacppd_v1_acc");
    $error = $conn->errorMessage unless PGRES_TUPLES_OK eq $result->resultStatus;
    print_error_and_exit($dict{ACC_ERROR_1},$conn,$error,$dict{ERROR},$dict{CAUSE}) if $error ne '';
    $usercount = $result->getvalue(0, 0);

    #--- Page setup ----
    if($pagenumber==1){
        $min_row = 0;
        $max_row = $init{LIST_PAGE};
        $max_row = $usercount if $usercount <= $max_row;
    }else{
        $max_row = $pagenumber * $init{LIST_PAGE};
        $min_row = $max_row - $init{LIST_PAGE};
        $max_row = $usercount if $max_row > $usercount;
    }
    $cicle = ceil($usercount/$init{LIST_PAGE});
    $cicle++;
    $selectpage = $pagenumber;
    if ($selectpage > 1){
         $prepage = $selectpage - 1 ;
    }else{ $prepage = 1; }
    if ($selectpage < $cicle - 1){
        $nextpage = $selectpage + 1;
    }else{ $nextpage = $selectpage; }

    #---------------------------------------------------------------------------------
    print"<small>[<A HREF=./acc.pl?w=$switch class=Link>$dict{RETURN}</A>]</small><p>" if $unid ne '';
    #---------------------------------------------------------------------------------

    #--- page line ------
    &print_page_line('acc.pl',0,$pagenumber,$cicle,$selectpage,$switch,$nextpage,$dict{PAGE},$dict{NEXT_PAGE},$dict{USER},$dict{cmdSearch});

    $cmd = "SELECT unid,accid,phone,nas,port,time 
             FROM tacppd_v1_acc ";

    #--- title table ---
    print"<p><TABLE class=TitleTable>";
    print"<TR>";
     print"<TD class=TitleTable0></TD>";

       #--- accid ---
       print"<TD class=TitleTable2 VALIGN=TOP>";
       $cmd = print_sort_table_header($switch,$cmd,1,2,'acc.pl',$dict{ACC},$dict{SORT_DESC},$dict{SORT_ASC},'accid','','');
       print"</TD>";
       print"<TD class=TitleTable0></TD>";

       #--- phone ---
       print"<TD class=TitleTable3 VALIGN=TOP>";
       $cmd = print_sort_table_header($switch,$cmd,3,4,'acc.pl',$dict{ACC_PHONE},$dict{SORT_DESC},$dict{SORT_ASC},'phone','','');
       print"</TD>";
       print"<TD class=TitleTable0></TD>";

       #--- nas ---
       print"<TD class=TitleTable3 VALIGN=TOP>";
       $cmd = print_sort_table_header($switch,$cmd,5,6,'acc.pl',$dict{ACC_NAS},$dict{SORT_DESC},$dict{SORT_ASC},'nas','','');
       print"</TD>";
       print"<TD class=TitleTable0></TD>";

       #--- port ---
       print"<TD class=TitleTable3 VALIGN=TOP>";
       $cmd = print_sort_table_header($switch,$cmd,7,8,'acc.pl',$dict{ACC_PORT},$dict{SORT_DESC},$dict{SORT_ASC},'port','','');
       print"</TD>";
       print"<TD class=TitleTable0></TD>";

       #--- time ---
       print"<TD class=TitleTable3 VALIGN=TOP>";
       $cmd = print_sort_table_header($switch,$cmd,9,10,'acc.pl',$dict{ACC_TIME},$dict{SORT_DESC},$dict{SORT_ASC},'time','','');
       print"</TD>";
       print"<TD class=TitleTable0></TD>";

     print"</TD>";
     print"<TD></TD>";
    print"</TR>";
    print"</TABLE>";

    $cmd .= "LIMIT $init{LIST_PAGE} OFFSET $min_row";
    $result = $conn->exec("$cmd");
    $error = $conn->errorMessage unless PGRES_TUPLES_OK eq $result->resultStatus;
    print_error_and_exit($dict{AVP_ERROR_1},$conn,$error,$dict{ERROR},$dict{CAUSE}) if $error ne '';
    $row = $result->ntuples; 

    #-- Ok ---
    if($row > 0){

        print"<PRE>";
        my($pr_accid,$pr_phone,$pr_nas,$pr_port);

        #--- list ----
        for(my $i=0; $i<$row; $i++){
           $unid = $result->getvalue($i, 0);
           $accid = $result->getvalue($i, 1);
           $phone = $result->getvalue($i, 2);
           $nas = $result->getvalue($i, 3);
           $port = $result->getvalue($i, 4);
           $time = $result->getvalue($i, 5);

           $pr_phone = $phone;
           $pr_nas = $nas;

           $pr_accid = sprintf("%-17s",$accid);

           $pr_phone = sprintf("%-26s",$pr_phone);
           $pr_phone = substr("$pr_phone",0,22).'<font color=red>|.. </font>' if length("$pr_phone") > 26; 

           $pr_nas = sprintf("%-26s",$nas);
           $pr_nas = substr("$pr_nas",0,22).'<font color=red>|.. </font>' if length("$pr_nas") > 26; 

           $pr_port = sprintf("%-26s",$port);
           $pr_port = substr("$pr_port",0,22).'<font color=red>|.. </font>' if length("$pr_port") > 26; 


           ##################
           print"   <A HREF=./acc.pl?p=$pagenumber&w=$switch&unid=$unid class=Link TITLE=\"$phone    $nas    $port    $time\">$pr_accid $pr_phone $pr_nas $pr_port $time</A><br>";
           ##################
        } #for
        print"</PRE>";


    #--- error ----
    }else{
        print"<PRE>   $dict{NULLROW} </PRE>";
    }

    #--- page line ------
    &print_page_line('acc.pl',0,$pagenumber,$cicle,$selectpage,$switch,$nextpage,$dict{PAGE},$dict{NEXT_PAGE},$dict{USER},$dict{cmdSearch});

    $conn->reset;

}else{
    $error = $conn->errorMessage;
    print"$dict{ConnectError} <font color=red>$error</font>";
}
print"</BODY></HTML>";

########################################################################
########################################################################
########################################################################

sub print_examples_textarea{
    my ($id,$ta,$width,$conn) = @_;
    my ($result,$error,$row,$name);

    print"<p>";
    print"<SPAN class=InputText>$dict{EXAMPLES}</SPAN>:<br>";

    $result = $conn->exec("SELECT DISTINCT $id FROM tacppd_v1_acc ORDER BY $id");
    $error = $conn->errorMessage unless PGRES_TUPLES_OK eq $result->resultStatus;
    if ($error ne ''){
       print"<font color=red>$error</font>";
    }else{
        $row = $result->ntuples;
        print"<nobr>";
        print"<TEXTAREA WRAP=off NAME=$ta ROWS=5 COLS=$width ReadOnly=\"on\">";
        for(my $i=0; $i<$row; $i++){
             $name = $result->getvalue($i, 0);
             print"$name\n";
        }
        print"</TEXTAREA>";
        print"</nobr>";
    }
}
