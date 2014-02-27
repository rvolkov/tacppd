
sub print_selectlang_form{

    my ($selectlang) = @_;
    my @list = ();
    my $list;

    print <<END;

    <DIV class=SelectLang>
    <FORM METHOD=POST NAME='SelectLang' ACTION=./index.pl>
    <SPAN class=SelectLang>$dict{lblSwitchLang}</SPAN><br>
    <SELECT NAME='SelectLang' class=SelectLang>
END
    opendir FILE_LIST, "./dictionary";
     @list = grep !/^\.\.?$/ && !/^read\.me/ && !/^index/ , readdir FILE_LIST;
     @list=sort(@list);
    closedir FILE_LIST; 
    foreach $list (@list){
       if($selectlang eq $list){
           print"<option value=\"$list\" selected> $list";
       }else{
           print"<option value=\"$list\"> $list";
       }
    }
    print <<END;

    </SELECT><br>
    <INPUT TYPE='submit' VALUE="$dict{cmdSwitchLang}" class='SelectLang'>
    </FORM>
    </DIV>
END
}

1;

