#ifndef JKS_LIST_H
#define JKS_LIST_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef void (*jks_list_destructor_t)(void *data);

typedef struct jks_list_item jks_list_item_t;

typedef struct jks_list_memory_fn
{
	void *(*malloc)(size_t size);
	void *(*realloc)(void *ptr, size_t size);
	void (*free)(void *data);
} jks_list_memory_fn_t;

struct jks_list_item
{
	jks_list_item_t *prev;
	jks_list_item_t *next;
	char data[];
};

typedef struct jks_list_iterator
{
	jks_list_item_t *item;
} jks_list_iterator_t;

typedef struct jks_list
{
	jks_list_item_t *head;
	jks_list_item_t *tail;
	size_t size;
	size_t data_size;
	jks_list_destructor_t destructor;
	const jks_list_memory_fn_t *memory_fn;
} jks_list_t;

void jks_list_init(jks_list_t *list, size_t data_size,
                   jks_list_destructor_t destructor,
                   const jks_list_memory_fn_t *memory_fn);
void jks_list_destroy(jks_list_t *list);

bool jks_list_resize(jks_list_t *list, size_t size);

void *jks_list_get(const jks_list_t *list, size_t offset);
void *jks_list_get_head(const jks_list_t *list);
void *jks_list_get_tail(const jks_list_t *list);

void *jks_list_push(jks_list_t *list, void *data, size_t offset);
void *jks_list_push_head(jks_list_t *list, void *data);
void *jks_list_push_tail(jks_list_t *list, void *data);

bool jks_list_erase(jks_list_t *list, size_t offset);
bool jks_list_erase_head(jks_list_t *list);
bool jks_list_erase_tail(jks_list_t *list);

jks_list_iterator_t jks_list_iterator_begin(const jks_list_t *list);
jks_list_iterator_t jks_list_iterator_end(const jks_list_t *list);
jks_list_iterator_t jks_list_iterator_find(const jks_list_t *list,
                                           size_t offset);

void *jks_list_iterator_get(const jks_list_iterator_t *iterator);
void jks_list_iterator_erase(jks_list_t *list,
                             const jks_list_iterator_t *iterator);
bool jks_list_iterator_is_end(const jks_list_t *list,
                              const jks_list_iterator_t *iterator);
void jks_list_iterator_next(jks_list_iterator_t *iterator);

#define JKS_LIST_FOREACH(iterator, list) \
	for (jks_list_iterator_t iterator = jks_list_iterator_begin(list); \
	     !jks_list_iterator_is_end(list, &iterator); \
	     jks_list_iterator_next(&iterator))

#ifdef __cplusplus
}
#endif

#endif
