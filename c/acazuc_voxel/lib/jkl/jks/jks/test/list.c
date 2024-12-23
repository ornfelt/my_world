#include "list.h"
#include <assert.h>
#include <stddef.h>
#include <stdio.h>

void test_list()
{
	jks_list_t list;
	jks_list_init(&list, sizeof(uint32_t), NULL);
	uint32_t tmp = 5;
	assert(jks_list_push_back(&list, &tmp));
	tmp = 6;
	assert(jks_list_push_back(&list, &tmp));
	tmp = 4;
	assert(jks_list_push_front(&list, &tmp));
	assert(list.size == 3);
	assert(list.head != NULL);
	assert(list.tail != NULL);
	assert(list.head->prev == NULL);
	assert(list.head->next != NULL);
	assert(list.tail->prev != NULL);
	assert(list.tail->next == NULL);
	assert(list.head->next == list.tail->prev);
	assert(list.head != list.tail);
	assert(jks_list_erase(&list, 1));
	assert(list.size == 2);
	void *data;
	assert((data = jks_list_get(&list, 0)));
	assert(*(uint32_t*)data == 4);
	assert((data = jks_list_get(&list, 1)));
	assert(*(uint32_t*)data == 6);
	jks_list_destroy(&list);
	printf("[OK] test_list\n");
}
