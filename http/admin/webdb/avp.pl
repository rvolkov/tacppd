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
my ($avpid,$service,$protocol,$avp,$unid);

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
         <SPAN class=SelectLang>$dict{AVP_BIG}</SPAN>
      </TD>";

print"<TD VALIGN=TOP class=SiteTop3>
         <TABLE><TR>
          <TD WIDTH=70 class=inner><A HREF=.. class=Link>$dict{EXIT}</A></TD>
          <TD WIDTH=70 class=inner> </TD>
          <TD WIDTH=140 colspan=2 class=inner><A HREF=./index.pl class=Link TITLE=\"$dict{LIST_BIG}\">$dict{LIST}</A></TD>
          <TD WIDTH=140 colspan=2 class=inner><A HREF=./edit.pl class=Link TITLE=\"$dict{NEW_BIG}\">$dict{NEW}</A></TD>
         </TR><TR>
          <TD WIDTH=140 colspan=2 class=inner><SPAN class=Link>\L$dict{AVP}\E</SPAN></TD>
          <TD WIDTH=140 colspan=2 class=inner><A HREF=./acc.pl class=Link TITLE=\"$dict{ACC_BIG}\">\L$dict{ACC}\E</A></TD>
          <TD WIDTH=140 colspan=2 class=inner><A HREF=./cmd.pl class=Link TITLE=\"$dict{CMD_BIG}\">\L$dict{CMD}\E</A></TD>
         </TR></TABLE>
      </TD>";
print"</TR></TABLE>";
##################################################################

