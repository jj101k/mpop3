#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "pop3.h"
#include "net.h"


/*

If rendezvous (aka zeroconf) support exists:

bind to a wild port if 110 isn't possible
register in multicast DNS "<hostname>._pop3._tcp.local IN SRV <self address>:<port>"

*/

struct child_process dummy_process={NULL, 0, psAlive}, *last_process;

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

int accept_loop(int l_socket, int sockaddr_len) {
	int accepted_fd;
	pid_t child_pid;
	struct sockaddr *socket_info=malloc(sockaddr_len);
	int socket_info_len;

	last_process=&dummy_process;
	process_count=0;
	signal(SIGCHLD, _handle_sigchild);
	while(1) {
		socket_info_len=sockaddr_len;
		accepted_fd=accept(l_socket, socket_info, &socket_info_len);
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
	return 1;
}

#if USE_RENDEZVOUS
#include <dns_sd.h>
#endif

#include <errno.h>

#include <syslog.h>
#include <stdarg.h>

int net_server_start(bool su_after_bind) {
	struct addrinfo addrhints, *first_addrinfo_result, *ai_res_p;
	int rv;
	int sockets_in_use=0;
	int current_socket;
	pid_t nhpidlist[MAX_SOCKETS];

	memset(&addrhints, 0, sizeof(addrhints));
	addrhints.ai_family=PF_UNSPEC;
	addrhints.ai_socktype=SOCK_STREAM; // We only want stream-capable services
	//addrhints.ai_flags=AI_PASSIVE;
	rv=getaddrinfo(NULL, "pop3", &addrhints, &first_addrinfo_result);
	if(rv!=0) {
		// gai_strerror(error);
		return 0;
	}
#if USE_RENDEZVOUS
	DNSServiceRef sdRef;
	DNSServiceErrorType dnssd_err;
	char used_rendezvous=0;
	uint16_t using_port=0;
#endif
	for(ai_res_p=first_addrinfo_result; ai_res_p && sockets_in_use<MAX_SOCKETS; ai_res_p=ai_res_p->ai_next) {
		current_socket = socket(ai_res_p->ai_family, ai_res_p->ai_socktype, ai_res_p->ai_protocol);
		if (current_socket < 0) continue;

#if USE_RENDEZVOUS
		if(using_port) {
			if(ai_res_p->ai_family==PF_INET) {
				((struct sockaddr_in *)(ai_res_p->ai_addr))->sin_port=using_port;
			} else if(ai_res_p->ai_family==PF_INET6) {
				((struct sockaddr_in6 *)(ai_res_p->ai_addr))->sin6_port=using_port;
			}
		}
#endif
		if (bind(current_socket, ai_res_p->ai_addr, ai_res_p->ai_addrlen) < 0) {
#if USE_RENDEZVOUS
			if(using_port) {
				close(current_socket);
				syslog(LOG_ERR, "bind() failed");
				continue;
			} else if(ai_res_p->ai_family==PF_INET) {
				((struct sockaddr_in *)(ai_res_p->ai_addr))->sin_port=0;
			} else if(ai_res_p->ai_family==PF_INET6) {
				((struct sockaddr_in6 *)(ai_res_p->ai_addr))->sin6_port=0;
			}
			if(bind(current_socket, ai_res_p->ai_addr, ai_res_p->ai_addrlen) < 0) {
				syslog(LOG_ERR, "bind() failed");
				close(current_socket);
				continue;
			}
#else
			syslog(LOG_ERR, "bind() failed");
			close(current_socket);
			continue;
#endif
		}

		if (listen(current_socket, SOMAXCONN) < 0) {
			syslog(LOG_ERR, "listen() failed");
			close(current_socket);
			continue;
		}


		pid_t childpid=fork();
		if(childpid==-1) {
			close(current_socket);
			syslog(LOG_ERR, "Fork() failed");
			break;
		} else if(!childpid) {
			// Child
			if(su_after_bind) drop_privs();
			accept_loop(current_socket, ai_res_p->ai_addrlen);
			exit(0);
		} else {
			// Parent
			nhpidlist[sockets_in_use]=childpid;
#if USE_RENDEZVOUS
			if(getsockname(current_socket, ai_res_p->ai_addr, &(ai_res_p->ai_addrlen))!=0) {
				// nothing
			} else if(ai_res_p->ai_family==PF_INET) {
				using_port=((struct sockaddr_in *)(ai_res_p->ai_addr))->sin_port;
			} else if(ai_res_p->ai_family==PF_INET6) {
				using_port=((struct sockaddr_in6 *)(ai_res_p->ai_addr))->sin6_port;
			}
			close(current_socket);
			printf("Using port: %i\n", using_port);
			if(using_port && ! used_rendezvous) {
				dnssd_err=DNSServiceRegister( &sdRef, kDNSServiceFlagsAutoRename, 0,
					NULL, "_pop3._tcp", NULL,
					NULL, using_port, 0, NULL, NULL, NULL); 
				if(dnssd_err==kDNSServiceErr_NoError) used_rendezvous=1;
			}
#endif
		}
		sockets_in_use++;
	}
	freeaddrinfo(first_addrinfo_result);
	if (sockets_in_use == 0) return 0;
	int i;
	for(i=0;i<sockets_in_use;i++) {
		wait(NULL);
	}
#if USE_RENDEZVOUS
	if(used_rendezvous) DNSServiceRefDeallocate(sdRef);
#endif
	return 1;
}
