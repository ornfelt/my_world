#include "jkg.h"

#include <stdlib.h>

bool
jkg_id_init(struct jkg_id_list *list, uint32_t first, uint32_t last)
{
	struct jkg_id_range *entry;

	TAILQ_INIT(&list->ranges);
	list->first = first;
	list->last = last;
	entry = malloc(sizeof(*entry));
	if (!entry)
		return false;
	entry->first = first;
	entry->last = last;
	TAILQ_INSERT_TAIL(&list->ranges, entry, chain);
	return true;
}

void
jkg_id_destroy(struct jkg_id_list *list)
{
	struct jkg_id_range *entry;

	while ((entry = TAILQ_FIRST(&list->ranges)))
	{
		TAILQ_REMOVE(&list->ranges, entry, chain);
		free(entry);
	}
}

bool
jkg_id_alloc(struct jkg_id_list *list, uint32_t *id)
{
	struct jkg_id_range *entry;

	entry = TAILQ_FIRST(&list->ranges);
	if (!entry)
		return false;
	*id = entry->first;
	if (entry->first == entry->last)
	{
		TAILQ_REMOVE(&list->ranges, entry, chain);
		free(entry);
	}
	else
	{
		entry->first++;
	}
	return true;
}

bool
jkg_id_free(struct jkg_id_list *list, uint32_t id)
{
	struct jkg_id_range *entry;
	struct jkg_id_range *next;

	if (id < list->first
	 || id > list->last)
		return false;
	TAILQ_FOREACH(entry, &list->ranges, chain)
	{
		if (id == entry->last + 1)
		{
			next = TAILQ_NEXT(entry, chain);
			if (next && next->first == id + 1)
			{
				entry->last = next->last;
				TAILQ_REMOVE(&list->ranges, next, chain);
				free(next);
				return true;
			}
			entry->last++;
			return true;
		}
		if (id + 1 == entry->first)
		{
			entry->first--;
			return true;
		}
		if (id < entry->first)
		{
			next = malloc(sizeof(*next));
			if (!next)
				return false;
			next->first = id;
			next->last = id;
			TAILQ_INSERT_BEFORE(entry, next, chain);
			return true;
		}
		if (id <= entry->last)
			return false; /* double free */
	}
	next = malloc(sizeof(*next));
	if (!next)
		return false;
	next->first = id;
	next->last = id;
	TAILQ_INSERT_TAIL(&list->ranges, entry, chain);
	return true;
}
