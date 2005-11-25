#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

#include "pop3.h"
#include "modules.h"
#include "parse_conffile.h"
#include "net.h"
#include "storage_functions.h"
#include "auth_functions.h"

#define HELP_STRING "usage: " PACKAGE_NAME " [-f <filename>] [-sh]"

int main(int argc, char *argv[]) {
	char stdin_mode=0;
	int option_char;
	char *config_filename=SYSCONFDIR "/" PACKAGE_NAME ".conf";

	while( ( option_char=getopt(argc, argv, "f:hs") )!=-1) {
		switch(option_char) {
			case 'f':
				config_filename=optarg; break;
			case 's':
				stdin_mode=1; break;
			case 'h':
				printf("%s\n", HELP_STRING);
				return 0;
		}
	}
	openlog(PACKAGE_NAME, LOG_PERROR|LOG_PID, LOG_MAIL);
	parse_conffile(config_filename);
	auth_hookup(NULL);
	storage_hookup(NULL);
	if(stdin_mode) {
		if((!_auth_need_root()) && (_storage_need_user()==wuAny)) drop_privs();
		handle_connection(stdin, stdout);
		return 0;
	} else {
		return net_server_start((!_auth_need_root()) && (_storage_need_user()==wuAny));
	}
};
