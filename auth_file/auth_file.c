#include <stdlib.h>
#include <string.h>
#include "pop3.h"

#include "conffile.h"
struct simpleConfig *config_hookup() {return NULL;}

#define MAX_PASSWORD_LINE_LEN 1024

#define PASSWORD_FILE "sample.password"

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
