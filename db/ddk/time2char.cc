
// convert time() value to string for store in char() fields
char*
time2char(time_t t) {
	static char str[100];
	snprintf(str,sizeof(str)-1,"%u",t);
	return str;
}
