%option noyywrap
%option nounput

%{
#include <dlfcn.h>
#include <string.h>
#include <syslog.h>
#include "modules.h"
#include "conffile.h"

struct simpleConfig *current_tags=NULL;

char const *skip_spaces(char const *in_string)	{
	char const *stringp;
	for(stringp=in_string; stringp[0]==' ' || stringp[0]=='\t'; stringp++) ;
	return stringp;
}
%}

PATH [[:alnum:]\._/][[:alnum:]_\.\-/]*

%%

^"pidfile"[[:space:]]+{PATH}$ {
	char const *pidfile_path=skip_spaces(yytext+strlen("pidfile"));
	set_pidfile(pidfile_path);
}

^"auth_module"[[:space:]]+{PATH}$ {
	char const *module_path=skip_spaces(yytext+strlen("auth_module"));
	void *dlhandle=dlopen(module_path, RTLD_NOW|RTLD_LOCAL);
	if(dlhandle) {
		auth_hookup(dlhandle);
	} else {
		syslog(LOG_WARNING, "Couldn't find auth module '%s'", module_path);
	}
}
^"storage_module"[[:space:]]+{PATH}$ {
	char const *module_path=skip_spaces(yytext+strlen("storage_module"));
	void *dlhandle=dlopen(module_path, RTLD_NOW|RTLD_LOCAL);
	if(dlhandle) {
		storage_hookup(dlhandle);
	} else {
		syslog(LOG_WARNING, "Couldn't find storage module '%s'", module_path);
	}
}

[\r\n]+

^#.*

^"> "[[:alnum:]_]+[[:space:]]*.+ {
	struct simpleConfig *sc_cursor;
	char found_tag=0;
	size_t tag_length;
	char const *keyword_start=yytext+strlen("> ");
	for(sc_cursor=current_tags;sc_cursor && sc_cursor->tag; sc_cursor++) {
		tag_length=strlen(sc_cursor->tag);
		if((!strncmp(keyword_start, sc_cursor->tag, tag_length)) && ( keyword_start[tag_length]==' ' || keyword_start[tag_length]=='\t' ) ) {
			// Match
			found_tag=1;
			char const *argument_start=skip_spaces(keyword_start+tag_length);
			char rv=(sc_cursor->function)(argument_start);
			if(!rv) {
				syslog(LOG_WARNING, "Module config '%s' failed", sc_cursor->tag);
			}
			break;
		}
	}
	if(!found_tag) {
		syslog(LOG_WARNING, "Unknown config line '%s'", yytext);
	}
}

%%

char parse_conffile(char *filename) {
	yyin=fopen(filename, "r");
	if(yyin) {
		yylex();
		return 1;
	} else {
		return 0;
	}
}
