#include "event_handler.h"
#include "reactor.h"

event_handler* create_server_eh(reactor* r, int port, int size);
void destroy_server_eh(event_handler* eh);

