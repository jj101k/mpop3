#include <string.h>
#include "pop_commands.h"
#include "strings.h"
#include "pop3.h"
#include "auth_functions.h"

#define ABSOLUTE_MAX_USERNAME_LENGTH 1024


#ifdef USE_OPENSSL
#include <openssl/evp.h>
#endif

struct pop3_command_rv
	pop3_rv_misc_success={1, 0, NULL, NULL}, pop3_rv_misc_failure={0, 0, NULL, NULL},
	pop3_rv_quiet_success={1, 1, NULL, NULL}, pop3_rv_quiet_failure={0, 1, NULL, NULL},
	pop3_rv_invalid={0, 0, E_INVALID_COMMAND, NULL}, pop3_rv_badargs={0, 0, E_BAD_ARGUMENTS},

	pop3_rv_login_success={1, 0, S_LOGIN_SUCCESS, NULL}, pop3_rv_user_success={1, 0, S_USER_SUCCESS, NULL},

	pop3_rv_bad_message={0, 0, E_BADMESSAGE, NULL}, pop3_rv_edelay={0, 0, E_DELAY, P3EXT_LOGIN_DELAY},
	pop3_rv_elocked={0, 0, E_LOCKED, P3EXT_IN_USE}, pop3_rv_ebadlogin={0, 0, E_BADLOGIN, NULL},
	pop3_rv_internal_error={0, 0, E_INTERNAL_ERROR}, pop3_rv_not_implemented={0, 0, E_NOT_IMPLEMENTED};

char *user_and_pass[]={"USER", "PASS", NULL};
char *user_only[]={"USER", NULL};

struct popcommand pop3_commands[]={
	{"CAPA",pop3_CAPA, 0,0, BIT(p3Transaction)|BIT(p3Authorisation), NULL, NULL},
	{"USER",pop3_USERPASS, 1,1, BIT(p3Authorisation), user_and_pass, NULL},
	{"PASS",pop3_USERPASS, 1,1, BIT(p3Authorisation), NULL, user_only},
	{"APOP",pop3_APOP, 2,2, BIT(p3Authorisation), user_and_pass, NULL},

	{"NOOP",pop3_NOOP, 0,0, BIT(p3Transaction), NULL, NULL},
	{"STAT",pop3_STAT, 0,0, BIT(p3Transaction), NULL, NULL},
	{"LIST",pop3_LIST, 0,1, BIT(p3Transaction), NULL, NULL},
	{"UIDL",pop3_UIDL, 0,1, BIT(p3Transaction), NULL, NULL},
	{"RETR",pop3_RETR, 1,1, BIT(p3Transaction), NULL, NULL},
	{"TOP",pop3_TOP, 2,2, BIT(p3Transaction), NULL, NULL},
	{"DELE",pop3_DELE, 1,1, BIT(p3Transaction), NULL, NULL},
	{"RSET",pop3_RSET, 0,0, BIT(p3Transaction), NULL, NULL},

	{"QUIT",pop3_QUIT, 0,0, BIT(p3Transaction)|BIT(p3Authorisation), NULL, NULL},
	{NULL,NULL,0,0}
};

struct pop3_message * valid_message(unsigned long int index) {
	struct pop3_message *message;
	if(index<1 || index>_storage_message_count()) {
		return NULL;
	}
	message=_storage_message_number(index);
	if((!message) || message->deleted) return NULL;
	return message;
}

struct pop3_command_rv pop3_rv_message_deleted={1, 0, S_MESSAGE_DELETED, NULL};

struct pop3_command_rv pop3_DELE(int argc, char *argv[], enum pop3_state *unused2, FILE *ifp, FILE *ofp) {
	struct pop3_message *current_message;
	unsigned long int index;
	
	index=strtoull(argv[1], NULL, 10);
	current_message=valid_message(index);
	if(!current_message) {
		return pop3_rv_bad_message;
	}

	current_message->deleted=1;
	return pop3_rv_message_deleted;
}

struct pop3_command_rv pop3_RETR(int argc, char *argv[], enum pop3_state *unused2, FILE *ifp, FILE *ofp) {
	struct pop3_message *current_message;
	unsigned long int index;
	
	index=strtoull(argv[1], NULL, 10);
	current_message=valid_message(index);
	if(!current_message) {
		return pop3_rv_bad_message;
	}

	_send_OK(ofp, S_MESSAGE_FOLLOWS);
	_storage_dump_message(current_message, ofp);
	_pop3_fprintf(ofp, ".\r\n");
	
	return pop3_rv_quiet_success;
}

struct pop3_command_rv pop3_TOP(int argc, char *argv[], enum pop3_state *unused2, FILE *ifp, FILE *ofp) {
	struct pop3_message *current_message;
	unsigned long int index;
	unsigned long int max_lines;
	
