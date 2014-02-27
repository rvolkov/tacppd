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
my ($conn,$result,$error,$row,$cmd,$temp);
my ($min_row,$max_row,$cicle,$selectpage,$prepage,$nextpage);
my ($avpid,$cmdperm,$cmddeny,$argperm,$argdeny,$unid);

#--- Init local variables ------
my ($selectlang,$usercount,$pagenumber,$switch) = ('','','','');
$unid = '';

#--- Global variables -------
our %init = ();
do "./config/lpmx_config.pl"  || die;

#--- Language ---
$selectlang = $query->cookie("my_language");                               # language from cookies
$selectlang = $init{DefaultLang} unless $selectlang;                       # default language

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
         <SPAN class=SelectLang>$dict{CMD_BIG}</SPAN>
      </TD>";

print"<TD VALIGN=TOP class=SiteTop3>
         <TABLE><TR>
          <TD WIDTH=70 class=inner><A HREF=.. class=Link>$dict{EXIT}</A></TD>
          <TD WIDTH=70 class=inner> </TD>
          <TD WIDTH=140 colspan=2 class=inner><A HREF=./index.pl class=Link TITLE=\"$dict{LIST_BIG}\">$dict{LIST}</A></TD>
          <TD WIDTH=140 colspan=2 class=inner><A HREF=./edit.pl class=Link TITLE=\"$dict{NEW_BIG}\">$dict{NEW}</A></TD>
         </TR><TR>
          <TD WIDTH=140 colspan=2 class=inner><A HREF=./avp.pl class=Link TITLE=\"$dict{AVP_BIG}\">\L$dict{AVP}\E</A></TD>
          <TD WIDTH=140 colspan=2 class=inner><A HREF=./acc.pl class=Link TITLE=\"$dict{ACC_BIG}\">\L$dict{ACC}\E</A></TD>
          <TD WIDTH=140 colspan=2 class=inner><SPAN class=Link>\L$dict{CMD}\E</SPAN></TD>
         </TR></TABLE>
      </TD>";
print"</TR></TABLE>";
##################################################################

