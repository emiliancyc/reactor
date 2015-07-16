#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <errno.h>

#include "user_list.h"
#include "ru.h"

extern errno;

int init_server(int* s, int* e);
int accept_client(int s, int e);
int serve_client(int e, struct epoll_event* ee, struct user_list* ul);
int handle_client_message(int fd, struct message* m, struct user_list* ul);

int main(int argc, const char *argv[])
{
	int i = 0;

	int srv_fd = -1;
	int epoll_fd = -1;

	struct epoll_event es[MAX_USERS];
	struct user_list* ul = create_user_list();

	if (init_server(&srv_fd, &epoll_fd) != 0)
		return 1;

	for(;;) {
		i = epoll_wait(epoll_fd, es, MAX_USERS, -1);
		if (i < 0) {
			printf("Cannot wait for events\n");
			close(epoll_fd);
			close(srv_fd);
			return 1;
		}

		for (--i; i > -1; --i) {
			if (es[i].data.fd == srv_fd) {
				if (accept_client(srv_fd, epoll_fd) < 0)
					return 1;
			} else {
				if (serve_client(epoll_fd, &es[i], ul) < 0) {
					close(epoll_fd);
					close(srv_fd);
					return 1;
				}
			}
		}
	}

	return 0;
}

int init_server(int* s, int* e)
{
	int srv_fd = -1;
	int epoll_fd = -1;
	struct sockaddr_in srv_addr;
	struct epoll_event ee;

	memset(&srv_addr, 0, sizeof(struct sockaddr_in));
	memset(&ee, 0, sizeof(e));

	srv_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if (srv_fd < 0) {
		printf("Cannot create socket\n");
		return 1;
	}

	srv_addr.sin_family = AF_INET;
	srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	srv_addr.sin_port = htons(5557);
	if (bind(srv_fd, (struct sockaddr*) &srv_addr, sizeof(srv_addr)) < 0) {
		printf("Cannot bind socket\n");
		close(srv_fd);
		return 1;
	}

	if (listen(srv_fd, 1) < 0) {
		printf("Cannot listen\n");
		close(srv_fd);
		return 1;
	}

	epoll_fd = epoll_create(MAX_USERS + 1);
	if (epoll_fd < 0) {
		printf("Cannot create epoll\n");
		close(srv_fd);
		return 1;
	}

	ee.events = EPOLLIN;
	ee.data.fd = srv_fd;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, srv_fd, &ee) < 0) {
		printf("Cannot add server socket to epoll\n");
		close(epoll_fd);
		close(srv_fd);
		return 1;
	}

	*s = srv_fd;
	*e = epoll_fd;
	return 0;
}

int accept_client(int s, int e)
{
	int cli_fd = -1;
	struct sockaddr_in cli_addr;
	socklen_t cli_addr_len = sizeof(cli_addr);
	struct epoll_event ee;

	memset(&cli_addr, 0, sizeof(cli_addr));
	memset(&ee, 0, sizeof(ee));

	cli_fd = accept(s, (struct sockaddr*) &cli_addr, &cli_addr_len);
	if (cli_fd < 0) {
		printf("Cannot accept client: %s\n", strerror(errno));
		close(e);
		close(s);
		return -1;
	}

	ee.events = EPOLLIN;
	ee.data.fd = cli_fd;
	if (epoll_ctl(e, EPOLL_CTL_ADD, cli_fd, &ee) < 0) {
		printf("Cannot add client to epoll\n");
		close(e);
		close(s);
		return -1;
	}

	return cli_fd;
}

int serve_client(int e, struct epoll_event* ee, struct user_list* ul)
{
	int result = -1;
	struct message* m = 0;
	if (ee->events & EPOLLIN) {
		m = receive_message(ee->data.fd);
		if (m)
			result = handle_client_message(ee->data.fd, m, ul);
	}

	if (result < 0) {
		ul->rm_user_by_fd(ul->ctx, ee->data.fd);
		result = epoll_ctl(e, EPOLL_CTL_DEL, ee->data.fd, ee);
		close(ee->data.fd);
		result = 0;
	}

	return result;
}

int handle_client_message(int fd, struct message* m, struct user_list* ul)
{
	int result = -1;
	struct user* u = 0;
	const char** ulst = 0;
	size_t len = 0;
	switch (m->x) {
		case LOG_IN:
			u = malloc(sizeof(struct user));
			u->fd = fd;
			strcpy(u->name, m->y);
			ul->add_user(ul->ctx, u);
			result = send_ack_nack(fd, false, 0);
			break;
		case USER_LIST:
			ulst = ul->get_user_names(ul->ctx, &len);
			result = send_user_list_reply(fd, ulst, len);
			free(ulst);
			break;
		default:
			break;
	}

	delete_message(m);
	return result;
}

