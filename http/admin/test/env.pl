#!/usr/bin/perl

#------------------
print "Content-type: text/html\n\n"; 
print"<HEAD><TITLE>ENV-TEST</TITLE></HEAD>";
print"<BODY BGCOLOR=#FFFFFF>";

my ($buffer);

if ($ENV{REQUEST_METHOD} eq "GET") {

    print"Чтение <b>GET</b> &nbsp; &nbsp; \$ENV{REQUEST_METHOD} = $ENV{REQUEST_METHOD}<br>";
    print"\$ENV{QUERY_STRING} = $ENV{QUERY_STRING}<br>";

    $buffer = $ENV{QUERY_STRING};
    print"<br>STRING = $buffer";


} elsif ($ENV{REQUEST_METHOD} eq "POST") {

    print"Чтение <b>POST</b> &nbsp; &nbsp; \$ENV{REQUEST_METHOD} = $ENV{REQUEST_METHOD}<br>";
    print"\$ENV{CONTENT_LENGTH} = $ENV{CONTENT_LENGTH}<br>";
    print"\$ENV{QUERY_STRING} = $ENV{QUERY_STRING}<br>";


    read(STDIN, $buffer, $ENV{CONTENT_LENGTH});
    print"<br>STRING = $buffer";
    print"<br>---- decoding --------";
    $buffer =~ s/%([\dA-Fa-f][\dA-Fa-f])/pack ("C", hex ($1)) /eg; 
    print"<br>STRING = $buffer\n";


}else{

    print"<font color=red>Метод не распознан!</font> &nbsp; &nbsp; \$ENV{REQUEST_METHOD} = $ENV{REQUEST_METHOD}<br>\n";

}

print"</BODY>\n";
