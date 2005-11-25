#include <stdlib.h>
#include <stdio.h>
#include "pop3.h"

#include "storage.h"

struct pop3_command_rv {
	int successful;
	char response_already_sent;
	char *extra_string;
	char *extended_error_code;
};

extern struct pop3_command_rv pop3_rv_misc_success, pop3_rv_misc_failure, pop3_rv_quiet_success, pop3_rv_quiet_failure, pop3_rv_invalid, pop3_rv_badargs;

struct popcommand {
	char *name;
	struct pop3_command_rv (*function)(int, char *[], enum pop3_state *, FILE *, FILE *);
	unsigned int min_argc;
	unsigned int max_argc;
	unsigned int valid_states;
	char **valid_after_failed;
	char **valid_after_successful;
};

// trans: stat list retr dele noop rset quit
// opt/trans: top uidl 
// opt/auth: user* pass* apop*

#define DEFINE_POP3(cmd) struct pop3_command_rv cmd(int, char *[], enum pop3_state *, FILE *, FILE *)

DEFINE_POP3(pop3_CAPA);

DEFINE_POP3(pop3_USERPASS);
DEFINE_POP3(pop3_APOP);
DEFINE_POP3(pop3_NOOP);
DEFINE_POP3(pop3_STAT);
DEFINE_POP3(pop3_LIST);
DEFINE_POP3(pop3_UIDL);
DEFINE_POP3(pop3_RETR);
DEFINE_POP3(pop3_TOP);
DEFINE_POP3(pop3_DELE);
DEFINE_POP3(pop3_RSET);
DEFINE_POP3(pop3_QUIT);

extern struct popcommand pop3_commands[];
