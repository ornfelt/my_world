#include "array.h"
#include <string.h>
#include <stdlib.h>

#define JKS_ARRAY_MALLOC(size) (array->memory_fn ? array->memory_fn->malloc(size) : malloc(size))
#define JKS_ARRAY_REALLOC(ptr, size) (array->memory_fn ? array->memory_fn->realloc(ptr, size) : realloc(ptr, size))
#define JKS_ARRAY_FREE(ptr) (array->memory_fn ? array->memory_fn->free(ptr) : free(ptr))

void jks_array_init(jks_array_t *array, size_t data_size,
                    jks_array_destructor_t destructor,
                    const jks_array_memory_fn_t *memory_fn)
{
	array->data = NULL;
	array->size = 0;
	array->capacity = 0;
	array->data_size = data_size;
	array->destructor = destructor;
	array->memory_fn = memory_fn;
}

void jks_array_destroy(jks_array_t *array)
{
	if (array->destructor)
	{
		for (size_t i = 0; i < array->size; ++i)
			array->destructor(jks_array_get(array, i));
	}
	JKS_ARRAY_FREE(array->data);
	array->size = 0;
	array->data = NULL;
}

static size_t npot(size_t v)
{
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
#if __SIZE_WIDTH__ == 64
	v |= v >> 32;
#endif
	v++;
	return v;
}

bool jks_array_resize(jks_array_t *array, size_t size)
{
	if (array->size == size)
		return true;
	if (size >= array->capacity)
	{
		size_t new_capacity = size;
		if (array->capacity * 2 > new_capacity)
			new_capacity = array->capacity * 2;
		if (!jks_array_reserve(array, npot(new_capacity)))
			return false;
	}
	if (size < array->size)
	{
		if (array->destructor)
		{
			for (size_t i = size; i < array->size; ++i)
				array->destructor(jks_array_get(array, i));
		}
	}
	array->size = size;
	return true;
}

void *jks_array_grow(jks_array_t *array, size_t size)
{
	if (!size)
		return NULL;
	if (!jks_array_resize(array, array->size + size))
		return NULL;
	return jks_array_get(array, array->size - size);
}

bool jks_array_reserve(jks_array_t *array, size_t capacity)
{
	if (array->capacity >= capacity)
		return true;
	void *new_data = JKS_ARRAY_REALLOC(array->data, capacity * array->data_size);
	if (!new_data)
		return false;
	array->data = new_data;
	array->capacity = capacity;
	return true;
}

bool jks_array_shrink(jks_array_t *array)
{
	if (!array->size)
	{
		JKS_ARRAY_FREE(array->data);
		array->data = NULL;
		array->capacity = 0;
		return true;
	}
	void *new_data = JKS_ARRAY_REALLOC(array->data, array->size * array->data_size);
	if (!new_data)
		return false;
	array->data = new_data;
	array->capacity = array->size;
	return true;
}

void *jks_array_push_front(jks_array_t *array, const void *data)
{
	if (!jks_array_resize(array, array->size + 1))
		return NULL;
	void *dst = jks_array_get(array, 0);
	memmove(jks_array_get(array, 1), dst, (array->size - 1) * array->data_size);
	if (data)
		memmove(dst, data, array->data_size);
	return dst;
}

void *jks_array_push_back(jks_array_t *array, const void *data)
{
	void *dst = jks_array_grow(array, 1);
	if (!dst)
		return NULL;
	if (data)
		memmove(dst, data, array->data_size);
	return dst;
}

void *jks_array_push(jks_array_t *array, const void *data, size_t offset)
{
	if (!jks_array_resize(array, array->size + 1))
		return NULL;
	void *dst = jks_array_get(array, offset);
	memmove(jks_array_get(array, offset + 1), dst, (array->size - offset - 1) * array->data_size);
	if (data)
		memmove(dst, data, array->data_size);
	return dst;
}

bool jks_array_erase(jks_array_t *array, size_t offset)
{
	if (offset >= array->size)
		return false;
	if (array->destructor)
		array->destructor(jks_array_get(array, offset));
	memmove(jks_array_get(array, offset), jks_array_get(array, offset + 1), (array->size - offset - 1) * array->data_size);
	array->size--;
	return true;
}

jks_array_iterator_t jks_array_iterator_begin(const jks_array_t *array)
{
	jks_array_iterator_t iter = {array->data, array->data_size};
	return iter;
}

jks_array_iterator_t jks_array_iterator_end(const jks_array_t *array)
{
	jks_array_iterator_t iter = {(uint8_t*)array->data + array->size * array->data_size, array->data_size};
	return iter;
}

jks_array_iterator_t jks_array_iterator_find(const jks_array_t *array, size_t offset)
{
	if (offset >= array->size)
		return jks_array_iterator_end(array);
	jks_array_iterator_t iter = {(uint8_t*)array->data + offset * array->data_size, array->data_size};
	return iter;
}

void jks_array_iterator_erase(jks_array_t *array, const jks_array_iterator_t *iterator)
{
	if (array->destructor)
		array->destructor(iterator->data);
	size_t offset = ((uint8_t*)iterator->data - (uint8_t*)array->data) / array->data_size;
	memmove(iterator->data, (uint8_t*)iterator->data + array->data_size, (array->size - offset - 1) * array->data_size);
	array->size--;
}
