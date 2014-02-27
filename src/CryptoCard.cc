// cryptocard
// (c) Copyright in 2000-2007 by Roman Volkov and contributors
// see http://tacppd.org for more information
//
// file: CryptoCard.cc
// description: CryptoCard support
//
// Used CryptoCard code written by Alec Peterson <ahp@hilander.com>

//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted, see COPYING file

/*
The cryptocard code use libdes package with next copyright notice:


Copyright (C) 1995-1997 Eric Young (eay@cryptsoft.com)
All rights reserved.

This package is an DES implementation written by Eric Young (eay@cryptsoft.com).
The implementation was written so as to conform with MIT's libdes.

This library is free for commercial and non-commercial use as long as
the following conditions are aheared to.  The following conditions
apply to all code found in this distribution.

Copyright remains Eric Young's, and as such any Copyright notices in
the code are not to be removed.
If this package is used in a product, Eric Young should be given attribution
as the author of that the SSL library.  This can be in the form of a textual
message at program startup or in documentation (online or textual) provided
with the package.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. All advertising materials mentioning features or use of this software
   must display the following acknowledgement:
   This product includes software developed by Eric Young (eay@cryptsoft.com)

THIS SOFTWARE IS PROVIDED BY ERIC YOUNG ``AS IS'' AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE.

The license and distribution terms for any publically available version or
derivative of this code cannot be changed.  i.e. this code cannot simply be
copied and put under another distrubution license
[including the GNU Public License.]

The reason behind this being stated in this direct manner is past
experience in code simply being copied and the attribution removed
from it and then being distributed as part of other packages. This
implementation was a non-trivial and unpaid effort.
*/

// Uses the challenge and key passed in and determines if the
// passed in response is correct or not.  If the response is valid, return
// 0 else return 1.  If decimal only results are being used, set hexflag
// to 0 otherwise pass a 1.  If the response string has a '-' at the
// 4th character position, the response is interpreted in telephone
// display format.

#include "global.h"

//#include <stdio.h>
//#include <ctype.h>
//#include <string.h>

//#include "cryptocard.h"
//#include <des.h>
//#include "md5.h"

# define BinToA(c)		((c) <= 9 ? (c) + '0' : ((c) - 10 + 'A'))

// for debugging messages
extern int debug_flag;

// swap bytes function (review it for several achitectures)
void
CryptoCard::swap(unsigned char *p) {
#ifdef i386
#define SWAP_BYTES(a, b)
#else /* i386 */
#define SWAP_BYTES(a, b)	c = a; a = b; b = c
	unsigned char c;
#endif /* i386 */
	SWAP_BYTES(p[0], p[3]); SWAP_BYTES(p[1], p[2]);
	SWAP_BYTES(p[4], p[7]); SWAP_BYTES(p[5], p[6]);
}

// generate cc response
char*
CryptoCard::generate_response(char *key,char *challenge,char *nextchallenge) {
	unsigned char c;
	unsigned long k[2];		// the key as 8 bytes
	unsigned char data[8];	// the data to encrypt
	unsigned char rb[8];		// the encrypted result
	static char resps[9];	// the calculated response as a string
	short i;						// counter
	des_key_schedule schedule;	// Processed DES key

	// parse the key from the input string
	char *kc = (char *)k;
	int temp[8];

	if(key[0] == '0' && key[1] == 'x') {
		// key is in hexadecimal
		key += 2;
		sscanf(key, "%2x%2x%2x%2x%2x%2x%2x%2x",
			&temp[0], &temp[1], &temp[2], &temp[3],
			&temp[4], &temp[5], &temp[6], &temp[7]);
	} else {
		// key is in octal digits (as entered to cryptocard)
		sscanf(key, "%3o%3o%3o%3o%3o%3o%3o%3o",
			&temp[0], &temp[1], &temp[2], &temp[3],
			&temp[4], &temp[5], &temp[6], &temp[7]);
	}
	for (i = 0; i < 8; i++) kc[i] = temp[i];
	// initialize data to encrypt
	bcopy(challenge, data, 8);
	// initialize the key for 16 bit algorithm
	des_set_key(k, schedule);
	swap(data);
	des_encrypt(data, rb, schedule, 1);
	swap(rb);

	// derive the "next challenge" from the current calculated response
	if(nextchallenge) {
		for(i = 0; i < 8; i++) {
			*nextchallenge = rb[i] & 0x0f;
			if(*nextchallenge >= 10) *nextchallenge -= 10;
			*nextchallenge++ += '0';
		}
		*nextchallenge = 0;	// terminating null
	}
   
	// convert the first 4 bytes of result to a string
	bzero(resps, 9);

	for(i = 0 ; i < 4 ; i++) {
		resps[i * 2] = BinToA((rb[i] >> 4) & 0xf);
		resps[(i * 2) + 1] = BinToA(rb[i] & 0xf);
	}
	return resps;
}

