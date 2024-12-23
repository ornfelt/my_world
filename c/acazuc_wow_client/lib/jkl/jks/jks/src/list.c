#include "list.h"
#include <string.h>
#include <stdlib.h>

#define JKS_LIST_MALLOC(size) (list->memory_fn ? list->memory_fn->malloc(size) : malloc(size))
#define JKS_LIST_REALLOC(ptr, size) (list->memory_fn ? list->memory_fn->realloc(ptr, size) : realloc(ptr, size))
#define JKS_LIST_FREE(ptr) (list->memory_fn ? list->memory_fn->free(ptr) : free(ptr))

void jks_list_init(jks_list_t *list, size_t data_size,
                   jks_list_destructor_t destructor,
                   const jks_list_memory_fn_t *memory_fn)
{
	list->head = NULL;
	list->tail = NULL;
	list->size = 0;
	list->data_size = data_size;
	list->destructor = destructor;
	list->memory_fn = memory_fn;
}

void jks_list_destroy(jks_list_t *list)
{
	if (list->destructor)
	{
		for (jks_list_item_t *item = list->head, *next; item; item = next)
		{
			next = item->next;
			list->destructor(item->data);
			JKS_LIST_FREE(item);
		}
	}
	else
	{
		for (jks_list_item_t *item = list->head, *next; item; item = next)
		{
			next = item->next;
			JKS_LIST_FREE(item);
		}
	}
	list->size = 0;
	list->head = NULL;
	list->tail = NULL;
}

static jks_list_item_t *create_item(jks_list_t *list)
{
	return JKS_LIST_MALLOC(sizeof(jks_list_item_t) + list->data_size);
}

bool jks_list_resize(jks_list_t *list, size_t size)
{
	if (list->size == size)
		return true;
	if (list->size < size)
	{
		jks_list_item_t *new_head = create_item(list);
		if (new_head == NULL)
			return false;
		new_head->prev = list->head;
		new_head->next = NULL;
		jks_list_item_t *new_tail = new_head;
		size_t to_alloc = size - list->size - 1;
		while (to_alloc)
		{
			jks_list_item_t *tmp = create_item(list);
			if (!tmp)
			{
				jks_list_item_t *item = new_head;
				while (item)
				{
					jks_list_item_t *next = item->next;
					JKS_LIST_FREE(item);
					item = next;
				}
				return false;
			}
			new_tail->next = tmp;
			tmp->prev = new_tail;
			new_tail = tmp;
		}
		if (!list->head)
			list->head = new_head;
		if (list->tail)
			list->tail->next = new_head;
		list->tail = new_tail;
	}
	else
	{
		size_t to_remove = list->size - size;
		jks_list_item_t *item = list->tail;
		while (to_remove)
		{
			if (item->prev)
				item->prev->next = NULL;
			jks_list_item_t *next = item->prev;
			if (list->destructor)
				list->destructor(item->data);
			JKS_LIST_FREE(item);
			item = next;
			to_remove--;
		}
		list->tail = item;
		if (!size)
			list->head = NULL;
	}
	return true;
}

static jks_list_item_t *get_item(const jks_list_t *list, size_t offset)
{
	if (offset >= list->size)
		return NULL;
	if (offset <= list->size / 2)
	{
		jks_list_item_t *item = list->head;
		while (offset)
		{
			offset--;
			item = item->next;
		}
		return item;
	}
	else
	{
		offset = list->size - offset - 1;
		jks_list_item_t *item = list->tail;
		while (offset)
		{
			offset--;
			item = item->prev;
		}
		return item;
	}
}

void *jks_list_get(const jks_list_t *list, size_t offset)
{
	jks_list_item_t *item = get_item(list, offset);
	if (!item)
		return NULL;
	return item->data;
}

void *jks_list_get_head(const jks_list_t *list)
{
	if (!list->head)
		return NULL;
	return list->head->data;
}

void *jks_list_get_tail(const jks_list_t *list)
{
	if (!list->tail)
		return NULL;
	return list->tail->data;
}