	index=strtoull(argv[1], NULL, 10);
	current_message=valid_message(index);
	if(!current_message) {
		return pop3_rv_bad_message;
	}

	max_lines=strtoull(argv[2], NULL, 10);

	_send_OK(ofp, S_MESSAGE_FOLLOWS);
	_storage_dump_headers(current_message, ofp);
	_storage_dump_message_lines(current_message, max_lines, ofp);
	_pop3_fprintf(ofp, ".\r\n");
	
	return pop3_rv_quiet_success;
}

struct pop3_command_rv pop3_CAPA(int argc, char *argv[], enum pop3_state *current_state, FILE *ifp, FILE *ofp) {
	_send_OK(ofp, S_CAPA_RESPONSE);
	if(*current_state==p3Transaction) {
	}
	_pop3_fprintf(ofp, "TOP\r\n");
	_pop3_fprintf(ofp, "USER\r\n");
	_pop3_fprintf(ofp, "RESP-CODES\r\n");
	_pop3_fprintf(ofp, "PIPELINING\r\n");
	if(_storage_uidl_supported()) _pop3_fprintf(ofp, "UIDL\r\n");
	int login_delay=_auth_default_login_delay();
	if(login_delay) _pop3_fprintf(ofp, "LOGIN-DELAY %i\r\n", login_delay);
	_pop3_fprintf(ofp, "IMPLEMENTATION %s\r\n", S_SERVER_IMPLEMENTATION);
	//_pop3_fprintf(ofp, "SASL\r\n"); FIXME
	//_pop3_fprintf(ofp, "EXPIRE NEVER\r\n"); FIXME
	_pop3_fprintf(ofp, ".\r\n");
	return pop3_rv_quiet_success;
}

struct pop3_command_rv pop3_USERPASS(int argc, char *argv[], enum pop3_state *current_state, FILE *ifp, FILE *ofp) {
	static char username[ABSOLUTE_MAX_USERNAME_LENGTH];
	if(!strcasecmp(argv[0],"PASS")) {
		if(!username[0]) {
			return pop3_rv_ebadlogin;
		}
		char const *mailbox=_auth_attempt_login(username, argv[1]);
		if(mailbox) {
			if(_auth_login_delay_needed(username)) {
				return pop3_rv_edelay; // delayed
			} else if(_storage_lock_mailbox(mailbox)) {
				*current_state=p3Transaction;
				return pop3_rv_login_success;
			} else {
				return pop3_rv_elocked; // locked
			}
		} else {
			return pop3_rv_ebadlogin; // invalid
		}
	} else {
		if(strlen(argv[1])>=ABSOLUTE_MAX_USERNAME_LENGTH) return pop3_rv_ebadlogin;
		strncpy(username, argv[1], strlen(argv[1])+1);
		return pop3_rv_user_success;
	}
}

struct pop3_command_rv pop3_APOP(int argc, char *argv[], enum pop3_state *current_state, FILE *ifp, FILE *ofp) {
#ifdef USE_OPENSSL
	char const *timestamp=_auth_timestamp();
	if(!timestamp) return pop3_rv_not_implemented;
	char const *username=argv[1];
	char const *password=_auth_password(username);
	EVP_MD_CTX *ssl_context;
	char digest_raw[EVP_MAX_MD_SIZE];
	char digest_hex[(2*EVP_MAX_MD_SIZE)+1];

	if(!password) return pop3_rv_ebadlogin;

	ssl_context=EVP_MD_CTX_create();
	if(!EVP_DigestInit(ssl_context, EVP_md5())) return pop3_rv_internal_error;
	if(!EVP_DigestUpdate(ssl_context, timestamp, strlen(timestamp))) {EVP_MD_CTX_cleanup(ssl_context); return pop3_rv_internal_error;}
	if(!EVP_DigestUpdate(ssl_context, password, strlen(password))) {EVP_MD_CTX_cleanup(ssl_context); return pop3_rv_internal_error;}
	if(!EVP_DigestFinal(ssl_context, digest_raw, NULL)) {EVP_MD_CTX_cleanup(ssl_context); return pop3_rv_internal_error;}
	hex_from_binary(digest_hex, digest_raw, EVP_MD_size(EVP_md5()));
	if(!strcasecmp(argv[2],digest_hex)) {
		char const *mailbox=_auth_login(username);
		if(!mailbox) {
			return pop3_rv_internal_error;
		} else if(_auth_login_delay_needed(username)) {
			return pop3_rv_edelay; // delayed
		} else if(_storage_lock_mailbox(mailbox)) {
			*current_state=p3Transaction;
			return pop3_rv_login_success;
		} else {
			return pop3_rv_elocked; // locked
		}
	} else {
		return pop3_rv_ebadlogin;
	}
#else
	return pop3_rv_not_implemented;
#endif	
}

