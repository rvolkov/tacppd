
#include <iostream>
#include <time.h>

using namespace std;

// convert ISO datetime to time() ticks
// (2001-03-24 10:30:05)
time_t
iso2time(char *iso) {
	char str[20];
	struct tm ttt;
	
	// read year
	int k = 0;
	for(int i=0; i<4; i++,k++) {
		str[k] = iso[i];
	}
	str[k] = 0;	
	ttt.tm_year = atoi(str) - 1900;

//cout<<"year="<<str<<" atoi="<<ttt.tm_year<<endl;

	// read month
	k = 0;
	for(int i=5; i<7; i++,k++) {
		str[k] = iso[i];
	}
	str[k] = 0;	
	ttt.tm_mon = atoi(str) - 1;

//cout<<"mon="<<str<<" atoi="<<ttt.tm_mon<<endl;

	// read day
	k = 0;
	for(int i=8; i<10; i++,k++) {
		str[k] = iso[i];
	}
	str[k] = 0;	
	ttt.tm_mday = atoi(str);

//cout<<"day="<<str<<" atoi="<<ttt.tm_mday<<endl;

	// read hour
	k = 0;
	for(int i=11; i<13; i++,k++) {
		str[k] = iso[i];
	}
	str[k] = 0;	
	ttt.tm_hour = atoi(str);

//cout<<"hour="<<str<<" atoi="<<ttt.tm_hour<<endl;

	// read minutes
	k = 0;
	for(int i=14; i<16; i++,k++) {
		str[k] = iso[i];
	}
	str[k] = 0;	
	ttt.tm_min = atoi(str);

//cout<<"min="<<str<<" atoi="<<ttt.tm_min<<endl;

	// read seconds
	k = 0;
	for(int i=17; i<19; i++,k++) {
		str[k] = iso[i];
	}
	str[k] = 0;	
	ttt.tm_sec = atoi(str);

//cout<<"sec="<<str<<" atoi="<<ttt.tm_sec<<endl;

//cout<<"mktime="<<mktime(&ttt)<<" asctime="<<asctime(&ttt)<<endl;
	return mktime(&ttt);
}

//int main() {
//	char *str = "2001-03-24 10:30:05";
//	time_t t = iso2time(str);
//	time_t *x = &t;
//
//	cout<<"str="<<str<<" time_t="<<t<<" asctime="<<asctime(localtime(x))<<endl;
//}
