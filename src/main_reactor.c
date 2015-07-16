#include "reactor.h"
#include "server_eh.h"

int main(int argc, char** argv){
//int port = atoi(argv[2]);
//int size = atoi(argv[1]);
	reactor* r = create_reactor(atoi(argv[..]));
	eventhandler* seh = create_server_eh(r, port, size);
	r->add_eh(r,seh);
	r->event_loop(r);
	return 0;
}
	
