#include <stdio.h>
#include <dlfcn.h>
char const *(*_auth_attempt_login)(char const *, char const *);
char const *(*_auth_timestamp)();
char const *(*_auth_password)(char const *);
char const *(*_auth_login)(char const *);
int (*_auth_login_delay_needed)(char const *);
int (*_auth_default_login_delay)();

#define LOAD_MISC_FUNC(foo,bar) {if(dlhandle) foo=dlsym(dlhandle, bar); else if(!foo) {foo=dlsym(RTLD_DEFAULT, "_default" bar); if(!foo) printf("%s\n", dlerror());}} 

int auth_hookup(void *dlhandle) {
	LOAD_MISC_FUNC(_auth_attempt_login, "_auth_attempt_login");
	LOAD_MISC_FUNC(_auth_timestamp, "_auth_timestamp");
	LOAD_MISC_FUNC(_auth_password, "_auth_password");
	LOAD_MISC_FUNC(_auth_login, "_auth_login");
	LOAD_MISC_FUNC(_auth_login_delay_needed, "_auth_login_delay_needed");
	LOAD_MISC_FUNC(_auth_default_login_delay, "_auth_default_login_delay");
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

int storage_hookup(void *dlhandle) {
	LOAD_MISC_FUNC(_storage_array_style,"_storage_array_style");
	LOAD_MISC_FUNC(_storage_lock_mailbox,"_storage_lock_mailbox");
	LOAD_MISC_FUNC(_storage_message_count,"_storage_message_count");
	LOAD_MISC_FUNC(_storage_message_sum,"_storage_message_sum");
	LOAD_MISC_FUNC(_storage_first_message,"_storage_first_message");
	LOAD_MISC_FUNC(_storage_message_number,"_storage_message_number");
	LOAD_MISC_FUNC(_storage_uidl_supported,"_storage_uidl_supported");
	LOAD_MISC_FUNC(_storage_dump_message,"_storage_dump_message");
	LOAD_MISC_FUNC(_storage_dump_message_lines,"_storage_dump_message_lines");
	LOAD_MISC_FUNC(_storage_dump_headers,"_storage_dump_headers");
	LOAD_MISC_FUNC(_storage_synch,"_storage_synch");
	LOAD_MISC_FUNC(_storage_dump_fragment,"_storage_dump_fragment");
	return 1;
}