$conn = Pg::connectdb("host=$init{prim_dbhost} dbname=$init{prim_dbname} user=$init{prim_dbuser} password=$init{prim_dbpassword}");
if (PGRES_CONNECTION_OK == $conn->status) {


    if($unid ne ''){
        $result = $conn->exec("SELECT avpid,service,protocol,avp FROM tacppd_v1_avp WHERE unid=$unid");
        $error = $conn->errorMessage unless PGRES_TUPLES_OK eq $result->resultStatus;
        print_error_and_exit($dict{ERROR21},$conn,$error,$dict{ERROR},$dict{CAUSE}) if $error ne '';
        $avpid = $result->getvalue(0, 0);
        $service = $result->getvalue(0, 1);
        $protocol = $result->getvalue(0, 2);
        $avp = $result->getvalue(0, 3);
    }

    print"<br><FORM METHOD=POST ACTION=./avp_save.pl NAME=AvpForm>";
    print"<TABLE class=SiteTop><TR><TD VALIGN=TOP>";
     print"<INPUT TYPE=hidden NAME='p' VALUE=\"$pagenumber\">";
     print"<INPUT TYPE=hidden NAME='w' VALUE=\"$switch\">";

     print"<TABLE class=InnerTable>";
     print"<TR>";

     #-- avpid [16] ---
     print"<TD WIDTH=160 VALIGN=TOP>";
      print"<SPAN class=InputText>$dict{AVP}</SPAN>:<br>";
      print"<INPUT TYPE=text AUTOCOMPLETE=\"off\" SIZE=20 MAXLENGTH=16 NAME=avpid class=PlaneText VALUE=\"$avpid\">";
      &print_examples_textarea('avpid','ex1',15,$conn);

      print"<br><br>";

      print"<INPUT TYPE=hidden NAME='unid' VALUE=\"$unid\">";
      if($unid ne ''){
          print"<INPUT TYPE='submit' NAME=SaveCmd VALUE=$dict{cmdEdit} class=SaveCmd> ";
      }else{
          print"<INPUT TYPE='submit' NAME=SaveCmd VALUE=$dict{cmdSave} class=SaveCmd> ";
      }

     print"</TD>";

     #-- service [15] ---
     print"<TD WIDTH=160 VALIGN=TOP>";
      print"<SPAN class=InputText>$dict{AVP_SERV}</SPAN>:<br>";
      print"<INPUT TYPE=text AUTOCOMPLETE=\"off\" SIZE=20 MAXLENGTH=15 NAME=service class=PlaneText VALUE=\"$service\">";
      &print_examples_textarea('service','ex2',15,$conn);

      print"<br><br>";
      print"<INPUT TYPE=checkbox NAME=delflag VALUE=1 onClick=SetDeleteMark(AvpForm,\"$dict{cmdDel}\",\"$dict{cmdEdit}\");>";
      print"<SPAN class=Link>$dict{DELFLAG}</SPAN>";

     print"</TD>";

     #-- protocol [15] ---
     print"<TD WIDTH=150 VALIGN=TOP>";
      print"<SPAN class=InputText>$dict{AVP_PROT}</SPAN>:<br>";
      print"<INPUT TYPE=text AUTOCOMPLETE=\"off\" SIZE=20 MAXLENGTH=15 NAME=protocol class=PlaneText VALUE=\"$protocol\">";
      &print_examples_textarea('protocol','ex3',15,$conn);
     print"</TD>";

     #-- avp [128] ---
     print"<TD VALIGN=TOP>";
      print"<SPAN class=InputText>$dict{AVP_AVP}</SPAN>:<br>";
      print"<INPUT TYPE=text AUTOCOMPLETE=\"off\" SIZE=60 MAXLENGTH=128 NAME=avp class=PlaneText VALUE=\"$avp\">";
      &print_examples_textarea('avp','ex4',45,$conn);
     print"</TD>";

     print"</TR>";
     print"</TABLE>";
    print"</TD></TR></TABLE>";
    print"</FORM>";

    $result = $conn->exec("SELECT count(avp) FROM tacppd_v1_avp");
    $error = $conn->errorMessage unless PGRES_TUPLES_OK eq $result->resultStatus;
    print_error_and_exit($dict{AVP_ERROR_1},$conn,$error,$dict{ERROR},$dict{CAUSE}) if $error ne '';
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
    print"<small>[<A HREF=./avp.pl?w=$switch class=Link>$dict{RETURN}</A>]</small><p>" if $unid ne '';
    #---------------------------------------------------------------------------------


    #--- page line ------
    &print_page_line('avp.pl',0,$pagenumber,$cicle,$selectpage,$switch,$nextpage,$dict{PAGE},$dict{NEXT_PAGE},$dict{USER},$dict{cmdSearch});

    $cmd = "SELECT unid,avpid,service,protocol,avp 
             FROM tacppd_v1_avp ";

    #--- title table ---
    print"<p><TABLE class=TitleTable>";
    print"<TR>";
     print"<TD class=TitleTable0></TD>";

       #--- avpid ---
       print"<TD class=TitleTable2 VALIGN=TOP>";
       $cmd = print_sort_table_header($switch,$cmd,1,2,'avp.pl',$dict{AVP},$dict{SORT_DESC},$dict{SORT_ASC},'avpid','','');
       print"</TD>";
       print"<TD class=TitleTable0></TD>";

       #--- service ---
       print"<TD class=TitleTable2 VALIGN=TOP>";
       $cmd = print_sort_table_header($switch,$cmd,3,4,'avp.pl',$dict{AVP_SERV},$dict{SORT_DESC},$dict{SORT_ASC},'service','','');
       print"</TD>";
       print"<TD class=TitleTable0></TD>";

       #--- protocol ---
       print"<TD class=TitleTable2 VALIGN=TOP>";
       $cmd = print_sort_table_header($switch,$cmd,5,6,'avp.pl',$dict{AVP_PROT},$dict{SORT_DESC},$dict{SORT_ASC},'protocol','','');
       print"</TD>";
       print"<TD class=TitleTable0></TD>";

       #--- avp ---
       print"<TD class=TitleTable1 VALIGN=TOP>";
       $cmd = print_sort_table_header($switch,$cmd,7,8,'avp.pl',$dict{AVP_AVP},$dict{SORT_DESC},$dict{SORT_ASC},'avp','','');
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
        #--- list ----
        for(my $i=0; $i<$row; $i++){
           $unid = $result->getvalue($i, 0);
           $avpid = $result->getvalue($i, 1);
           $service = $result->getvalue($i, 2);
           $protocol = $result->getvalue($i, 3);
           $avp = $result->getvalue($i, 4);

           $avpid = sprintf("%-17s",$avpid);
           $service = sprintf("%-17s",$service);
           $protocol = sprintf("%-17s",$protocol);

           ##################
           print"   <A HREF=./avp.pl?p=$pagenumber&w=$switch&unid=$unid class=Link TITLE=\"\">$avpid $service$protocol $avp</A><br>";
           ##################
        } #for
        print"</PRE>";


    #--- error ----
    }else{
        print"<PRE>   $dict{NULLROW} </PRE>";
    }

    #--- page line ------
    &print_page_line('avp.pl',0,$pagenumber,$cicle,$selectpage,$switch,$nextpage,$dict{PAGE},$dict{NEXT_PAGE},$dict{USER},$dict{cmdSearch});


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

    $result = $conn->exec("SELECT DISTINCT $id FROM tacppd_v1_avp ORDER BY $id");
    $error = $conn->errorMessage unless PGRES_TUPLES_OK eq $result->resultStatus;
    if ($error ne ''){
       print"<font color=red>$error</font>";
    }else{
        $row = $result->ntuples;
        print"<TEXTAREA NAME=$ta WRAP=off ROWS=5 COLS=$width ReadOnly=\"on\">";
        for(my $i=0; $i<$row; $i++){
             $name = $result->getvalue($i, 0);
             print"$name\n";
        }
        print"</TEXTAREA>";
    }
}

