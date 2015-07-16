#include <stdlib.h>
#include <stdbool.h>

enum message_type {
	ACK_NACK,
	LOG_IN,
	USER_LIST,
	USER_LIST_REPLY
};

struct message {
	enum message_type x;
	char* y;
	char* z;
};

int send_ack_nack(int fd, bool is_error, const char* error_msg);
int send_log_in(int fd, const char* name);
int send_user_list(int fd);
int send_user_list_reply(int fd, const char* names[], size_t len);
struct message* receive_message(int fd);
void delete_message(struct message* m);

