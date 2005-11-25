#ifndef __net_h
#define __net_h
#include <stdbool.h>

enum process_status {psAlive, psDead};

struct child_process {
	struct child_process *next;
	pid_t pid;
	enum process_status status;
};

#define MAX_CHILDREN SOMAXCONN
#define MAX_SOCKETS 10

int net_server_start(bool);
#endif
