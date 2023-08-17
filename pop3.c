#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include "auth_functions.h"
#include "pop_commands.h"
#include "strings.h"
#include "pop3.h"

// This is a vaguely sensible max
// more practically, this/2 would be okay.
#define MAX_POP3_ARG_COUNT RFC_MAX_INPUT_LENGTH

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <pwd.h>
char drop_privs_to_user(char const *username) {
	if(!getuid()) {
		struct passwd *pswd=getpwnam(username);
		if(!pswd) return 0;
		if(!pswd->pw_uid) return 0; // Don't SU to root
		if(!pswd->pw_gid) return 0; // ...or root's buddies
		if(setgid(pswd->pw_gid)!=0) return 0;
		if(setuid(pswd->pw_uid)!=0) return 0;
		return 1;
	} else {
		// If we're already unprivileged, return success.
		return 1;
	}
}

#include <syslog.h>
void drop_privs() {
	if(!drop_privs_to_user(SAFE_USER)) {
		syslog(LOG_ERR, "Failed to drop privs, aborting");
		exit(1);
	}
}


#include <stdarg.h>
int _pop3_fprintf(FILE *out, char const *format, ...) {
	va_list ap;
	int rv;
	char *buff=malloc(RFC_MAX_OUTPUT_LENGTH+1);
	va_start(ap, format);
	rv=vsnprintf(buff, RFC_MAX_OUTPUT_LENGTH+1, format, ap);
	va_end(ap);
	if(strlen(buff)<2) {
		strcat(buff, "\r\n");
	} else if(strcmp(buff+strlen(buff)-2, "\r\n")!=0) {
		strcpy(buff+strlen(buff)-2, "\r\n");
	}
	fwrite(buff, strlen(buff), 1, out);
	return rv;
}

int _send_misc(FILE *ofp, char *prefix, char *message, char *extended_error) {
	if(message) {
		if(extended_error) {
			return _pop3_fprintf(ofp, "%s [%s] %s\r\n", prefix, extended_error, message);
		} else {
			return _pop3_fprintf(ofp, "%s %s\r\n", prefix, message);
		}
	} else {
		return _pop3_fprintf(ofp, "%s\r\n", prefix);
	}
}

int grep_equal(char *input, char *compare[]) {
	int i;
	for(i=0; compare[i]; i++) {
		if(!strcasecmp(input, compare[i])) {
			return 1;
		}
	}
	return 0;
}

FILE *sigalarm_out;

void handle_sigalarm(int signum) {
	_send_ERR(sigalarm_out, E_TIMEOUT, NULL);
	exit(0);
}

enum pop3_state command_loop(FILE *ifp, FILE *ofp, enum pop3_state current_state) {
	char command_line[RFC_MAX_INPUT_LENGTH];
	static char previous_command_line[RFC_MAX_INPUT_LENGTH]="";
	char *argv[MAX_POP3_ARG_COUNT];
	unsigned int argc=0;
	char *cursor;
	int i;
	struct pop3_command_rv pop3_command_rv;
	static struct pop3_command_rv previous_pop3_command_rv={1,0,NULL};

	{
		sigalarm_out=ofp;
		sig_t old_sigalarm_handler=signal(SIGALRM, handle_sigalarm);
		alarm(DEFAULT_TIMEOUT);

		if(!fgets(command_line, RFC_MAX_INPUT_LENGTH, ifp)) return p3Dead;

		alarm(0);
		signal(SIGALRM, old_sigalarm_handler);
	}

	cursor=command_line;
	super_chomp(command_line);

	while(cursor) {
		argv[argc++]=cursor;
		strsep(&cursor," ");
		if(strlen(argv[argc-1]) > RFC_MAX_ARG_LENGTH || strlen(argv[argc-1]) < RFC_MIN_ARG_LENGTH ) {
			_send_ERR(ofp, E_PROTOCOL_ERROR, NULL);
			return current_state;
		}
	}
	pop3_command_rv=pop3_rv_invalid;
	char bad_command_args=0;
	for(i=0;pop3_commands[i].name;i++) {
		if(! ( pop3_commands[i].valid_states & BIT(current_state) ) ) continue;
		if(!strcasecmp(pop3_commands[i].name, argv[0])) {
			int do_command=0;

			if(argc-1 < pop3_commands[i].min_argc || argc-1 > pop3_commands[i].max_argc) {
				bad_command_args=1;
			} else if(pop3_commands[i].valid_after_failed) {
				if(!previous_command_line[0]) {
					// empty line??
					do_command=1;
				} else if(previous_pop3_command_rv.successful) {
					do_command=0;
				} else if(grep_equal(previous_command_line, pop3_commands[i].valid_after_failed)) {
					do_command=1;
				} else {
					do_command=0;
				}
			} else if(pop3_commands[i].valid_after_successful) {
				if(!previous_pop3_command_rv.successful) {
					do_command=0;
				} else if(grep_equal(previous_command_line, pop3_commands[i].valid_after_successful)) {
					do_command=1;
				} else {
					do_command=0;
				}
			} else {
				do_command=1;
			}
			if(do_command) {
				pop3_command_rv=(*(pop3_commands[i].function))(argc, argv, &current_state, ifp, ofp);
			} else if(bad_command_args) {
				pop3_command_rv=pop3_rv_badargs;
			} else {
				pop3_command_rv=pop3_rv_invalid;
			}
			break;
		}
	}
	strncpy(previous_command_line, command_line, sizeof(previous_command_line));
	previous_pop3_command_rv=pop3_command_rv;

	if(!pop3_command_rv.response_already_sent) {
		if(pop3_command_rv.successful) {
			_send_OK(ofp, pop3_command_rv.extra_string);
		} else {
			_send_ERR(ofp, pop3_command_rv.extra_string, pop3_command_rv.extended_error_code);
		}
	}
	return current_state;
}

int handle_connection(FILE *ifp, FILE *ofp) {
	enum pop3_state current_state=p3Authorisation;
#if USE_OPENSSL
	char const *ts=_auth_timestamp();
	if(ts) {
		_pop3_fprintf(ofp, POP3_SUCCESS " %s %s\r\n", S_SERVER_ID, ts);
	} else {
		_pop3_fprintf(ofp, POP3_SUCCESS " %s\r\n", S_SERVER_ID);
	}
#else
	_pop3_fprintf(ofp, POP3_SUCCESS " %s\r\n", S_SERVER_ID);
#endif
	while(1) {
		current_state=command_loop(ifp, ofp, current_state);
		if(current_state == p3Dead) {
			break;
		}
		if(current_state == p3Update) {
			_storage_synch();
			break;
		}
	}
	return 1;
}
