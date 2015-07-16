#include "server_eh.h"
#include "sys/socket.h"
#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"

struct eh_ctx{
	int fd;
	reactor* r;
};

static int get_fd(event_handler* self){
	return self->ctx->fd;
}

static int handle_event(event_handler* self, const struct epoll_event* e){
	int cli_fd=-1;
	event_handler* cli_eh = 0 ;
	
	struct sockaddr_in cli_addr;
	socklen_t cli_addr_len = sizeof(cli_addr);
	memset(&cli_addr, 0, cli_addr_len);
	cli_fd=accept(self->ctx->fd, &cli_addr, &cli_addr_len);
	if(cli_fd<0){
		printf("Cannot accept client\n");
		exit(1);
	}
	
	cli_eh = create_client_eh(cli_fd);
	self->ctx->r->add_eh(self->ctx->r, cli_eh);
	return 0;
}

event_handler* create_server_eh(reactor* r, int port, int size){
	event_handler* seh=malloc(sizeof(event_handler));
	eh_ctx* ctx = malloc(sizeof(eh_ctx));
	ctx->fd=socket(AF_INET, SOCK_NONBLOCK|SOCK_STREAM, 0); //sprawdzic czy sie zgadza z serwerem poprzednim
	//bind(ctx->fd, ...);
	//listen(ctx->fd, ...); //tutaj bedzie copypaste z poprzedniego programu
	seh->ctx=ctx; //tutaj mamy polimorfizm bo przypisujemy wskazniki na konstruktor
	seh->get_fd = get_fd;
	seh->handle_event = handle_event;
	seh->destroy = destroy_server_eh;
	return seh;
}

static void destroy_server_eh(event_handler* self){
	free(self->ctx);
	free(self);
}

	
