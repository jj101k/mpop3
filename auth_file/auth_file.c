#include <stdlib.h>
#include "pop3.h"

#define MAX_PASSWORD_LINE_LEN 1024

#define PASSWORD_FILE "sample.password"

#ifdef USE_OPENSSL

#include <openssl/rand.h>
#include <sys/utsname.h>
#include <string.h>

char *timestamp=NULL;
#define RAND_BYTES 16

char const *_auth_timestamp() {
	if(timestamp) return timestamp;
	char buff[RAND_BYTES];
	struct utsname uname_out;
	if(!RAND_bytes(buff, RAND_BYTES)) return NULL;
	if(uname(&uname_out)!=0) return NULL;
	timestamp=malloc((RAND_BYTES*2) + 3 + strlen(uname_out.nodename)+1);
	timestamp[0]='<';
	hex_from_binary(timestamp+1, buff, RAND_BYTES);
	sprintf(timestamp+(RAND_BYTES*2)+1, "@%s>", uname_out.nodename);
	return timestamp;
}

#else

char const *_auth_timestamp() {
	return NULL;
}

#endif

char const *_auth_password(char const *username) {
	FILE *password_fp;
	char pwline[MAX_PASSWORD_LINE_LEN];
	char *password_part;

	password_fp=fopen(PASSWORD_FILE, "r");
	if(!password_fp) return NULL;
	while(fgets(pwline, MAX_PASSWORD_LINE_LEN, password_fp)) {
		super_chomp(pwline);
		password_part=pwline;
		strsep(&password_part, ":");
		if(!strcmp(username, pwline)) {
			fclose(password_fp);
			return password_part;
		}
	}
	fclose(password_fp);
	return NULL;
}
char const *_auth_attempt_login(char const *username, char const *password) {
	char const *real_password=_auth_password(username);
	if(real_password && !strcmp(password, real_password)) return username;
	return NULL;
}
char const *_auth_login(char const *username) {
	return username;
}
