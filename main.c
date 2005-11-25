#include <stdio.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>

#include <arpa/inet.h>

#include "pop3.h"
#include "modules.h"
#include "parse_conffile.h"


#define PRIMARY_PROTOCOL "ip"
#define SECONDARY_PROTOCOL "ipv6"

#define DEFAULT_BACKLOG 20

enum process_status {psAlive, psDead};

struct child_process {
	struct child_process *next;
	pid_t pid;
	enum process_status status;
} dummy_process={NULL, 0, psAlive}, *last_process;

#define MAX_CHILDREN DEFAULT_BACKLOG

int process_count=0;

int dead_processes=0;

void update_child_list() {
	struct child_process *current_process, *previous_process;
	if(!dead_processes) return;
	int t_process_count=0;
	for(current_process=&dummy_process; current_process;current_process=current_process->next) {
		if(current_process->status == psAlive) {
			t_process_count++;
			previous_process=current_process;
		} else {
			previous_process->next=current_process->next;
			free(current_process);
			current_process=previous_process;
			dead_processes--;
		}
	}
	last_process=previous_process;
	process_count=t_process_count;
}

#include <signal.h>
#include <sys/wait.h>

void _handle_sigchild(int unused) {
	int status;
	pid_t dead_child=wait(&status);
	if(dead_child<=0) return;
	dead_processes++;
	struct child_process *current_process;
	for(current_process=&dummy_process; current_process;current_process=current_process->next) {
		if(current_process->pid == dead_child) {
			current_process->status=psDead;
			break;
		}
	}
}

int _bind_etc_ipng() {
	int accepted_fd;
	struct sockaddr_in6 sockaddr;
	int socket_fd=socket(AF_INET6, SOCK_STREAM, 0);
	pid_t child_pid;

	if(socket_fd==-1) return -1;
	sockaddr.sin6_len=sizeof(in6addr_any);
	sockaddr.sin6_family=AF_INET6;
	sockaddr.sin6_port=DEFAULT_PORT;
	sockaddr.sin6_flowinfo=0;
	sockaddr.sin6_addr=in6addr_any;
	sockaddr.sin6_scope_id=1;
	if(bind(socket_fd, (struct sockaddr *)&sockaddr, sizeof(sockaddr))==-1) return -1;
	if(listen(socket_fd, DEFAULT_BACKLOG)==-1) return -1;
	int sockaddr_len=sizeof(sockaddr);
	last_process=&dummy_process;
	process_count=0;
	signal(SIGCHLD, _handle_sigchild);
	while(1) {
		accepted_fd=accept(socket_fd, (struct sockaddr *)&sockaddr, &sockaddr_len);
		if(accepted_fd<0) continue;
		child_pid=fork();
		if(child_pid<0) return -1;
		else if(!child_pid) {
			FILE *accepted_fp=fdopen(accepted_fd, "r+");
			handle_connection(accepted_fp, accepted_fp);
			exit(0);
		} else {
			close(accepted_fd);
			struct child_process *new_cp=malloc(sizeof(struct child_process));
			new_cp->next=NULL;
			new_cp->pid=child_pid;
			new_cp->status=psAlive;
			update_child_list();
			last_process->next=new_cp;
			while(process_count>=MAX_CHILDREN) {sleep(1);update_child_list();}
		}
	}
	return socket_fd;
}

int _bind_etc_ip() {
	int accepted_fd;
	struct sockaddr_in sockaddr;
	int socket_fd=socket(AF_INET, SOCK_STREAM, 0);
	pid_t child_pid;

	if(socket_fd==-1) return -1;
	sockaddr.sin_family=IPPROTO_TCP;
	sockaddr.sin_port=DEFAULT_PORT;
	sockaddr.sin_addr.s_addr=INADDR_ANY;
	if(bind(socket_fd, (struct sockaddr *)&sockaddr, sizeof(sockaddr))==-1) return -1;
	if(listen(socket_fd, DEFAULT_BACKLOG)==-1) return -1;
	int sockaddr_len=sizeof(sockaddr);
	last_process=&dummy_process;
	process_count=0;
	signal(SIGCHLD, _handle_sigchild);
	while(1) {
		accepted_fd=accept(socket_fd, (struct sockaddr *)&sockaddr, &sockaddr_len);
		if(accepted_fd<0) continue;
		child_pid=fork();
		if(child_pid<0) return -1;
		else if(!child_pid) {
			FILE *accepted_fp=fdopen(accepted_fd, "r+");
			handle_connection(accepted_fp, accepted_fp);
			exit(0);
		} else {
			close(accepted_fd);
			struct child_process *new_cp=malloc(sizeof(struct child_process));
			new_cp->next=NULL;
			new_cp->pid=child_pid;
			new_cp->status=psAlive;
			update_child_list();
			last_process->next=new_cp;
			while(process_count>=MAX_CHILDREN) {sleep(1);update_child_list();}
		}
	}
	return socket_fd;
}

int _bind_etc(char *proto) {
	if(!strcmp(proto, "ip")) return _bind_etc_ip(); else return _bind_etc_ipng();
}

int tcp_server() {
	int socket;
	pid_t pid;

	if(SECONDARY_PROTOCOL) {
		pid=fork();
		if(pid==-1) return -2;
		else if(!pid) {
			// child

			socket=_bind_etc(SECONDARY_PROTOCOL);
			if(socket<0) exit(1);
			exit(0);
		}
	}
	socket=_bind_etc(PRIMARY_PROTOCOL);
	if(socket<0) {
		// kill child
		return -1;
	}
	return 0;
}

#include <dlfcn.h>
int main(int argc, char *argv[]) {
	parse_conffile("mpop3.conf");
	auth_hookup(NULL);
	storage_hookup(NULL);
	if(argc>1 && !strcmp(argv[1],"-s")) {
		handle_connection(stdin, stdout);
		return 0;
	} else {
		return tcp_server();
	}
};
