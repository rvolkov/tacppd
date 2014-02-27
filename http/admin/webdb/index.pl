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
my ($username,$dopen,$dclose,$avpid,$accid,$maxsess,$print_dclose,$print_username);

#--- Init local variables ------
my ($selectlang,$usercount,$switch,$pagenumber,$search) = ('','','','','');

#--- Global variables -------
our %init = ();
do "./config/lpmx_config.pl"  || die;

#--- Language ---
$selectlang = $query->param("SelectLang");              # set language
$selectlang = $query->cookie("my_language") unless $selectlang;  # language from cookies
$selectlang = $init{DefaultLang} unless $selectlang;           # default language
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

my $cookie = cookie(
   -name    => 'my_language', 
   -value   => "$selectlang", 
   -expires => '+90d',
   -path    => '/',
   -domain  => ".$ENV{SERVER_NAME}"
);


print header(-cookie => $cookie);
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

         #--- Select language ---
         do "./lib/print_form.pl";
         print_selectlang_form($selectlang);

print"</TD>";
print"<TD VALIGN=TOP class=SiteTop2>
         <SPAN class=big>$dict{TitlePages}</SPAN>
         <br>
         <SPAN class=SelectLang>$dict{LIST_BIG}</SPAN>
      </TD>";
print"<TD VALIGN=TOP class=SiteTop3>
         <TABLE><TR>
          <TD WIDTH=70 class=inner><A HREF=.. class=Link>$dict{EXIT}</A></TD>
          <TD WIDTH=70 class=inner>  </TD>
          <TD WIDTH=140 colspan=2 class=inner><SPAN class=Link>$dict{LIST}</SPAN></TD>
          <TD WIDTH=140 colspan=2 class=inner><A HREF=./edit.pl class=Link TITLE=\"$dict{NEW_BIG}\">$dict{NEW}</A></TD>
         </TR><TR>
          <TD WIDTH=140 colspan=2 class=inner><A HREF=./avp.pl class=Link TITLE=\"$dict{AVP_BIG}\">\L$dict{AVP}\E</A></TD>
          <TD WIDTH=140 colspan=2 class=inner><A HREF=./acc.pl class=Link TITLE=\"$dict{ACC_BIG}\">\L$dict{ACC}\E</A></TD>
          <TD WIDTH=140 colspan=2 class=inner><A HREF=./cmd.pl class=Link TITLE=\"$dict{CMD_BIG}\">\L$dict{CMD}\E</A></TD>
         </TR></TABLE>
      </TD>";
print"</TR></TABLE>";
##################################################################


