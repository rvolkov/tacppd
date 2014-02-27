
// convert time() to ISO datetime format
// (2001-03-24 10:30:05)
static char*
time2iso(time_t t) {
	struct tm *ttt;
	struct tm res;
	ttt = localtime_r(&t,&res);
	static char str[64];
	bzero(str,sizeof(str));
	snprintf(str,sizeof(str)-1,"%04d-%02d-%02d %02d:%02d:%02d",
		1900+ttt->tm_year,ttt->tm_mon+1,ttt->tm_mday,
			ttt->tm_hour,ttt->tm_min,ttt->tm_sec);
	return str;
}
