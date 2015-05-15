#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>



#include "dect.h"
#include "tty.h"
#include "error.h"
#include "boot.h"
#include "state.h"
#include "util.h"
#include "app.h"
#include "nvs.h"



#define MAX_EVENTS 10
#define BUF_SIZE 50000

struct sigaction act;

void sighandler(int signum, siginfo_t * info, void * ptr) {

	printf("Recieved signal %d\n", signum);
}


int client_fd_g = 0;



int main(int argc, char * argv[]) {
	
	struct epoll_event ev, events[MAX_EVENTS];
	int state = BOOT_STATE;
	int epoll_fd, nfds, i, count, listen_fd, client_fd, ret;
	uint8_t inbuf[BUF_SIZE];
	uint8_t outbuf[BUF_SIZE];
	void (*state_event_handler)(event_t *e);
	int dect_fd;
	event_t event;
	event_t *e = &event;
	config_t c;
	config_t *config = &c;
	struct sockaddr_in my_addr, peer_addr;
	socklen_t peer_addr_size;
	uint8_t buf[BUF_SIZE];


	e->in = inbuf;
	e->out = outbuf;

	setbuf(stdout, NULL);
	
	/* Setup signal handler. When writing data to a
	   client that closed the connection we get a 
	   SIGPIPE. We need to catch it to avoid being killed */
	memset(&act, 0, sizeof(act));
	act.sa_sigaction = sighandler;
	act.sa_flags = SA_SIGINFO;
	sigaction(SIGPIPE, &act, NULL);
	

	/* Setup epoll instance */
	epoll_fd = epoll_create(10);
	if (epoll_fd == -1) {
		exit_failure("epoll_create\n");
	}


	/* Setup serial input */
	dect_fd = open("/dev/ttyS1", O_RDWR);
	if (dect_fd == -1) {
		exit_failure("open\n");
	}

	ev.events = EPOLLIN;
	ev.data.fd = dect_fd;

	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, dect_fd, &ev) == -1) {
		exit_failure("epoll_ctl\n");
	}

	
	/* Setup listening socket */
	memset(&my_addr, 0, sizeof(my_addr));
	my_addr.sin_family = AF_INET;
	my_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	my_addr.sin_port = htons(7777);
	
	if ( (listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1 ) {
		exit_failure("socket");
	}

	if ( (bind(listen_fd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr))) == -1) {
		exit_failure("bind");
	}
	
	if ( (listen(listen_fd, MAX_LISTENERS)) == -1 ) {
		exit_failure("bind");
	}

	
	ev.events = EPOLLIN;
	ev.data.fd = listen_fd;

	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &ev) == -1) {
		exit_failure("epoll_ctl\n");
	}


	/* Check user arguments and init config */
	check_args(argc, argv, config);

	/* Initial transition */
	if (config->mode == PROG_MODE) {

		/* Program new firmware */
		state_add_handler(boot_state, dect_fd);
		state_transition(BOOT_STATE);

	} else if (config->mode == NVS_MODE) {

		/* Firmware written, setup NVS */
		state_add_handler(nvs_state, dect_fd);
		state_transition(NVS_STATE);

	} else if (config->mode == APP_MODE) {

		/* Radio on, start regmode */
		state_add_handler(app_state, dect_fd);
		state_transition(APP_STATE);

	} else {
		err_exit("No known operating mode selected\n");
	}


	for(;;) {

		nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
		if (nfds == -1) {
			exit_failure("epoll_wait\n");
		}

		for (i = 0; i < nfds; ++i) {
			if (events[i].data.fd == dect_fd) {

				e->fd = dect_fd;
				e->incount = read(e->fd, e->in, BUF_SIZE);
				//util_dump(e->in, e->incount, "[READ]");
				
				/* Dispatch to current event handler */
				state_event_handler = state_get_handler();
				state_event_handler(e);

				/* Write reply if there is one */
				if (e->outcount > 0) {
					util_dump(e->out, e->outcount, "[WRITE]");
					write(e->fd, e->out, e->outcount);
				}

				/* Reset event_t */
				e->outcount = 0;
				e->incount = 0;
				memset(e->out, 0, BUF_SIZE);
				memset(e->in, 0, BUF_SIZE);
				
			} else if (events[i].data.fd == listen_fd) {

				peer_addr_size = sizeof(peer_addr);
				if ( (client_fd = accept(listen_fd, (struct sockaddr *) &peer_addr, &peer_addr_size)) == -1) {
					exit_failure("accept");
				} else {

					printf("accepted connection: %d\n", client_fd);

					/* Add new connection to epoll instance */
					ev.events = EPOLLIN;
					ev.data.fd = client_fd;

					if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev) == -1) {
						exit_failure("epoll_ctl\n");
					}

					client_fd_g = client_fd;
				}
				
			} else {
				
				client_fd = events[i].data.fd;
				
				/* Client connection */
				ret = recv(client_fd, buf, sizeof(buf), 0);
				if ( ret == -1 ) {
					
					perror("recv");
				} else if ( ret == 0 ) {

					/* Deregister fd */
					if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL) == -1) {
						exit_failure("epoll_ctl\n");
					}
					
					/* Client connection closed */
					printf("client closed connection\n");
					if (close(client_fd) == -1) {
						exit_failure("close");
					}
					
					client_fd_g = 0;

				} else {

					/* Data is read from client */
					/* We should buffer packet here to make 
					   sure we send a complete application frame */
					util_dump(buf, ret, "[CLIENT]");
					busmail_send(buf, ret);
				}
				
			}
		}
	}
	
	return 0;
}
