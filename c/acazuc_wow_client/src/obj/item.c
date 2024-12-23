#include "obj/update_fields.h"
#include "obj/item.h"

#include "memory.h"
#include "const.h"
#include "log.h"
#include "wow.h"
#include "wdb.h"

#define ITEM ((struct item*)object)

static void on_object_field_entry(struct object *object)
{
	wdb_get_item(g_wow->wdb, object_fields_get_u32(&object->fields, OBJECT_FIELD_ENTRY));
}

static void (* const g_field_functions[ITEM_FIELD_MAX])(struct object *object) =
{
	[OBJECT_FIELD_ENTRY] = on_object_field_entry,
};

static bool ctr(struct object *object, uint64_t guid)
{
	if (!object_vtable.ctr(object, guid))
		return false;
	if (!object_fields_resize(&object->fields, ITEM_FIELD_MAX))
		return false;
	object_fields_set_u32(&object->fields, OBJECT_FIELD_TYPE, OBJECT_MASK_OBJECT | OBJECT_MASK_ITEM);
	return true;
}

static void dtr(struct object *object)
{
	object_vtable.dtr(object);
}

static void on_field_changed(struct object *object, uint32_t field)
{
	if (field < ITEM_FIELD_MAX)
	{
		if (g_field_functions[field])
			g_field_functions[field](object);
	}
	object_vtable.on_field_changed(object, field);
}

const struct object_vtable item_object_vtable =
{
	.ctr = ctr,
	.dtr = dtr,
	.on_field_changed = on_field_changed,
};

struct item *item_new(uint64_t guid)
{
	struct object *object = mem_malloc(MEM_OBJ, sizeof(struct item));
	if (!object)
		return NULL;
	object->vtable = &item_object_vtable;
	if (!object->vtable->ctr(object, guid))
	{
		object->vtable->dtr(object);
		mem_free(MEM_OBJ, object);
		return NULL;
	}
	return (struct item*)object;
}