$conn = Pg::connectdb("host=$init{prim_dbhost} dbname=$init{prim_dbname} user=$init{prim_dbuser} password=$init{prim_dbpassword}");
if (PGRES_CONNECTION_OK == $conn->status) {

    my $curtime = get_cur_datetime(time);
    if($switch == 7){
        $result = $conn->exec("SELECT count(username) FROM tacppd_v1_usr WHERE dclose < '$curtime'");
    }elsif($search ne ''){
        $result = $conn->exec("SELECT count(username) FROM tacppd_v1_usr WHERE LOWER(username) LIKE LOWER('%$search%')");
    }else{
        $result = $conn->exec("SELECT count(username) FROM tacppd_v1_usr");
    }
    $error = $conn->errorMessage unless PGRES_TUPLES_OK eq $result->resultStatus;
    print_error_and_exit($dict{ERROR_1},$conn,$error,$dict{ERROR},$dict{CAUSE}) if $error ne '';
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


    print"<br><FORM METHOD=POST ACTION='index.pl' NAME='SearchForm'>";
    print"<SPAN class=SelectLang>$dict{INDEX_TOTU}: <b>$usercount</b> &nbsp; &nbsp;";
    print"$dict{INDEX_PAGEU}: <b>$init{LIST_PAGE}</b></SPAN> &nbsp; &nbsp;";
    print"<SPAN class=SelectLang>$dict{INDEX_SEARCH}: <b>$search</b></SPAN>" if $search ne '';

      #--- page line ------
      &print_page_line('index.pl',1,$pagenumber,$cicle,$selectpage,$switch,$nextpage,$dict{PAGE},$dict{NEXT_PAGE},$dict{USER},$dict{cmdSearch},$search);

    print"</FORM>";

    $cmd = "SELECT username,date_part('epoch',dopen),date_part('epoch',dclose),avpid,accid,maxsess 
             FROM tacppd_v1_usr ";

    #--- title table ---
    print"<TABLE class=TitleTable>";
    print"<TR>";
     print"<TD class=TitleTable0></TD>";

     #--- username (+) ---
     print"<TD class=TitleTable1 VALIGN=TOP>";
     #-- [search] --
     if($search ne ''){
         $cmd .= " WHERE LOWER(username) LIKE LOWER('%$search%') ORDER BY username ASC ";
         print"<SPAN class=Link>$dict{USERNAME}</SPAN>&nbsp; 
             <A HREF=./index.pl?w=2 class=Link TITLE=\"$dict{SORT_DESC}\">&#150;</A> 
             <A HREF=./index.pl?w=1 class=Link TITLE=\"$dict{SORT_ASC}\">+</A>";
     }else{
         $cmd = print_sort_table_header($switch,$cmd,1,2,'index.pl',$dict{USERNAME},$dict{SORT_DESC},$dict{SORT_ASC},'username','','');
     }
     print"</TD>";
     print"<TD class=TitleTable0></TD>";

     #--- dopen (-) ---
     print"<TD class=TitleTable1 VALIGN=TOP>";
     $cmd = print_sort_table_header($switch,$cmd,3,4,'index.pl',$dict{DOPEN},$dict{SORT_DESC},$dict{SORT_ASC},'dopen','username','r');
     print"</TD>";
     print"<TD class=TitleTable0></TD>";

     #--- dclose (#) ---
     print"<TD class=TitleTable1 VALIGN=TOP>";
         #-- [+] --
         if($switch == 5){
           $cmd .= " ORDER BY dclose ASC, username ";
           print"<SPAN class=Link>$dict{DCLOSE}</SPAN>&nbsp; 
                 <A HREF=./index.pl?w=6 class=Link TITLE=\"$dict{SORT_DESC}\">&#150;</A> 
                 <SPAN class=Link>+</SPAN>
                 <A HREF=./index.pl?w=7 class=Link TITLE=\"$dict{SORT_CLOSE}\">#</A>
                ";
         #-- [-] --
         }elsif($switch == 6){
           $cmd .= " ORDER BY dclose DESC, username ";
           print"<SPAN class=Link>$dict{DCLOSE}&nbsp;  
                 &#150;</SPAN> 
                 <A HREF=./index.pl?w=5 class=Link TITLE=\"$dict{SORT_ASC}\">+</A>
                 <A HREF=./index.pl?w=7 class=Link TITLE=\"$dict{SORT_CLOSE}\">#</A>
                ";
         #-- [#] --
         }elsif($switch == 7){
           $cmd .= " WHERE dclose < '$curtime' ORDER BY dclose DESC, username ";
           print"<SPAN class=Link>$dict{DCLOSE}</SPAN>&nbsp; 
                 <A HREF=./index.pl?w=6 class=Link TITLE=\"$dict{SORT_DESC}\">&#150;</A> 
                 <A HREF=./index.pl?w=5 class=Link TITLE=\"$dict{SORT_ASC}\">+</A>
                 <SPAN class=Link>#</SPAN>
                ";
         #-- [+-#] --
         }else{
           print"<A HREF=./index.pl?w=7 class=Link TITLE=\"$dict{SORT_CLOSE}\">$dict{DCLOSE}</A>&nbsp; 
                 <A HREF=./index.pl?w=6 class=Link TITLE=\"$dict{SORT_DESC}\">&#150;</A> 
                 <A HREF=./index.pl?w=5 class=Link TITLE=\"$dict{SORT_ASC}\">+</A>
                 <A HREF=./index.pl?w=7 class=Link TITLE=\"$dict{SORT_CLOSE}\">#</A>
                ";
         }
     print"</TD>";
     print"<TD class=TitleTable0></TD>";

     #--- avpid (+) ---
     print"<TD class=TitleTable2 VALIGN=TOP>";
     $cmd = print_sort_table_header($switch,$cmd,8,9,'index.pl',$dict{AVP},$dict{SORT_DESC},$dict{SORT_ASC},'avpid','username','');
     print"</TD>";
     print"<TD class=TitleTable0></TD>";

     #--- accid ---
     print"<TD class=TitleTable2 VALIGN=TOP>";
     $cmd = print_sort_table_header($switch,$cmd,10,11,'index.pl',$dict{ACC},$dict{SORT_DESC},$dict{SORT_ASC},'accid','username','');
     print"</TD>";
     print"<TD class=TitleTable0></TD>";

     #--- maxsess ---
     print"<TD class=TitleTable2 VALIGN=TOP>";
     $cmd = print_sort_table_header($switch,$cmd,12,13,'index.pl',$dict{MAXSESS},$dict{SORT_DESC},$dict{SORT_ASC},'maxsess','username','r');
     print"</TD>";
     print"<TD></TD>";

    print"</TR>";
    print"</TABLE>";

    $cmd .= "LIMIT $init{LIST_PAGE} OFFSET $min_row";
    $result = $conn->exec("$cmd");
    $error = $conn->errorMessage unless PGRES_TUPLES_OK eq $result->resultStatus;
    print_error_and_exit($dict{ERROR_1},$conn,$error,$dict{ERROR},$dict{CAUSE}) if $error ne '';
    $row = $result->ntuples; 

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
             $print_username = substr("$print_username",0,21).'..' if length("$print_username") > 21; 
             $print_username = sprintf("%-23s",$print_username);
             $avpid = sprintf("%-17s",$avpid);
             $accid = sprintf("%-17s",$accid);
             $maxsess = sprintf("%12s",$maxsess);
             $dopen = get_cur_datetime($dopen);
             $print_dclose = '';
             $print_dclose = get_cur_datetime($dclose) if $dclose ne '';
             $print_dclose = sprintf("%19s",$print_dclose);

             #************
             $print_username =~ s/($search)/<font color=blue>$1<\/font>/i if $search ne '';
             if($dclose < time){
                 print"   <A HREF=./edit.pl?p=$pagenumber&w=$switch&en=$username&s=$search class=Link TITLE=\"$username\">$print_username $dopen</A>    <font color=red>$print_dclose</font>      $avpid$accid $maxsess<br>";
             }else{
                 print"   <A HREF=./edit.pl?p=$pagenumber&w=$switch&en=$username&s=$search class=Link TITLE=\"$username\">$print_username $dopen</A>    $print_dclose      $avpid$accid $maxsess<br>";
             }
             #************

        } #for
        print"</PRE>";

    #--- error ----
    }else{
        if($search eq ''){
            print"<PRE>   <font color=blue>$dict{NULLROW}</font></PRE>";
           
        }else{
            print"<PRE>   $dict{ERROR_3}: ";
            print"<font color=blue>$search</font></PRE>";
        }
    }

    #--- page line ------
    print"<p>";
    &print_page_line('index.pl',0,$pagenumber,$cicle,$selectpage,$switch,$nextpage,$dict{PAGE},$dict{NEXT_PAGE},$dict{USER},$dict{cmdSearch},$search);

    $conn->reset;

}else{
    $error = $conn->errorMessage;
    print"$dict{ConnectError} <font color=red>$error</font>";
}
print"</BODY></HTML>";

########################################################################
########################################################################
########################################################################
