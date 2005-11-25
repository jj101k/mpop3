#include <stdio.h>
#include <string.h>
#include "pop3.h"
#include "modules.h"
#include "parse_conffile.h"
#include "net.h"

int main(int argc, char *argv[]) {
	parse_conffile("mpop3.conf");
	auth_hookup(NULL);
	storage_hookup(NULL);
	if(argc>1 && !strcmp(argv[1],"-s")) {
		handle_connection(stdin, stdout);
		return 0;
	} else {
		return net_server_start();
	}
};
