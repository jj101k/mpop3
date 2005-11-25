%option noyywrap
%option nounput

%{
#include <dlfcn.h>
#include <string.h>
#include <syslog.h>
#include "modules.h"
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
