#include <stdlib.h>
#include <string.h>
#include "auth_functions.h"
char const *_default_auth_timestamp() {
	return NULL;
}

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
