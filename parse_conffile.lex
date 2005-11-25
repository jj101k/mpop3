%option noyywrap

%{
#include <dlfcn.h>
#include <string.h>
#include "modules.h"
%}

PATH [[:alnum:]\._/][[:alnum:]_\.\-/]*

%%

"auth_module "{PATH}$ {
	char *module_path=yytext+strlen("auth_module ");
	void *dlhandle=dlopen(module_path, RTLD_NOW|RTLD_LOCAL);
	auth_hookup(dlhandle);
}
"storage_module "{PATH}$ {
	char *module_path=yytext+strlen("auth_module ");
	void *dlhandle=dlopen(module_path, RTLD_NOW|RTLD_LOCAL);
	storage_hookup(dlhandle);
}

[\r\t\n]+

#.*

%%

void parse_conffile(char *filename) {
	yyin=fopen(filename, "r");
	if(yyin) yylex();
}
