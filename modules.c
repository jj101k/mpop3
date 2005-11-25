#include <stdio.h>
#include <dlfcn.h>
#include "auth_default.h"
#include "storage_default.h"
#include "conffile.h"

char const *(*_auth_attempt_login)(char const *, char const *);
char const *(*_auth_timestamp)();
char const *(*_auth_password)(char const *);
char const *(*_auth_login)(char const *);
int (*_auth_login_delay_needed)(char const *);
int (*_auth_default_login_delay)();
char (*_auth_need_root)() ;

#define LOAD_MISC_FUNC(fname) {if(dlhandle) fname=dlsym(dlhandle, #fname); else if(!fname) {fname=_default##fname;}}

int auth_hookup(void *dlhandle) {
	LOAD_MISC_FUNC(_auth_attempt_login);
	LOAD_MISC_FUNC(_auth_timestamp);
	LOAD_MISC_FUNC(_auth_password);
	LOAD_MISC_FUNC(_auth_login);
	LOAD_MISC_FUNC(_auth_login_delay_needed);
	LOAD_MISC_FUNC(_auth_default_login_delay);
	LOAD_MISC_FUNC(_auth_need_root);
	struct simpleConfig *(*config_hookup)() = dlsym(dlhandle, "config_hookup");
	if(config_hookup) {
		current_tags=config_hookup();
	} else {
		current_tags=NULL;
	}
	return 1;
}

enum arrayStyle (*_storage_array_style)();
int (*_storage_lock_mailbox)(char const *mailbox);
unsigned long int (*_storage_message_count)();
unsigned long int (*_storage_message_sum)();
struct pop3_message *(*_storage_first_message)();
struct pop3_message *(*_storage_message_number)(unsigned long int);
int (*_storage_uidl_supported)();
int (*_storage_dump_message)(struct pop3_message *, FILE *);
int (*_storage_dump_message_lines)(struct pop3_message *, unsigned long int, FILE *);
int (*_storage_dump_headers)(struct pop3_message *, FILE *);
int (*_storage_synch)();
int (*_storage_dump_fragment)(unsigned long int, unsigned long int, unsigned long int, unsigned long int, FILE *);
enum whichUser (*_storage_need_user)() ;

int storage_hookup(void *dlhandle) {
	LOAD_MISC_FUNC(_storage_array_style);
	LOAD_MISC_FUNC(_storage_lock_mailbox);
	LOAD_MISC_FUNC(_storage_message_count);
	LOAD_MISC_FUNC(_storage_message_sum);
	LOAD_MISC_FUNC(_storage_first_message);
	LOAD_MISC_FUNC(_storage_message_number);
	LOAD_MISC_FUNC(_storage_uidl_supported);
	LOAD_MISC_FUNC(_storage_dump_message);
	LOAD_MISC_FUNC(_storage_dump_message_lines);
	LOAD_MISC_FUNC(_storage_dump_headers);
	LOAD_MISC_FUNC(_storage_synch);
	LOAD_MISC_FUNC(_storage_dump_fragment);
	LOAD_MISC_FUNC(_storage_need_user);
	struct simpleConfig *(*config_hookup)() = dlsym(dlhandle, "config_hookup");
	if(config_hookup) {
		current_tags=config_hookup();
	} else {
		current_tags=NULL;
	}
	return 1;
}
