#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

#include "pop3.h"
#include "modules.h"
#include "parse_conffile.h"
#include "net.h"
#include "storage_functions.h"
#include "auth_functions.h"

#define HELP_STRING "usage: " PACKAGE_NAME " [-f <filename>] [-shD]"
pid_t real_master_pid=0;
int cwd_fd=-1;
char *pidfile="";
void set_pidfile(char *new_pidfile) {
	if(!pidfile[0]) {
		pidfile=malloc(strlen(new_pidfile)+1);
		// Do something when malloc fails FIXME
		strcpy(pidfile, new_pidfile);
	}
}

void remove_pidfile() {
	fchdir(cwd_fd);
	if(real_master_pid==0) {
		// This is the master
		if(pidfile[0]) {
			unlink(pidfile);
		}
	}
}
void just_exit_please(int signum) {
	exit(0);
}

int main(int argc, char *argv[]) {
	char stdin_mode=0;
	char detach_after_net=0;
	int option_char;
	char *config_filename=SYSCONFDIR "/" PACKAGE_NAME ".conf";

	cwd_fd = open(".", O_RDONLY, 0);
	if(cwd_fd<0) {
		syslog(LOG_ERR, "Can't find the CWD");
		exit( 1);
	}

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
		exit( 1);
	}
	if(pidfile[0]) {
		int pidfile_d=open(pidfile, O_WRONLY|O_CREAT|O_EXCL, 0666);
		if(pidfile_d<0) {
			syslog(LOG_ERR, "Couldn't create PID file '%s'", pidfile);
			exit( 1);
		}
		FILE *pidfile_p=fdopen(pidfile_d, "w");
		pid_t my_pid=getpid();
		fprintf(pidfile_p, "%u", my_pid);
		fclose(pidfile_p);
	}
	signal(SIGTERM, just_exit_please);
	signal(SIGINT, just_exit_please);
	signal(SIGPIPE, just_exit_please);
	atexit(remove_pidfile);
	auth_hookup(NULL);
	storage_hookup(NULL);
	if(stdin_mode) {
		if((!_auth_need_root()) && (_storage_need_user()==wuAny)) drop_privs();
		handle_connection(stdin, stdout);
		exit( 0);
	} else {
		if(detach_after_net) {
			real_master_pid=fork();
			if(real_master_pid<0) {
				syslog(LOG_ERR, "Couldn't fork");
				exit( 2);
			}
			if(!real_master_pid) {
				// child
				exit( net_server_start((!_auth_need_root()) && (_storage_need_user()==wuAny), 1));
			} else {
				sleep(5); // Give enough time to see if the net start worked.
				int status;
				if(wait4(real_master_pid, &status, WNOHANG, NULL)!=0) {
					if(WIFEXITED(status)) {
						exit( WEXITSTATUS(status));
					} else exit( 0);
				}
				exit( 0);
			}
		} else {
			exit( net_server_start((!_auth_need_root()) && (_storage_need_user()==wuAny), 0));
		}
	}
	exit(1);
	return 1;
};
