#include "bignum/bignum.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

struct bignum_ctx *bignum_ctx_new(void)
{
	struct bignum_ctx *ctx = malloc(sizeof(*ctx));
	if (!ctx)
		return NULL;
	ctx->entries = NULL;
	ctx->len = 0;
	return ctx;
}

void bignum_ctx_free(struct bignum_ctx *ctx)
{
	if (!ctx)
		return;
	for (size_t i = 0; i < ctx->len; ++i)
		bignum_free(ctx->entries[i].bignum);
	free(ctx->entries);
	free(ctx);
}

struct bignum *bignum_ctx_get(struct bignum_ctx *ctx)
{
	struct bignum_ctx_entry *entry;
	for (size_t i = 0; i < ctx->len; ++i)
	{
		entry = &ctx->entries[i];
		if (!entry->used)
			goto entry_found;
	}
	size_t newlen = ctx->len ? ctx->len * 2 : 8;
	struct bignum_ctx_entry *entries = realloc(ctx->entries,
	                                           sizeof(*entries) * newlen);
	if (!entries)
		return NULL;
	memset(&entries[ctx->len], 0, sizeof(*entries) * (newlen - ctx->len));
	entry = &entries[ctx->len];
	ctx->entries = entries;
	ctx->len = newlen;

entry_found:
	if (!entry->bignum)
	{
		entry->bignum = bignum_new();
		if (!entry->bignum)
			return NULL;
	}
	entry->used = 1;
	__bignum_zero(entry->bignum);
	return entry->bignum;
}

void bignum_ctx_release(struct bignum_ctx *ctx, struct bignum *bignum)
{
	if (!bignum)
		return;
	for (size_t i = 0; i < ctx->len; ++i)
	{
		struct bignum_ctx_entry *entry = &ctx->entries[i];
		if (entry->bignum != bignum)
			continue;
		assert(entry->used);
		entry->used = 0;
		return;
	}
	assert(!"release unexisting bignum ctx entry");
}
