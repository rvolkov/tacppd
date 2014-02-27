#!/usr/bin/perl

use MD5;

my ($str,$cryptokey,$varkey,$query,$result);

# The key which you should set in config file for database
$key='your database key';

# username
$var='username';

# password
$str='password';


my $md5 = new MD5;
print "password=",$str," key=",$key," username=",$var,"\n";

$enc = $md5->tac_encrypt($str,$key,$var);
print "encoded password=",$enc,"\n";

$dec = $md5->tac_decrypt($enc,$key,$var);
print "decoded password=",$dec,"\n";
