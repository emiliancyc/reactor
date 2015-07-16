typedef struct reactor_ctx reactor_ctx;
typedef struct reactor{
	reactor_ctx* ctx;
	void (*add_eh)(struct reactor* self, event_handler* eh);
	void (*rm_eh)(struct reactor* self, event_handler* eh);
 	void (*event_loop)(struct reactor* self);
} reactor;

reactor* create_reactor(size_t size);
void destroy_reactor(reactor* r);
