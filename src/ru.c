#include "ru.h"
#include <unistd.h>
#include <string.h>

static int send_bytes(int fd, const char* msg, size_t len)
{
	int result = -1;
	if (write(fd, &len, sizeof(size_t)) > 0)
		if (write(fd, msg, len) == len)
			result = 0;

	return result;
}

int send_ack_nack(int fd, bool is_error, const char* error_msg)
{
	char* msg = "1.0";
	size_t len = 3;
	if (is_error) {
		len = strlen(error_msg) + 4;
		msg = malloc((len + 1) * sizeof(char));
		strcpy(msg, "1.1.");
		strcpy(msg+4, error_msg);
	}

	return send_bytes(fd, msg, len);
}

int send_log_in(int fd, const char* name)
{
	size_t len = strlen(name) + 2;
	char* msg = malloc((len + 1) * sizeof(char));
	strcpy(msg, "2.");
	strcpy(msg + 2, name);

	return send_bytes(fd, msg, len);
}

int send_user_list(int fd)
{
	char* msg = "6.";
	size_t len = 2;

	return send_bytes(fd, msg, len);
}

int send_user_list_reply(int fd, const char* names[], size_t len)
{
	char* msg = 0;
	size_t mlen = 2;
	size_t i = 0;
	size_t offset = 0;
	for (; i < len; ++i) {
		mlen += strlen(names[i]) + 1; //dot separator
	}
	msg = malloc(mlen * sizeof(char));
	strcpy(msg, "7.");
	offset = 2;
	for (i = 0; i < len; ++i) {
		strcpy(msg+offset, names[i]);
		offset += strlen(names[i]);
		if (i < (len-1)) {
			msg[offset] = '.';
			++offset;
		}
	}

	return send_bytes(fd, msg, mlen);
}

struct message* receive_message(int fd)
{
	size_t len = 0;
	char* msg = 0;
	struct message* m = 0;

	if (read(fd, &len, sizeof(size_t)) < 1)
		return 0;

	msg = malloc(len * sizeof(char));
	if (read(fd, msg, len) != len) {
		return 0;
	}

	m = malloc(sizeof(struct message));
	switch (msg[0]) {
		case '1':
			m->x = ACK_NACK;
			m->y = 0;
			m->z = 0;
			break;
		case '2':
		case '7':
			m->x = LOG_IN;
			m->y = malloc((len-1) * sizeof(char));
			strncpy(m->y, msg+2, len-2);
			m->y[len-2] = 0;
			m->z = 0;
			break;
		case '6':
			m->x = USER_LIST;
			m->y = 0;
			m->z = 0;
			break;
		default:
			free(m);
			m = 0;
	}

	free(msg);
	return m;
}

void delete_message(struct message* m)
{
	if (m->y)
		free(m->y);

	if (m->z)
		free(m->z);

	free(m);
}

