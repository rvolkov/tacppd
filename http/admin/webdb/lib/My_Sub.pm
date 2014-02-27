package My_Sub; 
require Exporter; 
@ISA  =  qw(Exporter); 
@EXPORT  =  qw(get_cur_datetime get_num_date print_page_line print_error_and_exit print_sort_table_header);
use Time::Local;
use Time::localtime;
use locale;


sub print_sort_table_header{
    my($switch,$cmd,$f1,$f2,$script,$title,$sort_desc,$sort_asc,$field,$field2,$reverse) = @_;

    #-- [+] --
    if($switch == $f1){
        if ($field2 eq ''){
            $cmd .= " ORDER BY $field ASC ";
        }else{
            $cmd .= " ORDER BY $field ASC, $field2 ";
        }

        print"<SPAN class=Link>$title&nbsp;</SPAN>
              <A HREF=./$script?w=$f2 class=Link TITLE=\"$sort_desc\">&#150;</A> 
              <SPAN class=Link>+</SPAN>";

    #-- [-] --
    }elsif($switch == $f2){
        if ($field2 eq ''){
            $cmd .= " ORDER BY $field DESC ";
        }else{
            $cmd .= " ORDER BY $field DESC, $field2 ";
        }

        print"<SPAN class=Link>$title&nbsp; &#150;</SPAN>
              <A HREF=./$script?w=$f1 class=Link TITLE=\"$sort_asc\">+</A>";

    #-- [+-] --
    }else{
        #-- (+) --
        if($reverse eq ''){
            print"<A HREF=./$script?w=$f1 class=Link TITLE=\"$sort_asc\">$title</A>&nbsp; 
                  <A HREF=./$script?w=$f2 class=Link TITLE=\"$sort_desc\">&#150;</A> 
                  <A HREF=./$script?w=$f1 class=Link TITLE=\"$sort_asc\">+</A>";
        #-- (-) --
        }else{
            print"<A HREF=./$script?w=$f2 class=Link TITLE=\"$sort_desc\">$title</A>&nbsp; 
                  <A HREF=./$script?w=$f2 class=Link TITLE=\"$sort_desc\">&#150;</A> 
                  <A HREF=./$script?w=$f1 class=Link TITLE=\"$sort_asc\">+</A>";
        }
    }
    return($cmd);
}

sub print_error_and_exit{
   my($message,$conn,$error,$dict_error,$dict_cause) = @_;
   $conn->reset;

   print"<hr>";
   print"<font color=red><b>$dict_error!</b><br>";
   print"$message<br>";
   print"<b>$dict_cause:</b> $error";
   print"</font><hr>";
   print"</BODY></HTML>";
   exit;
}


sub print_page_line{
    my($scriptname,$flag,$pagenumber,$cicle,$selectpage,$switch,
       $nextpage,$dict_page,$dict_nextpage,$dict_user,$dict_search,$search) = @_;
    my $viewpage;

    print"<TABLE class=SiteTop>";
    print"<TR><TD VALIGN=TOP class=SiteTop3>";
     print"<SPAN class=Pages>$dict_page: &nbsp;</SPAN>";
     for ($pagenumber = 1; $pagenumber < $cicle; $pagenumber++){
         if($selectpage eq $pagenumber){
             $viewpage = $pagenumber;
             print"<SPAN class=Link> $viewpage </SPAN>";
         }else{
             $viewpage = $pagenumber;
             print" <A HREF=./$scriptname?p=$pagenumber&w=$switch&s=$search class=Link> $viewpage </A> ";
         }
     } #for
     if($selectpage < $nextpage ){
         print" <A HREF=./$scriptname?p=$nextpage&w=$switch&s=$search class=Link title=\"$dict_nextpage\"> &nbsp; &nbsp; &gt; &nbsp; &nbsp; </A> ";
     }
    print"</TD>";
    print"<TD VALIGN=TOP class=SiteTop2>";
     if($flag == 1){
          print"<SPAN class=Pages>$dict_user: &nbsp;</SPAN>";
          print"<INPUT TYPE=text AUTOCOMPLETE=\"off\" SIZE=10 MAXLENGTH=30 NAME=s CLASS=PlaneText>";
          print" <INPUT TYPE='submit' VALUE=\"$dict_search\" class='MiniCmd'>";
     }else{
          print"&nbsp;";
     }
    print"</TD></TR></TABLE>";
}


sub get_cur_datetime {
    my($time) = @_;
    my($obj_time) = localtime($time);
    my($curdata) = sprintf("%02d/%02d/%4d %02d:%02d:%02d", 
                             $obj_time->mon() + 1,
                             $obj_time->mday(),
                             $obj_time->year() + 1900,
                             $obj_time->hour(),
                             $obj_time->min(),
                             $obj_time->sec()
                             );
    return($curdata);
}


sub get_num_date{

   my ($date) = @_;
   my($day,$month,$year,$hour,$min,$sec);

   #-- mm/dd/yyyy hh:mm:ss --
   $month = substr("$date",0,2);
   $day = substr("$date",3,2);
   $year = substr("$date",6,4);
   $hour = substr("$date",11,2);
   $min = substr("$date",14,2);
   $sec = substr("$date",17,2);
   $month = $month - 1;

   return (-1) if $day < 0 || $day > 31;
   return (-1) if $month < 0 || $month > 11;
   return (-1) if $year < 2000 || $year > 3000;
   return (-1) if $hour < 0 || $hour > 23;
   return (-1) if $min < 0 || $min > 59;
   return (-1) if $sec < 0 || $sec > 59;

   $date = timelocal($sec,$min,$hour,$day,$month,$year);
   return($date);
} 


1;