void *jks_list_push(jks_list_t *list, void *data, size_t offset)
{
	jks_list_item_t *item;
	if (offset > list->size)
	{
		item = NULL;
	}
	else
	{
		item = get_item(list, offset);
		if (!item)
			return NULL;
	}
	jks_list_item_t *new_item = create_item(list);
	if (!new_item)
		return NULL;
	if (item)
	{
		new_item->prev = item;
		new_item->next = item->next;
		if (new_item->next)
			new_item->next->prev = new_item;
		item->next = new_item;
	}
	else
	{
		if (list->tail)
		{
			new_item->prev = item;
			list->tail->next = new_item;
			new_item->prev = list->tail;
			list->tail = new_item;
		}
		else
		{
			new_item->prev = NULL;
			new_item->next = NULL;
			list->head = new_item;
			list->tail = new_item;
		}
	}
	void *dst = item->data;
	if (data)
		memcpy(dst, data, list->data_size);
	list->size++;
	return dst;
}

void *jks_list_push_head(jks_list_t *list, void *data)
{
	jks_list_item_t *item = create_item(list);
	if (!item)
		return NULL;
	item->prev = NULL;
	item->next = list->head;
	void *dst = item->data;
	if (data)
		memcpy(dst, data, list->data_size);
	if (list->head)
		list->head->prev = item;
	list->head = item;
	if (!list->tail)
		list->tail = item;
	list->size++;
	return dst;
}

void *jks_list_push_tail(jks_list_t *list, void *data)
{
	jks_list_item_t *item = create_item(list);
	if (!item)
		return NULL;
	item->prev = list->tail;
	item->next = NULL;
	void *dst = item->data;
	if (data)
		memcpy(dst, data, list->data_size);
	if (list->tail)
		list->tail->next = item;
	list->tail = item;
	if (!list->head)
		list->head = item;
	list->size++;
	return dst;
}

bool jks_list_erase(jks_list_t *list, size_t offset)
{
	jks_list_item_t *item = get_item(list, offset);
	if (!item)
		return false;
	if (item == list->head)
		list->head = item->next;
	if (item == list->tail)
		list->tail = item->prev;
	if (item->next)
		item->next->prev = item->prev;
	if (item->prev)
		item->prev->next = item->next;
	if (list->destructor)
		list->destructor(item->data);
	JKS_LIST_FREE(item);
	list->size--;
	return true;
}

bool jks_list_erase_head(jks_list_t *list)
{
	jks_list_item_t *item = list->head;
	if (!item)
		return false;
	if (list->destructor)
		list->destructor(item->data);
	if (item == list->tail)
		list->tail = NULL;
	list->head = item->next;
	if (list->head)
		list->head->prev = NULL;
	JKS_LIST_FREE(item);
	list->size--;
	return true;
}

bool jks_list_erase_tail(jks_list_t *list)
{
	jks_list_item_t *item = list->tail;
	if (!item)
		return false;
	if (list->destructor)
		list->destructor(item->data);
	if (item == list->head)
		list->head = NULL;
	list->tail = item->prev;
	if (list->tail)
		list->tail->next = NULL;
	JKS_LIST_FREE(item);
	list->size--;
	return true;
}

jks_list_iterator_t jks_list_iterator_begin(const jks_list_t *list)
{
	jks_list_iterator_t iter = {list->head};
	return iter;
}

jks_list_iterator_t jks_list_iterator_end(const jks_list_t *list)
{
	(void)list;
	jks_list_iterator_t iter = {NULL};
	return iter;
}

jks_list_iterator_t jks_list_iterator_find(const jks_list_t *list, size_t offset)
{
	jks_list_iterator_t iter = {get_item(list, offset)};
	return iter;
}

void *jks_list_iterator_get(const jks_list_iterator_t *iterator)
{
	return (uint8_t*)iterator->item + sizeof(*iterator->item);
}

void jks_list_iterator_erase(jks_list_t *list,
                             const jks_list_iterator_t *iterator)
{
	jks_list_item_t *item = iterator->item;
	if (item == list->head)
		list->head = item->next;
	if (item == list->tail)
		list->tail = item->prev;
	if (item->next)
		item->next->prev = item->prev;
	if (item->prev)
		item->prev->next = item->next;
	if (list->destructor)
		list->destructor(item->data);
	list->size--;
	JKS_LIST_FREE(item);
}

bool jks_list_iterator_is_end(const jks_list_t *list,
                              const jks_list_iterator_t *iterator)
{
	(void)list;
	return iterator->item == NULL;
}

void jks_list_iterator_next(jks_list_iterator_t *iterator)
{
	iterator->item = iterator->item->next;
}
