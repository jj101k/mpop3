/*

Handles logins using the local (UNIX) auth database. Will only work if you're root.
And even then not on all UNIXes in all situations (it doesn't work on Mac OS X for some users)

*/
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>
#include "pop3.h"

char const *_auth_timestamp() {return NULL;} // Can't support APOP

char const *_auth_attempt_login(char const *username, char const *password) {
	struct passwd *pwitem;
	pwitem=getpwnam(username);

	if(!pwitem) return NULL;	

	if(password &&
		pwitem->pw_passwd && 
		!strcmp(crypt(password, pwitem->pw_passwd), pwitem->pw_passwd)
	) return username;

	return NULL;
}