struct pop3_command_rv pop3_rv_noop={1, 0, S_NOOP, NULL};

struct pop3_command_rv pop3_NOOP(int argc, char *argv[], enum pop3_state *unused2, FILE *ifp, FILE *ofp) {
	return pop3_rv_noop;
}
struct pop3_command_rv pop3_STAT(int argc, char *argv[], enum pop3_state *unused2, FILE *ifp, FILE *ofp) {
	unsigned long int message_count=0;
	unsigned long int message_sum=0;
	struct pop3_message *current_message;
	unsigned long int i;

	if(_storage_array_style()==asArray) {
		message_count=_storage_message_count();
		for(i=0, current_message=_storage_first_message();i<message_count;i++, current_message++) {
			if(!current_message->deleted) {
				message_sum+=current_message->size;
			}
		}
	} else {
		for(current_message=_storage_first_message();current_message;current_message=current_message->next) {
			if(!current_message->deleted) {
				message_count++;
				message_sum+=current_message->size;
			}
		}
	}

	_pop3_fprintf(ofp, POP3_SUCCESS " %lu %lu\r\n", message_count, message_sum);
	return pop3_rv_quiet_success;;
}

struct pop3_command_rv pop3_LIST(int argc, char *argv[], enum pop3_state *unused2, FILE *ifp, FILE *ofp) {
	struct pop3_message *current_message=_storage_first_message();
	long unsigned int i;
	
	if(argc==1) {
		_send_OK(ofp, S_LIST_FOLLOWS);
		if(_storage_array_style()==asArray) {
			unsigned long int message_count=_storage_message_count();
			for(i=1;i<=message_count;i++, current_message++) {
				if(!current_message->deleted) _pop3_fprintf(ofp, "%lu %lu\r\n", i, current_message->size);
			}
		} else {
			for(i=1;current_message;i++, current_message=current_message->next) {
				if(!current_message->deleted) _pop3_fprintf(ofp, "%lu %lu\r\n", i, current_message->size);
			}
		}
		_pop3_fprintf(ofp, ".\r\n");

		return pop3_rv_quiet_success;
	} else {
		i=strtoull(argv[1], NULL, 10);
		current_message=valid_message(i);
		if(!current_message) {
			return pop3_rv_bad_message;
		} else {
			_pop3_fprintf(ofp, POP3_SUCCESS " %lu %lu\r\n", i, current_message->size);
			return pop3_rv_quiet_success;
		}
	}
}

struct pop3_command_rv pop3_rv_reset={1, 0, S_RESET, NULL};

struct pop3_command_rv pop3_RSET(int argc, char *argv[], enum pop3_state *unused2, FILE *ifp, FILE *ofp) {
	struct pop3_message *current_message=_storage_first_message();
	unsigned long int i;
	
	if(_storage_array_style()==asArray) {
		unsigned long int message_count=_storage_message_count();
		for(i=0;i<message_count;current_message++, i++) {
			current_message->deleted=0;
		}
	} else {
		for(;current_message;current_message=current_message->next) {
			current_message->deleted=0;
		}
	}

	return pop3_rv_reset;
}
struct pop3_command_rv pop3_UIDL(int argc, char *argv[], enum pop3_state *unused2, FILE *ifp, FILE *ofp) {
	struct pop3_message *current_message=_storage_first_message();
	long unsigned int i;

	if(!_storage_uidl_supported()) {
		return pop3_rv_not_implemented;
	}
	
	if(argc==1) {
		_send_OK(ofp, S_LIST_FOLLOWS);
		if(_storage_array_style()==asArray) {
			unsigned long int message_count=_storage_message_count();
			for(i=1;i<=message_count;i++, current_message++) {
				if(!current_message->deleted) _pop3_fprintf(ofp, "%lu %s\r\n", i, current_message->uidl);
			}
		} else {
			for(i=1;current_message;i++, current_message=current_message->next) {
				if(!current_message->deleted) _pop3_fprintf(ofp, "%lu %s\r\n", i, current_message->uidl);
			}
		}
		_pop3_fprintf(ofp, ".\r\n");

		return pop3_rv_quiet_success;
	} else {
		i=strtoull(argv[1], NULL, 10);
		current_message=valid_message(i);
		if(!current_message) {
			return pop3_rv_bad_message;
		} else {
			_pop3_fprintf(ofp, POP3_SUCCESS " %lu %s\r\n", i, current_message->uidl);
			return pop3_rv_quiet_success;
		}
	}

	return pop3_rv_quiet_success;
}

struct pop3_command_rv pop3_rv_quit={1, 0, S_QUIT, NULL};

struct pop3_command_rv pop3_QUIT(int argc, char *argv[], enum pop3_state *current_state, FILE *ifp, FILE *ofp) {
	*current_state=p3Update;
	return pop3_rv_quit;
}