// check cc response
int
CryptoCard::check_response(char *key, char *challenge,
							char *response, int flag,char *nextchallenge) {
	char *digits = "0123456789";
	char *res = response;
	while(*res) *res++ = toupper(*res);
	res = generate_response(key, challenge, nextchallenge);

	// make it all decimal if requested
	if(flag == '2' || flag == '3') {
		char *p = res;
		// map hex characters 'A' thru 'F' as 0 thru 5 respectively
		while(*p) {
			if( *p >= 'A' ) *p = digits[ *p - 'A' ];
			else *p = digits[ *p - '0' ];
			p++;
		}
	}
	// handle telephone format results
	if(flag == '1' || flag == '3') res[3] = '-';
	TLOG(LOG_CC, "user's response: %s, expected response: %s",
		response, res);	
	return (!strcmp(response, res));
}

// generate challenge ?
char*
CryptoCard::random_challenge(char *challenge) {
	unsigned long k[2];				// key
	unsigned char data[8];			// the data to encrypt
	time_t timer;						// current time counter
	unsigned char rb[8];				// result
	short i;								// loop counter
	des_key_schedule schedule;		// Processed DES key

	time(&timer);
	data[1] = 0L;
	data[0] = timer;

	// initialize the key for 16 bit algorithm using
	// whatever is on the stack
	des_set_key(k, schedule);

	swap(data);
	des_encrypt(data, rb, schedule, 1);
	swap(rb);

	// convert the first 4 bytes of result to a string
	bzero(challenge, 9);

	for(i = 0 ; i < 4 ; i++) {
		challenge[i * 2] = BinToA((rb[i] >> 4) & 0xf);
		challenge[(i * 2) + 1] = BinToA(rb[i] & 0xf);
	}

	for (i = 0 ; i < 9 ; i++)
		// A through F get converted to 0 through 5 respectively
		if (challenge[i] > '9')
			challenge[i] = (challenge[i] - 'A') + '0';

	TLOG(LOG_CC, "random challenge = %s", challenge);	
	return (challenge);
}