$conn = Pg::connectdb("host=$init{prim_dbhost} dbname=$init{prim_dbname} user=$init{prim_dbuser} password=$init{prim_dbpassword}");
if (PGRES_CONNECTION_OK == $conn->status) {

    if($unid ne ''){
        $result = $conn->exec("SELECT avpid,cmdperm,cmddeny,argperm,argdeny FROM tacppd_v1_cmd WHERE unid=$unid");
        $error = $conn->errorMessage unless PGRES_TUPLES_OK eq $result->resultStatus;
        print_error_and_exit($dict{ERROR21},$conn,$error,$dict{ERROR},$dict{CAUSE}) if $error ne '';
        $avpid = $result->getvalue(0, 0);
        $cmdperm = $result->getvalue(0, 1);
        $cmddeny = $result->getvalue(0, 2);
        $argperm = $result->getvalue(0, 3);
        $argdeny = $result->getvalue(0, 4);
    }

    print"<br><FORM METHOD=POST ACTION=./cmd_save.pl NAME=CmdForm>";
    print"<TABLE class=SiteTop><TR><TD VALIGN=TOP>";
     print"<INPUT TYPE=hidden NAME='p' VALUE=\"$pagenumber\">";
     print"<INPUT TYPE=hidden NAME='w' VALUE=\"$switch\">";

     print"<TABLE class=InnerTable>";
     print"<TR>";

     print"<INPUT TYPE=hidden NAME='unid' VALUE=\"$unid\">";

     #-- avpid [16] ---
     print"<TD WIDTH=150 VALIGN=TOP>";
      print"<SPAN class=InputText>$dict{AVP}</SPAN>:<br>";

      $result = $conn->exec("SELECT DISTINCT avpid
                             FROM tacppd_v1_avp
                             ORDER BY avpid");
      $error = $conn->errorMessage unless PGRES_TUPLES_OK eq $result->resultStatus;
      print_error_and_exit($dict{ERROR_1},$conn,$error,$dict{ERROR},$dict{CAUSE}) if $error ne '';
      $row = $result->ntuples; 
      print"<SELECT NAME='avpid' class=MySelect>";
      for(my $i=0; $i<$row; $i++){
          $temp = $result->getvalue($i, 0);
          if($temp eq $avpid){
              print"<OPTION SELECTED VALUE=$temp>$temp";
          }else{
              print"<OPTION VALUE=$temp>$temp";
          }
      }
      print"</SELECT>";
      print"<br><br><br><br><br>";
      if($unid ne ''){
          print"<INPUT TYPE='submit' NAME=SaveCmd VALUE=$dict{cmdEdit} class=SaveCmd> ";
      }else{
          print"<INPUT TYPE='submit' NAME=SaveCmd VALUE=$dict{cmdSave} class=SaveCmd> ";
      }
     print"</TD>";

     #-- cmdperm [50] ---
     print"<TD WIDTH=180 VALIGN=TOP>";
      print"<SPAN class=InputText>$dict{CMD_CMDPERM}</SPAN>:<br>";
      print"<INPUT TYPE=text AUTOCOMPLETE=\"off\" SIZE=28 MAXLENGTH=50 NAME=cmdperm class=PlaneText VALUE=\"$cmdperm\">";
      &print_examples_textarea('cmdperm','ex1',20,$conn);

      print"<br><br>";
      print"<INPUT TYPE=checkbox NAME=delflag VALUE=1 onClick=SetDeleteMark(CmdForm,\"$dict{cmdDel}\",\"$dict{cmdEdit}\");>";
      print"<SPAN class=Link>$dict{DELFLAG}</SPAN>";

     print"</TD>";

     #-- cmddeny [50] ---
     print"<TD WIDTH=180 VALIGN=TOP>";
      print"<SPAN class=InputText>$dict{CMD_CMDDENY}</SPAN>:<br>";
      print"<INPUT TYPE=text AUTOCOMPLETE=\"off\" SIZE=28 MAXLENGTH=50 NAME=cmddeny class=PlaneText VALUE=\"$cmddeny\">";
      &print_examples_textarea('cmddeny','ex2',20,$conn);

     print"</TD>";

     #-- argperm [50] ---
     print"<TD WIDTH=180 VALIGN=TOP>";
      print"<SPAN class=InputText>$dict{CMD_ARGPERM}</SPAN>:<br>";
      print"<INPUT TYPE=text AUTOCOMPLETE=\"off\" SIZE=28 MAXLENGTH=50 NAME=argperm class=PlaneText VALUE=\"$argperm\">";
      &print_examples_textarea('argperm','ex3',20,$conn);

     print"</TD>";

     #-- argdeny [50] ---
     print"<TD WIDTH=180 VALIGN=TOP>";
      print"<SPAN class=InputText>$dict{CMD_ARGDENY}</SPAN>:<br>";
      print"<INPUT TYPE=text AUTOCOMPLETE=\"off\" SIZE=28 MAXLENGTH=50 NAME=argdeny class=PlaneText VALUE=\"$argdeny\">";
      &print_examples_textarea('argdeny','ex4',20,$conn);

     print"</TD>";
     print"</TR>";
     print"</TABLE>"; # InnerTable

    print"</TD></TR></TABLE>";
    print"</FORM>";

    $result = $conn->exec("SELECT count(avpid) FROM tacppd_v1_cmd");
    $error = $conn->errorMessage unless PGRES_TUPLES_OK eq $result->resultStatus;
    print_error_and_exit($dict{CMD_ERROR_1},$conn,$error,$dict{ERROR},$dict{CAUSE}) if $error ne '';
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
    print"<small>[<A HREF=./cmd.pl?w=$switch class=Link>$dict{RETURN}</A>]</small><p>" if $unid ne '';
    #---------------------------------------------------------------------------------

    #--- page line ------
    &print_page_line('cmd.pl',0,$pagenumber,$cicle,$selectpage,$switch,$nextpage,$dict{PAGE},$dict{NEXT_PAGE},$dict{USER},$dict{cmdSearch});

    $cmd = "SELECT unid,avpid,cmdperm,cmddeny,argperm,argdeny 
             FROM tacppd_v1_cmd ";

    #--- title table ---
    print"<p><TABLE class=TitleTable>";
    print"<TR>";
     print"<TD class=TitleTable0></TD>";

       #--- avpid ---
       print"<TD class=TitleTable2 VALIGN=TOP>";
       $cmd = print_sort_table_header($switch,$cmd,1,2,'cmd.pl',$dict{AVP},$dict{SORT_DESC},$dict{SORT_ASC},'avpid','','');
       print"</TD>";
       print"<TD class=TitleTable0></TD>";

       #--- cmdperm ---
       print"<TD class=TitleTable3 VALIGN=TOP>";
       $cmd = print_sort_table_header($switch,$cmd,3,4,'cmd.pl',$dict{CMD_CMDPERM},$dict{SORT_DESC},$dict{SORT_ASC},'cmdperm','','');
       print"</TD>";
       print"<TD class=TitleTable0></TD>";

       #--- cmddeny ---
       print"<TD class=TitleTable3 VALIGN=TOP>";
       $cmd = print_sort_table_header($switch,$cmd,5,6,'cmd.pl',$dict{CMD_CMDDENY},$dict{SORT_DESC},$dict{SORT_ASC},'cmddeny','','');
       print"</TD>";
       print"<TD class=TitleTable0></TD>";

       #--- argperm ---
       print"<TD class=TitleTable3 VALIGN=TOP>";
       $cmd = print_sort_table_header($switch,$cmd,7,8,'cmd.pl',$dict{CMD_ARGPERM},$dict{SORT_DESC},$dict{SORT_ASC},'argperm','','');
       print"</TD>";
       print"<TD class=TitleTable0></TD>";

       #--- argdeny ---
       print"<TD class=TitleTable3 VALIGN=TOP>";
       $cmd = print_sort_table_header($switch,$cmd,9,10,'cmd.pl',$dict{CMD_ARGDENY},$dict{SORT_DESC},$dict{SORT_ASC},'argdeny','','');
       print"</TD>";
       print"<TD class=TitleTable0></TD>";

     print"</TD>";
     print"<TD></TD>";
    print"</TR>";
    print"</TABLE>";

    $cmd .= "LIMIT $init{LIST_PAGE} OFFSET $min_row";
    $result = $conn->exec("$cmd");
    $error = $conn->errorMessage unless PGRES_TUPLES_OK eq $result->resultStatus;
    print_error_and_exit($dict{CMD_ERROR_1},$conn,$error,$dict{ERROR},$dict{CAUSE}) if $error ne '';
    $row = $result->ntuples; 

    #-- Ok ---
    if($row > 0){

        print"<PRE>";
        my($pr_avpid,$pr_cmdperm,$pr_cmddeny,$pr_argperm);

        #--- list ----
        for(my $i=0; $i<$row; $i++){

           $unid = $result->getvalue($i, 0);
           $avpid = $result->getvalue($i, 1);
           $cmdperm = $result->getvalue($i, 2);
           $cmddeny = $result->getvalue($i, 3);
           $argperm = $result->getvalue($i, 4);
           $argdeny = $result->getvalue($i, 5);

           $pr_avpid = sprintf("%-17s",$avpid);

           $pr_cmdperm = $cmdperm;
           $pr_cmddeny = $cmddeny;
           $pr_argperm = $argperm;


           $pr_cmdperm = sprintf("%-26s",$pr_cmdperm);
           $pr_cmdperm = substr("$pr_cmdperm",0,22).'<font color=red>|.. </font>' if length("$pr_cmdperm") > 26; 

           $pr_cmddeny = sprintf("%-26s",$pr_cmddeny);
           $pr_cmddeny = substr("$pr_cmddeny",0,22).'<font color=red>|.. </font>' if length("$pr_cmddeny") > 26; 

           $pr_argperm = sprintf("%-26s",$pr_argperm);
           $pr_argperm = substr("$pr_argperm",0,22).'<font color=red>|.. </font>' if length("$pr_argperm") > 26; 


           ##################
           print"   <A HREF=./cmd.pl?p=$pagenumber&w=$switch&unid=$unid class=Link TITLE=\"$cmdperm   $cmddeny   $argperm   $argdeny\">$pr_avpid $pr_cmdperm $pr_cmddeny  $pr_argperm $argdeny</A><br>";
           ##################

        } #for
        print"</PRE>";


    #--- error ----
    }else{
        print"<PRE>   $dict{NULLROW} </PRE>";
    }

    #--- page line ------
    &print_page_line('cmd.pl',0,$pagenumber,$cicle,$selectpage,$switch,$nextpage,$dict{PAGE},$dict{NEXT_PAGE},$dict{USER},$dict{cmdSearch});

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

    $result = $conn->exec("SELECT DISTINCT $id FROM tacppd_v1_cmd ORDER BY $id");
    $error = $conn->errorMessage unless PGRES_TUPLES_OK eq $result->resultStatus;
    if ($error ne ''){
       print"<font color=red>$error</font>";
    }else{
        $row = $result->ntuples;
        print"<nobr>";
        print"<TEXTAREA WRAP=off NAME=$ta ROWS=4 COLS=$width ReadOnly=\"on\">";
        for(my $i=0; $i<$row; $i++){
             $name = $result->getvalue($i, 0);
             print"$name\n";
        }
        print"</TEXTAREA>";
        print"</nobr>";
    }
}

