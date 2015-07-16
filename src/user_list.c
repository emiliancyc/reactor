#include "user_list.h"

#define MAX_IDX (MAX_USERS-1)

struct user_list_ctx {
	size_t current_idx;
	struct user* users[MAX_USERS];
};

static size_t find_idx(struct user_list_ctx* ctx, int fd)
{
	size_t i = 0;
	for (i = 0; i <= ctx->current_idx; i++) {
		if (ctx->users[i] && (ctx->users[i]->fd == fd)) {
			break;
		}
	}

	return i;
}

static void add_user(struct user_list_ctx* ctx, struct user* u)
{
	if (ctx->current_idx < MAX_IDX) {
		if ((ctx->current_idx == 0) && (ctx->users[0] == 0))
			ctx->users[0] = u;
		else
			ctx->users[++(ctx->current_idx)] = u;
	}
}

static void rm_user_by_fd(struct user_list_ctx* ctx, int fd)
{
	size_t i = find_idx(ctx, fd);
	if (i > ctx->current_idx)
		return;

	free(ctx->users[i]);
	if (i < ctx->current_idx)
		ctx->users[i] = ctx->users[ctx->current_idx];

	ctx->users[ctx->current_idx] = 0;
	if (ctx->current_idx > 0) {
		--(ctx->current_idx);
	}
}

static const struct user* get_user_by_fd(struct user_list_ctx* ctx, int fd)
{
	struct user* result = 0;
	size_t i = find_idx(ctx, fd);
	if (i <= ctx->current_idx)
		result = ctx->users[i];

	return result;
}

static size_t current_size(struct user_list_ctx* ctx)
{
	return ctx->current_idx + 1;
}

static const char** get_user_names(struct user_list_ctx* ctx, size_t* len)
{
	char** result = 0;
	size_t i = 0;

	*len = current_size(ctx);
	result =  malloc((*len) * sizeof(char*));

	for (; i < (*len); ++i) {
		result[i] = ctx->users[i]->name;
	}

	return (const char**) result;
}

struct user_list* create_user_list()
{
	struct user_list_ctx* ctx = (struct user_list_ctx*) malloc(sizeof(struct user_list_ctx));
	struct user_list* ul = (struct user_list*) malloc(sizeof(struct user_list));

	ctx->users[0] = 0;
	ctx->current_idx = 0;
	ul->ctx = ctx;
	ul->add_user = &add_user;
	ul->rm_user_by_fd = &rm_user_by_fd;
	ul->get_user_by_fd = &get_user_by_fd;
	ul->current_size = &current_size;
	ul->get_user_names = &get_user_names;

	return ul;
}

void delete_user_list(struct user_list* ul)
{
	free(ul->ctx);
	free(ul);
}

