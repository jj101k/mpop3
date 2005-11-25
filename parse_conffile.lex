%option noyywrap
%option nounput

%{
#include <dlfcn.h>
#include <string.h>
#include <syslog.h>
#include "modules.h"
#include "conffile.h"

struct simpleConfig *current_tags=NULL;
%}

PATH [[:alnum:]\._/][[:alnum:]_\.\-/]*

%%

"auth_module "{PATH}$ {
	char *module_path=yytext+strlen("auth_module ");
	void *dlhandle=dlopen(module_path, RTLD_NOW|RTLD_LOCAL);
	if(dlhandle) {
		auth_hookup(dlhandle);
	} else {
		syslog(LOG_WARNING, "Couldn't find auth module '%s'", module_path);
	}
}
"storage_module "{PATH}$ {
	char *module_path=yytext+strlen("storage_module ");
	void *dlhandle=dlopen(module_path, RTLD_NOW|RTLD_LOCAL);
	if(dlhandle) {
		storage_hookup(dlhandle);
	} else {
		syslog(LOG_WARNING, "Couldn't find storage module '%s'", module_path);
	}
}

[\r\t\n]+

#.*

.+ {
	struct simpleConfig *sc_cursor;
	char found_tag=0;
	size_t tag_length;
	for(sc_cursor=current_tags;sc_cursor && sc_cursor->tag; sc_cursor++) {
		tag_length=strlen(sc_cursor->tag);
		if((!strncmp(yytext, sc_cursor->tag, tag_length)) && yytext[tag_length]==' ') {
			// Match
			found_tag=1;
			char rv=(sc_cursor->function)(yytext+tag_length+1);
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
