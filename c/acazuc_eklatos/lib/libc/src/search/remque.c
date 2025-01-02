#include <search.h>

struct elem
{
	struct elem *next;
	struct elem *prev;
};

void remque(void *elem)
{
	struct elem *cur = elem;
	if (cur->prev)
		cur->prev->next = cur->next;
	if (cur->next)
		cur->next->prev = cur->prev;
}