// cc_getchal opens the cryptocard database file in *filename.
// It searches for *username and gets the next challenge from the file.
// If next challenge does not exist in the file, a random challenge is
// generated. Return 0 if successful, posts a message to the system
// errlog if an error occurs
int
CryptoCard::getchal(char *filename,char *username,char *nxtChallenge) {
	char file_line[MAX_LINE_LEN + 1];
	char tmp[MAX_LINE_LEN];
	char key[MAX_LINE_LEN];
	char response[MAX_LINE_LEN];
	char challenge[MAX_LINE_LEN];
	char display[MAX_LINE_LEN];
	char buffer[MAX_LINE_LEN];
	off_t user_offset;
	FILE* fp;
	int result, i;
	char *s;
	char tmp_key[49];

//	char username[32];
//	snprintf(username,sizeof(username),"%s",t.username);
	
	TLOG(LOG_CC,"cc_getchal: Looking for %s in file",username);
	tmp[0] = '\0';

	// Open the database file containing CRYPTOCard keys read write.
	fp = fopen(filename, "r");
	if ( fp == NULL ) {
		TLOG(LOG_CC, "cc_getchal: Can't open %s", filename);
		return(-1);
	}
	//* Now read the file and find the user name
	do {
		user_offset = ftell(fp);
		s = fgets(file_line, MAX_LINE_LEN, fp);
		if(feof(fp)) break;
		if(*s == '#') continue;					// comment string
		result = sscanf(file_line, "%s %s %s %s", tmp, key, display, challenge);
		if (tmp) {
			bcopy(key, tmp_key, 48);
			for (i = 0; i < OCT_KEY_LEN; i++) {
				sscanf(&tmp_key[2*i], "%2x", &key[i]);
			}
			if(!crypt_key(tmp, key, DES_DECRYPT)) return(-1);
				TLOG(LOG_CC, "cc_getchal: Found %s with key = %s and challenge = %s", tmp, key,
			}
			if( (result != 4) && (result != 3) ) tmp[0] = '\0';
	} while (strcmp(username, tmp));

	if( strcmp(tmp, username) ) {  /* No key for user */
		TLOG(LOG_CC, "cc_getchal: Nothing in %s or missing challenge for %s",
			filename, username );
		fclose(fp);
		return(-1);
	}

	// copy the next challenge to the user parameter
	memcpy(nxtChallenge, (result == 4)? challenge : random_challenge(challenge), 8);
	nxtChallenge[8] = '\0';
	TLOG(LOG_CC, "cc_getchal: Challenge user %s with %s", username, nxtChallenge );
	// close the file and return
	fclose(fp);
	return(0);
}

// cc_checkresp compares the response passed in to the response expected
int
CryptoCard::checkresp(db_user t,char *response,
			char *challenge) {
	char file_line[MAX_LINE_LEN + 1];
	char tmp[MAX_LINE_LEN];
	char key[MAX_LINE_LEN];
	char file_challenge[MAX_LINE_LEN];
	char *s;
	char buffer[MAX_LINE_LEN];
	char nxtChallenge[MAX_LINE_LEN];
	char display[MAX_LINE_LEN];
	off_t user_offset;
	FILE* fp;
	int result, i, temp;
	char tmp_key[49];

	char username[32];
	snprintf(username,sizeof(username),"%s",t.username);
	snprintf(key,sizeof(key),"%s",t.password);
	snprintf(file_challenge,sizeof(file_challenge),"%s",t.challenge);
	int flag = 0;
	if(utl::scomp(t.authtype,"cc0")) flag = 0;		// cc0
	if(utl::scomp(t.authtype,"cc1")) flag = 1;		// cc1
	if(utl::scomp(t.authtype,"cc2")) flag = 2;		// cc2
	if(utl::scomp(t.authtype,"cc3")) flag = 3;		// cc3
	
	report(LOG_INFO, "cc_checkresp: Looking for %s", username);
	tmp[0] = '\0';

	// Now read the file and find the user name
	do {
		user_offset = ftell(fp);
		s = fgets(file_line, MAX_LINE_LEN, fp);
		if(feof(fp)) break;
		if(*s == '#') continue;
		result = sscanf(file_line, "%s %s %s %s",
			tmp, key, display, file_challenge );
		if((result != 4) && (result !=3)) tmp[0] = '\0';
	} while(strcmp(username, tmp));
	if(strcmp(tmp, username)) {	// No key for user
		report(LOG_ERR, "cc_checkresp: Nothing in %s or missing key for %s",
			filename, username );
		fclose(fp);
		return (-1);
	}

	bcopy(key, tmp_key, 48);
	for(i = 0; i < OCT_KEY_LEN; i++)
		sscanf(&tmp_key[2*i], "%2x", &key[i]);
	if(!crypt_key(tmp, key, DES_DECRYPT)) return(-1);
	// ensure the database information is terminated properly
	key[25] = '\0';
	challenge[8] = '\0';
	response[8] = '\0';
	memset(nxtChallenge, '\0', 9);
	report(LOG_INFO,
		"cc_checkresp: Checking %s: key=%s, display=%d,\
 challenge=%s (resp received=%s)",
 			username, key, flag, challenge, response);
	temp = check_response(key,challenge,response,flag,(result==4) ? nxtChallenge : NULL);
	// store the next challenge in the cryptocard file
	if(result == 4) {
		if(fseek(fp, user_offset, SEEK_SET)) {
			report(LOG_ERR, "Can't reseek %s", filename );
			fclose(fp);
			return (-1);
		}
		i = strlen(file_line) - 6;
		for(s = &file_line[i] ; *s > ' ' ; s--);
		strcpy( ++s, nxtChallenge );
		if(fputs(file_line, fp) == EOF) {
			report(LOG_ERR, "Can't rewrite %s", filename );
			fclose(fp);
			return (-1);
		}
	}
	if(!temp) {
		report(LOG_ERR,
			"cc_checkresp: Incorrect response for %s (resp recd: %s)",
				username, response );
		fclose(fp);
		return (-1);
	}
	// close the file and return
	fclose(fp);
	return 0;
}

// updates next challenge field in config file when token is
// specified in cleartext mode
void
CryptoCard::store_nextchallenge(char *name, char *nextchallenge) {
	report(LOG_INFO, "updating config");
	if(!session.cfgfile) {
		report(LOG_ERR, "no config file specified");
		tac_exit(1);
	}
//	if(cfg_update_config(name, nextchallenge)) {
//		report(LOG_ERR, "cannot update file");
//		tac_exit(1);
//	}
	report(LOG_INFO, "updated");
}


// encrypts (crypt_flag = 1) or decrypts (crypt_flag = 0) user's key
// returns 1 if everything is O.K., otherwise 0 is returned
int
CryptoCard::crypt_key(char *username, char *key, int crypt_flag) {
	FILE *fp;
	char md5input[MAX_LINE_LEN];
	char md5output[MD5_LEN];
	char *tmp;
	char file_line[MAX_LINE_LEN], secret[MAX_LINE_LEN];
	int line_count = 0;
	int i;

	fp = fopen(MASTERKEY_FILE, "r");
	if(fp == NULL) {
		report(LOG_INFO, "Error: cannot open %s\n", MASTERKEY_FILE);
		return 0;
	}
	secret[0] = '\0';
	do {
		tmp = fgets(file_line, MAX_LINE_LEN, fp);
		if(!tmp) break;
		line_count++;
		if(*tmp == '#') continue;
		if(sscanf(file_line, "%s\n", secret) != 1) {
			printf("Warning: incorrect entry format at line %d in file %s\n", line_count, MASTERKEY_FILE);
			secret[0] = '\0';
			continue;
		}
	} while(!secret[0]);
	if(!secret) {
		printf("Masterkey is not found in file %s\n", MASTERKEY_FILE);
		return 0;
	}
	bcopy(secret, md5input, OCT_KEY_LEN);
	strncat(md5input, username, strlen(username));
	md5_calc(md5output, md5input, strlen(md5input));
	if(!crypt_flag) {
		tmp = (char *) malloc(sizeof(char) * MD5_LEN);	
		bcopy(key, tmp, MD5_LEN);
	}
	for(i = 0; i < MD5_LEN; i++) key[i] ^= md5output[i];
	bcopy(secret, md5input, OCT_KEY_LEN);
	md5input[strlen(secret)+1] = '\0';
	strncat(md5input, crypt_flag ? key : tmp, MD5_LEN);
	md5_calc(md5output, md5input, strlen(md5input));
	for(i = 0; i < 8; i++) key[MD5_LEN + i] ^= md5output[i];
	fclose(fp);
	return 1;
}

// internal state variables
#define STATE_AUTHEN_START   0	// no requests issued
#define STATE_AUTHEN_GETUSER 1	// username has been requested
#define STATE_AUTHEN_GETPASS 2	// password has been requested

struct private_data {
	char password[MAX_PASSWD_LEN + 1];
	int state;
};

char *challenge;

// Use cryptocard to verify a supplied password using state set up earlier
// when the username was supplied
static int
cryptocard_verify(char *passwd,struct authen_data * data) {
	struct private_data *p = data->method_data;
	int ret;
	char *nextchallenge;
	char *tmp, *str;

	nextchallenge = tac_malloc(10);
	data->status = TAC_PLUS_AUTHEN_STATUS_FAIL;
	switch(data->service) {
	 case TAC_PLUS_AUTHEN_SVC_LOGIN:
		tmp = cfg_get_pvalue(data->NAS_id->username, TAC_IS_USER, S_login,
			TAC_PLUS_RECURSE);
		break;
	 case TAC_PLUS_AUTHEN_SVC_ENABLE:
		tmp = cfg_get_pvalue(data->NAS_id->username, TAC_IS_USER, S_enable,
			TAC_PLUS_RECURSE);
		break;
	 default:
		tmp = cfg_get_pvalue(data->NAS_id->username, TAC_IS_USER, S_login,
			TAC_PLUS_RECURSE);
		break;
	}
	str = tac_find_substring("cryptocard ", tmp);
	ret = check_response(str, challenge, passwd, '3', nextchallenge);
	switch(!ret) {
	 case 0:
		data->status = TAC_PLUS_AUTHEN_STATUS_PASS;
		return 0;
	 default:
		return 0;
	}
}

/*
 * CryptoCard login authentication function. Wants a username
 * and a password, and tries to verify them via CryptoCard.
 *
 * Choose_authen will ensure that we already have a username before this
 * gets called.
 *
 * We will query for a password and keep it in the method_data.
 *
 * Any strings returned via pointers in authen_data must come from the
 * heap. They will get freed by the caller.
 *
 * Return 0 if data->status is valid, otherwise 1
 */
int
CryptoCard::cryptocard_fn(struct authen_data *data) {
	char *name, *passwd, *nextchallenge;
	struct private_data *p;
	char *prompt;
	char buf[256];
	int ret;

	p = (struct private_data *) data->method_data;
	if(!challenge) challenge = tac_malloc(10);
	// An abort has been received. Clean up and return
	if(data->flags & TAC_PLUS_CONTINUE_FLAG_ABORT) {
		if(data->method_data) free(data->method_data);
		data->method_data = NULL;
		return 1;
	}
	// Initialise method_data if first time through
	if(!p) {
		p = (struct private_data *) tac_malloc(sizeof(struct private_data));
		bzero(p, sizeof(struct private_data));
		data->method_data = p;
		p->state = STATE_AUTHEN_START;
	}
	if(!(char)data->NAS_id->username[0]) {
		switch (p->state) {
		 case STATE_AUTHEN_GETUSER:
			// we have previously asked for a username but none came back.
			// This is a gross error
			data->status = TAC_PLUS_AUTHEN_STATUS_ERROR;
			TLOG(LOG_CC, "%s: No username supplied after GETUSER",
				session.peer);
			return 0;
		 case STATE_AUTHEN_START:
			// No username. Try requesting one
			data->status = TAC_PLUS_AUTHEN_STATUS_GETUSER;
			if (data->service == TAC_PLUS_AUTHEN_SVC_LOGIN)
				prompt = "\nUser Access Verification\n\nUsername: ";
			else prompt = "Username: ";
			data->server_msg = tac_strdup(prompt);
			p->state = STATE_AUTHEN_GETUSER;
			return 0;
		 default:
			// something awful has happened. Give up and die
			TLOG(LOG_CC, "%s: cryptocard_fn bad state %d",
				session.peer, p->state);
			return 1;
		}
	}
	// we now have a username if we needed one
	name = data->NAS_id->username;
	// Do we have a password?
	passwd = p->password;
	if(!passwd[0]) {
		// no password yet. Either we need to ask for one and expect to get
		// called again, or we asked but nothing came back, which is fatal
		switch(p->state) {
		 case STATE_AUTHEN_GETPASS:
			// We  already asked for a password. This should be the reply
			strncpy(passwd, data->client_msg, MAX_PASSWD_LEN);
			passwd[MAX_PASSWD_LEN + 1] = '\0';
			break;
		 default:
			// Request a password
			passwd = cfg_get_login_secret(name, TAC_PLUS_RECURSE);
			if(!passwd && !STREQ(passwd, "cryptocard")) {
				TLOG(LOG_CC,
					"Cannot find cryptocard password declaratio for %s",name);
				data->status = TAC_PLUS_AUTHEN_STATUS_ERROR;
				return 1;
			}
			snprintf(buf, 120, "Challenge: %s\r\nEnter Response: ",
				random_challenge(challenge));
			data->server_msg = tac_strdup(buf);
			data->status = TAC_PLUS_AUTHEN_STATUS_GETPASS;
#ifndef CRYPTOCARD_DO_ECHO
			data->flags = TAC_PLUS_AUTHEN_FLAG_NOECHO;
#endif
			p->state = STATE_AUTHEN_GETPASS;
			return 0;
		}
	}

	// We have a username and password. Try validating
	// Assume the worst
	data->status = TAC_PLUS_AUTHEN_STATUS_FAIL;

	switch(data->service) {
	 case TAC_PLUS_AUTHEN_SVC_LOGIN:
	 case TAC_PLUS_AUTHEN_SVC_ENABLE:
		if(passwd[0]) ret = cryptocard_verify(passwd, data);
		else ret = 0;
		if(ret) { // Uh oh, we've got a problem
			data->server_msg = tac_strdup("CryptoCard problem!");
			// Fail the authentication
			data->status = TAC_PLUS_AUTHEN_STATUS_FAIL;
			return 0;
		}
		if(debug)
			TLOG(LOG_CC, "%s query for '%s' %s from %s %s",
				data->service == TAC_PLUS_AUTHEN_SVC_LOGIN ? "login" :
					"enable", name && name[0] ? name : "unknown",
						data->NAS_id->NAS_port && data->NAS_id->NAS_port[0] ?
							data->NAS_id->NAS_port : "unknown", session.peer,
								(data->status == TAC_PLUS_AUTHEN_STATUS_PASS) ?
									"accepted" : "rejected");
		break;
	 default:
		data->status = TAC_PLUS_AUTHEN_STATUS_ERROR;
		TLOG(LOG_CC, "%s: Bogus service value %d from packet",
			session.peer, data->service);
		break;
	}
	if(data->method_data) free(data->method_data);
	data->method_data = NULL;

	switch (data->status) {
	 case TAC_PLUS_AUTHEN_STATUS_ERROR:
	 case TAC_PLUS_AUTHEN_STATUS_FAIL:
	 case TAC_PLUS_AUTHEN_STATUS_PASS:
		return 0;
	 default:
		TLOG(LOG_CC, "%s: cryptocard_fn couldn't set recognizable status %d",
			session.peer, data->status);
		data->status = TAC_PLUS_AUTHEN_STATUS_ERROR;
		return 1;
	}
}

///////////////////////////////////////////////////
