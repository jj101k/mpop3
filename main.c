#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "pop3.h"
#include "modules.h"
#include "parse_conffile.h"
#include "net.h"
#include "storage_functions.h"
#include "auth_functions.h"

#define HELP_STRING "usage: " PACKAGE_NAME " [-f <filename>] [-shD]"

int main(int argc, char *argv[]) {
	char stdin_mode=0;
	char detach_after_net=0;
	int option_char;
	char *config_filename=SYSCONFDIR "/" PACKAGE_NAME ".conf";

	while( ( option_char=getopt(argc, argv, "f:hsD") )!=-1) {
		switch(option_char) {
			case 'f':
				config_filename=optarg; break;
			case 's':
				stdin_mode=1; break;
			case 'h':
				printf("%s\n", HELP_STRING);
				return 0;
			case 'D':
				detach_after_net=1; break;
		}
	}
	openlog(PACKAGE_NAME, LOG_PERROR|LOG_PID, LOG_MAIL);
	if(!parse_conffile(config_filename)) {
		syslog(LOG_ERR, "Couldn't open config file '%s'", config_filename);
		return 1;
	}
	auth_hookup(NULL);
	storage_hookup(NULL);
	if(stdin_mode) {
		if((!_auth_need_root()) && (_storage_need_user()==wuAny)) drop_privs();
		handle_connection(stdin, stdout);
		return 0;
	} else {
		if(detach_after_net) {
			pid_t real_master_pid=fork();
			if(real_master_pid<0) {
				syslog(LOG_ERR, "Couldn't fork");
				return 2;
			}
			if(!real_master_pid) {
				// child
				return net_server_start((!_auth_need_root()) && (_storage_need_user()==wuAny), 1);
			} else {
				sleep(5); // Give enough time to see if the net start worked.
				int status;
				if(wait4(real_master_pid, &status, WNOHANG, NULL)!=0) {
					if(WIFEXITED(status)) {
						return WEXITSTATUS(status);
					} else return 0;
				}
				return 0;
			}
		} else {
			return net_server_start((!_auth_need_root()) && (_storage_need_user()==wuAny), 0);
		}
	}
};
