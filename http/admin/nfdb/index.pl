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
my ($conn,$result,$error,$row,$cmd);
my ($min_row,$max_row,$cicle,$selectpage,$prepage,$nextpage);
my ($sysyyyymm,$splityyyymm);
my ($usernet,$name,$comment,$print_name,$print_usernet,$yyyymm,$bytes_in,$bytes_out);

#--- Init local variables ------
my ($selectlang,$usercount,$switch,$pagenumber,$search) = ('','','','','');

#--- Global variables -------
our %init = ();
do "./config/lpmx_config.pl"  || die;

#--- Language ---
$selectlang = $query->param("SelectLang");                                 # set language
$selectlang = $query->cookie("my_language") unless $selectlang;          # language from cookies
$selectlang = $init{DefaultLang} unless $selectlang;                     # default language
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


################################
$usernet = $query->param("ip");
$name = $query->param("name");
$yyyymm = $query->param("yyyymm");
################################


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
         </TR></TABLE>
      </TD>";
print"</TR></TABLE>";
##################################################################



$conn = Pg::connectdb("host=$init{prim_dbhost} dbname=$init{prim_dbname} user=$init{prim_dbuser} password=$init{prim_dbpassword}");
if (PGRES_CONNECTION_OK == $conn->status) {
    my $curtime = get_cur_datetime(time);


    #--- Main table ------
    ##################################################################
    print"<p>";
    print"<TABLE class=SiteTopBlue>";
    print"<TR>";
    print"<TD VALIGN=TOP>";
      print"<FORM METHOD=POST ACTION='index.pl' NAME='ViewForm'>";
      print"<INPUT TYPE=hidden NAME=name VALUE=''>";
      print"<INPUT TYPE=hidden NAME=p VALUE=''>";
      print"<INPUT TYPE=hidden NAME=s VALUE=''>";
      print"<INPUT TYPE=hidden NAME=w VALUE=''>";
      print"IP: ";
      print"<INPUT TYPE=text AUTOCOMPLETE=\"off\" SIZE=20 MAXLENGTH=60 NAME=ip CLASS=PlaneText VALUE=\"$usernet\">";

      print"&nbsp; YYYYMM: ";
        $result = $conn->exec("SELECT relname FROM pg_stat_user_tables WHERE relname LIKE 'nf_tot_%' ORDER BY relname DESC");
        $error = $conn->errorMessage unless PGRES_TUPLES_OK eq $result->resultStatus;
        print_error_and_exit($dict{ERROR_1},$conn,$error,$dict{ERROR},$dict{CAUSE}) if $error ne '';
        $row = $result->ntuples; 
      print"\n<SELECT NAME='yyyymm' class=SelectLang>";
      if($row > 0){
         for(my $i = 0; $i < $row; $i++){
              $sysyyyymm = $result->getvalue($i, 0);
              $splityyyymm = $sysyyyymm;
              $splityyyymm =~ s/^nf_tot_//;
              if($yyyymm eq $sysyyyymm){
                  print"<OPTION SELECTED VALUE=$sysyyyymm>$splityyyymm";
              }else{
                  print"<OPTION VALUE=$sysyyyymm>$splityyyymm";
              }
         }
      }
      print"</SELECT>";
      print"&nbsp; <INPUT TYPE='submit' VALUE=\"$dict{cmdView}\" class='MiniCmd'>";
      if ($usernet ne ''){
          print" &nbsp; &nbsp; &nbsp;  &nbsp;  &nbsp;<INPUT TYPE='button' VALUE=\"$dict{cmdClear}\" class='MiniCmdGr' onClick=\"window.document.ViewForm.ip.value=''; FormOnSubmit(window.document.ViewForm)\"; >" ;
          print" &nbsp; <INPUT TYPE='button' VALUE=\"$dict{cmdDown}\" class='MiniCmdGr' onClick=\"window.scroll(0,99999)\">";
      }
      print"</FORM>";

        #-->>>---------- Вывод трафика --------------------------------
        if($usernet ne ''){

           #--- Выбрали уникальные имя-порты ---
           my $cmd = "SELECT name,ip, SUM(bytes_in),SUM(bytes_out) FROM $yyyymm ";
           #--- подсеть ---
           if($usernet =~ /\//){
               $cmd .= "WHERE name = '$name' ";
           #--- одиночный адрес ---
           }else{
               $cmd .= "WHERE ip = '$usernet' OR ip = '$usernet/32' ";
           }
           $cmd .= "GROUP BY name,ip ORDER BY ip";
           $result = $conn->exec("$cmd");
           $error = $conn->errorMessage unless PGRES_TUPLES_OK eq $result->resultStatus;
           $row = $result->ntuples; 

           #--- Побежали по каждому имени-ip с построением таблицы трафика ---
           for (my $i = 0; $i < $row; $i++){
               $name = $result->getvalue($i, 0);
               $usernet = $result->getvalue($i, 1);
               $bytes_in = $result->getvalue($i, 2);
               $bytes_out = $result->getvalue($i, 3);

               #--- Рисуем таблички ---------
               print_table_with_traffic($name,$usernet,$bytes_in,$bytes_out,$yyyymm,$conn);
           }
        }
        #--<<<---------- Вывод трафика --------------------------------

    print"</TD>";
    print"</TR></TABLE>";
    ##################################################################



    if($search ne ''){
        $result = $conn->exec("SELECT count(usernet) FROM tacppd_v1_nfuser WHERE LOWER(name) LIKE LOWER('%$search%')");
    }else{
        $result = $conn->exec("SELECT count(usernet) FROM tacppd_v1_nfuser");
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


    print"<FORM METHOD=POST ACTION='index.pl' NAME='SearchForm'>";
    print"<SPAN class=SelectLang>$dict{INDEX_TOTU}: <b>$usercount</b> &nbsp; &nbsp;";
    print"$dict{INDEX_PAGEU}: <b>$init{LIST_PAGE}</b></SPAN> &nbsp; &nbsp;";
    print"<SPAN class=SelectLang>$dict{INDEX_SEARCH}: <b>$search</b></SPAN>" if $search ne '';

      #--- page line ------
      &print_page_line('index.pl',1,$pagenumber,$cicle,$selectpage,$switch,$nextpage,$dict{PAGE},$dict{NEXT_PAGE},$dict{USER},$dict{cmdSearch},$search);

    print"</FORM>";

    $cmd = "SELECT usernet,name,comment 
             FROM tacppd_v1_nfuser ";

    #--- title table ---
    print"<TABLE class=TitleTable>";
    print"<TR>";
     print"<TD class=TitleTable0></TD>";

     #--- name (+) ---
     print"<TD class=TitleTable1 VALIGN=TOP>";
     #-- [search] --
     if($search ne ''){
         $cmd .= " WHERE LOWER(name) LIKE LOWER('%$search%') ORDER BY name ASC ";
         print"<SPAN class=Link>$dict{USERNAME}</SPAN>&nbsp; 
             <A HREF=./index.pl?w=2 class=Link TITLE=\"$dict{SORT_DESC}\">&#150;</A> 
             <A HREF=./index.pl?w=1 class=Link TITLE=\"$dict{SORT_ASC}\">+</A>";
     }else{
         $cmd = print_sort_table_header($switch,$cmd,1,2,'index.pl',$dict{USERNAME},$dict{SORT_DESC},$dict{SORT_ASC},'name','','');
     }
     print"</TD>";
     print"<TD class=TitleTable0></TD>";

     #--- ip (+) ---
     print"<TD class=TitleTable1 VALIGN=TOP>";
     $cmd = print_sort_table_header($switch,$cmd,3,4,'index.pl',$dict{IP},$dict{SORT_DESC},$dict{SORT_ASC},'usernet','','');
     print"</TD>";
     print"<TD class=TitleTable0></TD>";

     #--- comment (+) ---
     print"<TD class=TitleTable1 VALIGN=TOP>";
     $cmd = print_sort_table_header($switch,$cmd,5,6,'index.pl',$dict{COMMENT},$dict{SORT_DESC},$dict{SORT_ASC},'comment','','');
     print"</TD>";
     print"<TD class=TitleTable0></TD>";

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
            $usernet = $result->getvalue($i, 0);
            $name = $result->getvalue($i, 1);
            $comment = $result->getvalue($i, 2);
            $name=~ s/\s+$//;
            $usernet=~ s/\s+$//;
            $print_name = $name;
            $print_usernet = $usernet;
            $print_name = substr("$print_name",0,21).'..' if length("$print_name") > 21; 
            $print_name = sprintf("%-23s",$print_name);
            $print_usernet = sprintf("%-23s",$print_usernet);

            print"<A href=\"javascript:paste('$usernet','$name','$pagenumber','$search','$switch',window.document.ViewForm)\" class=Link>   $print_name $print_usernet $comment</A><br>";


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

sub print_table_with_traffic{

    my ($name,$ip,$bytes_in,$bytes_out,$yyyymm,$conn) = @_;
    my ($result,$row,$start_date,$yyyy,$mm);
    my ($spider,$sp_yyyy,$sp_mm,$sp_dd,$dd,$flag,$formprint);

    $bytes_in = sprintf("%2.2f",$bytes_in/1024/1024); 
    $bytes_out = sprintf("%2.2f",$bytes_out/1024/1024); 

    print"NAME: <b>$name</b> &nbsp; ";
    print"IP: <b>$ip</b>  &nbsp; ";
    print"MBytes IN: <b><font color=blue>$bytes_in</font></b> &nbsp; ";
    print"MBytes OUT: <b><font color=red>$bytes_out</font></b> <br> ";
    #------------------------------------
    print"<TABLE class=listtable>";
    print"<TR class=listtable>";
     print"<TD class=nocolor><center>Date/Traffic</TD>";
     print"<TD class=blue><center>Bytes IN</TD>";
     print"<TD class=blue><center>MBytes IN</TD>";
     print"<TD class=red><center>Bytes OUT</TD>";
     print"<TD class=red><center>MBytes OUT</TD>";
    print"</TR>";
    #------------------------------------
    $result = $conn->exec("SELECT SUM(bytes_in) AS bytes_in,SUM(bytes_out) AS bytes_out, date_part('day',currenttime) AS currenttime FROM $yyyymm WHERE name='$name' AND ip='$ip' GROUP BY date_part('day',currenttime) ORDER BY currenttime");
    $yyyymm =~ s/^nf_tot_//;
    ($yyyy,$mm) = unpack("A4 A2", $yyyymm); 
    $start_date = get_num_date("$mm/01/$yyyy 00:00:00");
    #------------------------------------
    $spider = 0;
    $flag = 0;
    $dd = '';
    for(;;){
        $spider++;
        ($sp_yyyy,$sp_mm,$sp_dd) = get_year_month_from_date($start_date);
        #--- вывал, если пошел другой месяц -----
        last if $sp_mm ne $mm; 

        print"<TR class=listtable>";
        print"<TD class=nocolor><A HREF='javascript:open_child(\"./moredetail.pl?ip=$ip&d=$start_date\",550,600);void(0);' class=Link>$sp_dd\.$sp_mm\.$sp_yyyy</A></TD>";

          if($flag == 0){
              ($bytes_in,$bytes_out,$dd) = $result->fetchrow;
              $dd = sprintf("%02d", $dd);
          }

          if($dd eq $sp_dd){
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

        $start_date += 86400;
        last if $spider > 33;
    }
    print"</TABLE>";
    print"<br><br>";

}

#-- Получили числовой эквивалент даты --
sub get_num_date{
   my ($date) = @_;
   my($day,$month,$year,$hour,$min,$sec);
  # 11/23/2000 09:58:32
   $month = substr("$date",0,2);
   $day = substr("$date",3,2);
   $year = substr("$date",6,4);
   $hour = substr("$date",11,2);
   $min = substr("$date",14,2);
   $sec = substr("$date",17,2);
   $month=$month-1;
   $date = timelocal($sec,$min,$hour,$day,$month,$year);
   return($date);
}

sub get_year_month_from_date{
   my ($time) = @_;
   my($obj_time) = localtime($time);
   my $mm = sprintf("%02d", $obj_time->mon() + 1 );
   my $yyyy = $obj_time->year() + 1900;
   my $dd = sprintf("%02d", $obj_time->mday() );
   return($yyyy,$mm,$dd);
}
