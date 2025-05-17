#include <search.h>

struct elem
{
	struct elem *next;
	struct elem *prev;
};

void
insque(void *elem, void *prev)
{
	struct elem *prev_elem;
	struct elem *cur;

	cur = elem;
	if (!prev)
	{
		cur->prev = NULL;
		cur->next = NULL;
		return;
	}
	prev_elem = prev;
	if (prev_elem->next)
		prev_elem->next->prev = elem;
	cur->next = prev_elem->next;
	cur->prev = prev;
	prev_elem->next = elem;
}
