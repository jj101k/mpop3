#include <stdlib.h>
#include <string.h>
#include "pop3.h"
#include "auth_functions.h"

char _default_auth_need_root() {return 0;}

#ifdef USE_OPENSSL

#include <openssl/rand.h>
#include <sys/utsname.h>

char *timestamp=NULL;
#define RAND_BYTES 16

char const *_default_auth_timestamp() {
	if(timestamp) return timestamp;
	unsigned char buff[RAND_BYTES];
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

char const *_default_auth_timestamp() {
	return NULL;
}

#endif

char const *_default_auth_password(char const *username) {
	return NULL;
}
char const *_default_auth_attempt_login(char const *username, char const *password) {
	char const *compare_password=_auth_password(username);
	if(password && compare_password && !strcmp(compare_password, password)) return _auth_login(username);
	return NULL;
}
char const *_default_auth_login(char const *username) {
	return username;
}
int _default_auth_login_delay_needed(char const *username) {return 0;}
int _default_auth_default_login_delay() {return 0;};
