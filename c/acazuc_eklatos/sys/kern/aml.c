#define ENABLE_TRACE

#include <endian.h>
#include <queue.h>
#include <errno.h>
#include <acpi.h>
#include <std.h>
#include <sma.h>

#define NAME_MAX_LEN (255 * 4)

/*
 * some doc from acpi specs:
 * there are two "passed" of interpretation:
 * - parsing at load time
 * - execution at run time
 *
 * a definition block level is bytecode present in either device, powerresource, processor, scope or thermalzone (i.e: every scope that is not method)
 * everything (declaration of named objects & control structure (if, and, or, while..) can be present in a definition block
 */

#define AML_STATE_F_PARSE (1 << 0)
#define AML_STATE_F_PRINT (1 << 1)
#define AML_STATE_F_OFF   (1 << 2)
#define AML_STATE_F_EXEC  (1 << 3)

struct aml_it
{
	const uint8_t *ptr;
	size_t remaining;
	size_t offset;
};

struct aml_state
{
	struct uio *uio;
	struct aml_it it;
	size_t indent;
	int display_inline;
	struct acpi_ns *cur_ns;
	struct acpi_obj *root;
	int flags;
};

#define CASE_CONSTOBJ(state, opcode, data) \
	case 0x00: \
		return parse_zeroop(state, data); \
	case 0x01: \
		return parse_oneop(state, data); \
	case 0x0FF: \
		return parse_onesop(state, data); \

#define CASE_COMPUTATIONALDATA(state, opcode, data) \
	CASE_CONSTOBJ(state, opcode, data) \
	case 0x0A: \
		return parse_byteconst(state, data); \
	case 0x0B: \
		return parse_wordconst(state, data); \
	case 0x0C: \
		return parse_dwordconst(state, data); \
	case 0x0D: \
		return parse_string(state, opcode, data); \
	case 0x0E: \
		return parse_qwordconst(state, data); \

#define CASE_LOCALOBJ(state, opcode) \
	case 0x60: \
	case 0x61: \
	case 0x62: \
	case 0x63: \
	case 0x64: \
	case 0x65: \
	case 0x66: \
	case 0x67: \
		return parse_localobj(state, opcode); \

#define CASE_ARGOBJ(state, opcode) \
	case 0x68: \
	case 0x69: \
	case 0x6A: \
	case 0x6B: \
	case 0x6C: \
	case 0x6D: \
	case 0x6E: \
		return parse_argobj(state, opcode); \

#define CASE_EXPRESSION(state, opcode) \
	case 0x12: \
		return parse_packageop(state, NULL); \
	case 0x70: \
		return parse_storeop(state); \
	case 0x71: \
		return parse_refofop(state); \
	case 0x72: \
		return parse_addop(state); \
	case 0x73: \
		return parse_concatop(state); \
	case 0x74: \
		return parse_subtractop(state); \
	case 0x75: \
		return parse_incrementop(state); \
	case 0x76: \
		return parse_decrementop(state); \
	case 0x77: \
		return parse_multiplyop(state); \
	case 0x78: \
		return parse_divideop(state); \
	case 0x79: \
		return parse_shiftleftop(state); \
	case 0x7A: \
		return parse_shiftrightop(state); \
	case 0x7B: \
		return parse_andop(state); \
	case 0x7C: \
		return parse_nandop(state); \
	case 0x7D: \
		return parse_orop(state); \
	case 0x7E: \
		return parse_norop(state); \
	case 0x7F: \
		return parse_xorop(state); \
	case 0x80: \
		return parse_notop(state); \
	case 0x81: \
		return parse_findsetleftbitop(state); \
	case 0x82: \
		return parse_findsetrightbitop(state); \
	case 0x83: \
		return parse_derefofop(state); \
	case 0x84: \
		return parse_concatresop(state); \
	case 0x86: \
		return parse_notifyop(state); \
	case 0x87: \
		return parse_sizeofop(state); \
	case 0x88: \
		return parse_indexop(state); \
	case 0x89: \
		return parse_matchop(state); \
	case 0x8E: \
		return parse_objecttypeop(state); \
	case 0x90: \
		return parse_landop(state); \
	case 0x91: \
		return parse_lorop(state); \
	case 0x92: \
		return parse_lnotop(state); \
	case 0x93: \
		return parse_lequalop(state); \
	case 0x94: \
		return parse_lgreaterop(state); \
	case 0x95: \
		return parse_llessop(state); \
	case 0x96: \
		return parse_tobufferop(state); \
	case 0x97: \
		return parse_todecimalstringop(state); \
	case 0x98: \
		return parse_tohexstringop(state); \
	case 0x99: \
		return parse_tointegerop(state); \
	case 0x9C: \
		return parse_tostringop(state); \
	CASE_METHODINVOCATION(state, opcode) \

#define CASE_METHODINVOCATION(state, opcode) \
	case '\\': \
	case '^': \
	case '_': \
	case 'A': \
	case 'B': \
	case 'C': \
	case 'D': \
	case 'E': \
	case 'F': \
	case 'G': \
	case 'H': \
	case 'I': \
	case 'J': \
	case 'K': \
	case 'L': \
	case 'M': \
	case 'N': \
	case 'O': \
	case 'P': \
	case 'Q': \
	case 'R': \
	case 'S': \
	case 'T': \
	case 'U': \
	case 'V': \
	case 'W': \
	case 'X': \
	case 'Y': \
	case 'Z': \
		return parse_methodinvocation(state); \

#define CASE_STATEMENT(state, opcode) \
	case 0x9F: \
		return parse_continueop(state); \
	case 0xA0: \
		return parse_ifop(state); \
	case 0xA2: \
		return parse_whileop(state); \
	case 0xA4: \
		return parse_returnop(state); \
	case 0xA5: \
		return parse_breakop(state); \

#define CASE_NAMESTRING(state, opcode) \
	case '\\': \
	case '^': \
	case '_': \
	case 'A': \
	case 'B': \
	case 'C': \
	case 'D': \
	case 'E': \
	case 'F': \
	case 'G': \
	case 'H': \
	case 'I': \
	case 'J': \
	case 'K': \
	case 'L': \
	case 'M': \
	case 'N': \
	case 'O': \
	case 'P': \
	case 'Q': \
	case 'R': \
	case 'S': \
	case 'T': \
	case 'U': \
	case 'V': \
	case 'W': \
	case 'X': \
	case 'Y': \
	case 'Z': \
		return parse_simplename(state); \

#define CASE_SIMPLENAME(state, opcode) \
	CASE_NAMESTRING(state, opcode) \
	CASE_ARGOBJ(state, opcode) \
	CASE_LOCALOBJ(state, opcode) \

#define CASE_NAMESPACEMODIFIEROBJ(state, opcode) \
	case 0x06: \
		return parse_aliasop(state); \
	case 0x08: \
		return parse_nameop(state); \
	case 0x10: \
		return parse_scopeop(state); \

#define CASE_OBJECT(state, opcode) \
	CASE_NAMESPACEMODIFIEROBJ(state, opcode) \
	CASE_NAMEDOBJ(state, opcode) \

#define CASE_DATAOBJECT(state, opcode, data) \
	CASE_COMPUTATIONALDATA(state, opcode, data) \
	case 0x12: \
		return parse_packageop(state, data); \

#define CASE_NAMEDOBJ(state, opcode) \
	case 0x15: \
		return parse_externalop(state); \
	case 0x8A: \
		return parse_createdwordfield(state); \
	case 0x8B: \
		return parse_createwordfield(state); \
	case 0x8C: \
		return parse_createbytefield(state); \
	case 0x8D: \
		return parse_createbitfield(state); \
	case 0x8F: \
		return parse_createqwordfield(state); \

#define CASE_REFERENCETYPEOPCODE(state, opcode) \
	case 0x83: \
		return parse_derefofop(state); \
	case 0x88: \
		return parse_indexop(state); \

struct sma aml_state_sma;
struct sma acpi_data_sma;
struct sma acpi_obj_sma;
struct sma acpi_ns_sma;

static const char *tabs = "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t";

static struct acpi_ns *obj_get_ns(struct acpi_obj *obj);

void aml_init(void)
{
	sma_init(&aml_state_sma, sizeof(struct aml_state), NULL, NULL, "aml_state");
	sma_init(&acpi_data_sma, sizeof(struct acpi_data), NULL, NULL, "acpi_data");
	sma_init(&acpi_obj_sma, sizeof(struct acpi_obj), NULL, NULL, "acpi_obj");
	sma_init(&acpi_ns_sma, sizeof(struct acpi_ns), NULL, NULL, "acpi_ns");
}

static void skip(struct aml_state *state, size_t n)
{
	state->it.ptr += n;
	state->it.remaining -= n;
	state->it.offset += n;
}

static int peekb(struct aml_state *state, uint8_t *v)
{
	if (state->it.remaining < 1)
		return 1;
	*v = *state->it.ptr;
	return 0;
}

static int gets(struct aml_state *state, uint8_t *s, size_t n)
{
	if (state->it.remaining < n)
		return 1;
	memcpy(s, state->it.ptr, n);
	skip(state, n);
	return 0;
}

static int getb(struct aml_state *state, uint8_t *v)
{
	if (peekb(state, v))
		return 1;
	skip(state, 1);
	return 0;
}

static int peekw(struct aml_state *state, uint16_t *v)
{
	if (state->it.remaining < 2)
		return 1;
	*v = le16dec(state->it.ptr);
	return 0;
}

static int getw(struct aml_state *state, uint16_t *v)
{
	if (peekw(state, v))
		return 1;
	skip(state, 2);
	return 0;
}

static int peekd(struct aml_state *state, uint32_t *v)
{
	if (state->it.remaining < 4)
		return 1;
	*v = le32dec(state->it.ptr);
	return 0;
}

static int getd(struct aml_state *state, uint32_t *v)
{
	if (peekd(state, v))
		return 1;
	skip(state, 4);
	return 0;
}

static int peekq(struct aml_state *state, uint64_t *v)
{
	if (state->it.remaining < 8)
		return 1;
	*v = le64dec(state->it.ptr);
	return 0;
}

static int getq(struct aml_state *state, uint64_t *v)
{
	if (peekq(state, v))
		return 1;
	skip(state, 8);
	return 0;
}

static int ungetb(struct aml_state *state)
{
	state->it.ptr--;
	state->it.remaining++;
	state->it.offset--;
	return 0;
}

static void print_indent(struct aml_state *state)
{
	if (state->flags & AML_STATE_F_OFF)
		uprintf(state->uio, "[%05zx] ", state->it.offset);
	uprintf(state->uio, "%.*s", (int)state->indent, tabs);
}

static void indent_inc(struct aml_state *state)
{
	print_indent(state);
	uprintf(state->uio, "{\n");
	state->indent++;
}

static void indent_dec(struct aml_state *state)
{
	state->indent--;
	print_indent(state);
	uprintf(state->uio, "}\n");
}

static void inline_push(struct aml_state *state, const char *op,
                        int *prev_inline)
{
	if (!state->display_inline)
		print_indent(state);
	uprintf(state->uio, "%s(", op);
	*prev_inline = state->display_inline;
	state->display_inline = 1;
}

static void inline_pop(struct aml_state *state, int prev_inline)
{
	state->display_inline = prev_inline;
	if (state->display_inline)
		uprintf(state->uio, ")");
	else
		uprintf(state->uio, ")\n");
}

static void inline_sep(struct aml_state *state)
{
	uprintf(state->uio, ", ");
}

static void print_length(struct aml_state *state, size_t v)
{
	uprintf(state->uio, "0x%zx", v);
}

static void print_x64(struct aml_state *state, uint64_t v)
{
	uprintf(state->uio, "0x%016" PRIx64, v);
}

static void print_x32(struct aml_state *state, uint32_t v)
{
	uprintf(state->uio, "0x%08" PRIx32, v);
}

static void print_x16(struct aml_state *state, uint16_t v)
{
	uprintf(state->uio, "0x%04" PRIx16, v);
}

static void print_x8(struct aml_state *state, uint8_t v)
{
	uprintf(state->uio, "0x%02" PRIx8, v);
}

static void print_s(struct aml_state *state, const char *s)
{
	uprintf(state->uio, "%s", s);
}

static void fmt_name(char *buf, size_t *n, const char **name)
{
	if ((*name)[3] != '_')
	{
		buf[(*n)++] = *((*name)++);
		buf[(*n)++] = *((*name)++);
		buf[(*n)++] = *((*name)++);
		buf[(*n)++] = *((*name)++);
	}
	else if ((*name)[2] != '_')
	{
		buf[(*n)++] = *((*name)++);
		buf[(*n)++] = *((*name)++);
		buf[(*n)++] = *((*name)++);
		(*name)++;
	}
	else if ((*name)[1] != '_')
	{
		buf[(*n)++] = *((*name)++);
		buf[(*n)++] = *((*name)++);
		(*name) += 2;
	}
	else if ((*name)[0] != '_')
	{
		buf[(*n)++] = *((*name)++);
		(*name) += 3;
	}
	else
	{
		(*name) += 4;
	}
}

static void print_name(struct uio *uio, uint32_t name)
{
	char buf[5];
	size_t n = 0;
	uint32_t v = htole32(name);
	const char *tmp = (const char*)&v;
	fmt_name(buf, &n, &tmp);
	buf[n] = '\0';
	uprintf(uio, "%s", buf);
}

static void print_path(struct uio *uio, const char *path)
{
	char buf[NAME_MAX_LEN  / 4 * 5 + 1];
	size_t n = 0;
	while (*path == '\\' || *path == '^')
		buf[n++] = *(path++);
	while (*path)
	{
		fmt_name(buf, &n, &path);
		if (*path)
			buf[n++] = '.';
	}
	uprintf(uio, "%.*s", (int)n, buf);
}

static const char *obj_type_name(enum acpi_obj_type type)
{
	switch (type)
	{
		case ACPI_OBJ_ALIAS:
			return "Alias";
		case ACPI_OBJ_BIT_FIELD:
			return "BitField";
		case ACPI_OBJ_BYTE_FIELD:
			return "ByteField";
		case ACPI_OBJ_DEVICE:
			return "Device";
		case ACPI_OBJ_DWORD_FIELD:
			return "DWordField";
		case ACPI_OBJ_EVENT:
			return "Event";
		case ACPI_OBJ_FIELD:
			return "Field";
		case ACPI_OBJ_INDEX_FIELD:
			return "IndexField";
		case ACPI_OBJ_NAMED_FIELD:
			return "NamedField";
		case ACPI_OBJ_METHOD:
			return "Method";
		case ACPI_OBJ_MUTEX:
			return "Mutex";
		case ACPI_OBJ_NAME:
			return "Name";
		case ACPI_OBJ_OPERATION_REGION:
			return "OperationRegion";
		case ACPI_OBJ_POWER_RESOURCE:
			return "PowerResource";
		case ACPI_OBJ_PROCESSOR:
			return "Processor";
		case ACPI_OBJ_QWORD_FIELD:
			return "QWordField";
		case ACPI_OBJ_RAW_DATA_BUFFER:
			return "RawDataBuffer";
		case ACPI_OBJ_SCOPE:
			return "Scope";
		case ACPI_OBJ_THERMAL_ZONE:
			return "ThermalZone";
		case ACPI_OBJ_WORD_FIELD:
			return "WordField";
	}
	return "Unknown";
}

static const char *data_type_name(enum acpi_data_type type)
{
	switch (type)
	{
		case ACPI_DATA_BUFFER:
			return "Buffer";
		case ACPI_DATA_BYTE:
			return "Byte";
		case ACPI_DATA_DWORD:
			return "DWord";
		case ACPI_DATA_ONE:
			return "One";
		case ACPI_DATA_ONES:
			return "Ones";
		case ACPI_DATA_PACKAGE:
			return "Package";
		case ACPI_DATA_QWORD:
			return "QWord";
		case ACPI_DATA_STRING:
			return "String";
		case ACPI_DATA_WORD:
			return "Word";
		case ACPI_DATA_ZERO:
			return "Zero";
	}
	return "Unknown";
}

static void get_ns_path(char *buf, struct acpi_ns *ns)
{
	size_t n = 0;
	struct acpi_ns *tmp = ns;
	while (tmp)
	{
		n++;
		tmp = tmp->parent;
	}
	n--;
	tmp = ns;
	for (size_t i = 1; i <= n; ++i)
	{
		*(uint32_t*)&buf[(n - i) * 5] = tmp->obj->name;
		buf[(n - i) * 5 + 4] = '.';
		tmp = tmp->parent;
	}
	buf[n * 5] = '\0';
}

static struct acpi_obj *obj_alloc(struct acpi_ns *ns,
                                 enum acpi_obj_type type,
                                 uint32_t name)
{
	struct acpi_obj *obj = sma_alloc(&acpi_obj_sma, M_ZERO);
	if (!obj)
	{
		TRACE("object allocation failed");
		return NULL;
	}
	obj->ns = ns;
	obj->type = type;
	obj->name = name;
	if (ns)
		TAILQ_INSERT_TAIL(&obj->ns->obj_list, obj, chain);
	return obj;
}

static void obj_free(struct acpi_obj *obj)
{
	struct acpi_ns *ns = obj_get_ns(obj);
	if (ns)
	{
		struct acpi_obj *child = TAILQ_FIRST(&ns->obj_list);
		while (child)
		{
			obj_free(child);
			child = TAILQ_FIRST(&ns->obj_list);
		}
		sma_free(&acpi_ns_sma, ns);
	}
	sma_free(&acpi_obj_sma, obj);
}

static struct acpi_data *data_alloc(enum acpi_data_type type)
{
	struct acpi_data *data = sma_alloc(&acpi_data_sma, M_ZERO);
	if (!data)
	{
		TRACE("data allocation failed");
		return NULL;
	}
	data->type = type;
	return data;
}

static void data_free(struct acpi_data *data)
{
	if (!data)
		return;
	switch (data->type)
	{
		case ACPI_DATA_BUFFER:
			free(data->buffer.data);
			break;
		case ACPI_DATA_STRING:
			free(data->string.data);
			break;
		default:
			break;
	}
	sma_free(&acpi_data_sma, data);
}

static void ns_init(struct acpi_ns *ns, struct acpi_ns *parent,
                     struct acpi_obj *obj)
{
	ns->parent = parent;
	ns->obj = obj;
	TAILQ_INIT(&ns->obj_list);
}

static struct acpi_obj *ns_get_obj(struct acpi_ns *ns,
                                  uint32_t name)
{
	struct acpi_obj *obj;
	TAILQ_FOREACH(obj, &ns->obj_list, chain)
	{
		if (obj->name == name)
			return obj;
	}
	return NULL;
}

static struct acpi_ns *obj_get_ns(struct acpi_obj *obj)
{
	switch (obj->type)
	{
		case ACPI_OBJ_DEVICE:
			return &obj->device.ns;
		case ACPI_OBJ_POWER_RESOURCE:
			return &obj->power_resource.ns;
		case ACPI_OBJ_PROCESSOR:
			return &obj->processor.ns;
		case ACPI_OBJ_SCOPE:
			return &obj->scope.ns;
		case ACPI_OBJ_THERMAL_ZONE:
			return &obj->thermal_zone.ns;
		default:
			return NULL;
	}
}

static struct acpi_ns *get_ns(struct aml_state *state,
                             const char *path,
                             int create)
{
	struct acpi_ns *ns = state->cur_ns;
	if (path[0] == '\\')
	{
		ns = &state->root->scope.ns;
		path++;
	}
	else if (path[0] == '^')
	{
		do
		{
			path++;
			ns = ns->parent;
			if (!ns)
				ns = &state->root->scope.ns;
		} while (path[0] == '^');
	}
	while (path[4])
	{
		uint32_t name = *(uint32_t*)path;
		struct acpi_obj *obj = ns_get_obj(ns, name);
		if (!obj)
		{
			if (!create)
				return NULL;
			obj = obj_alloc(ns, ACPI_OBJ_SCOPE, name);
			if (!obj)
				return NULL;
			/* XXX this is not correct, but it
			 * helps us here to note that it's a hand-crafted
			 * scope and not a scope-declared object in aml
			 */
			obj->external = 1;
			ns_init(&obj->scope.ns, ns, obj);
		}
		path += 4;
		ns = obj_get_ns(obj);
		if (!ns)
		{
			TRACE("ns in non-ns obj: %s", obj_type_name(obj->type));
			return NULL;
		}
	}
	return ns;
}

static struct acpi_obj *get_obj_recursive(struct acpi_ns *ns,
                                         uint32_t name)
{
	if (!ns)
		return NULL;
	struct acpi_obj *obj = ns_get_obj(ns, name);
	if (obj)
		return obj;
	return get_obj_recursive(ns->parent, name);
}

static struct acpi_obj *get_obj(struct aml_state *state,
                               const char *path)
{
	int simple = 1;
	struct acpi_ns *ns = state->cur_ns;
	if (path[0] == '\\')
	{
		simple = 0;
		ns = &state->root->scope.ns;
		path++;
	}
	else if (path[0] == '^')
	{
		simple = 0;
		do
		{
			path++;
			ns = ns->parent;
			if (!ns)
				ns = &state->root->scope.ns;
		} while (path[0] == '^');
	}
	if (!*path)
		return ns->obj;
	while (1)
	{
		struct acpi_obj *obj = ns_get_obj(ns, *(uint32_t*)path);
		if (!obj)
		{
			if (simple)
				return get_obj_recursive(ns->parent, *(uint32_t*)path);
			return NULL;
		}
		path += 4;
		if (!*path)
			return obj;
		ns = obj_get_ns(obj);
		if (!ns)
		{
			TRACE("ns in non-ns obj: %s", obj_type_name(obj->type));
			return NULL;
		}
		simple = 0;
	}
}

static struct acpi_obj *register_obj(struct aml_state *state,
                                    enum acpi_obj_type type,
                                    const char *path,
                                    int external)
{
	struct acpi_ns *ns = get_ns(state, path, 1);
	if (!ns)
		return NULL;
	uint32_t name = *(uint32_t*)&path[strlen(path) - 4];
	struct acpi_obj *obj = ns_get_obj(ns, name);
	if (obj)
	{
		char scope[NAME_MAX_LEN + 1];
		get_ns_path(scope, ns);
		if (obj->type == type)
		{
			if (!external && !obj->external)
				TRACE("[%05zx] duplicate object definition of %s %s%s",
				      state->it.offset, obj_type_name(type),
				      scope, path);
			return obj;
		}
		if (obj->external && obj->type == ACPI_OBJ_SCOPE)
		{
			if (type == ACPI_OBJ_DEVICE
			 || type == ACPI_OBJ_POWER_RESOURCE
			 || type == ACPI_OBJ_PROCESSOR
			 || type == ACPI_OBJ_THERMAL_ZONE)
			{
				obj->type = type;
				return obj;
			}
		}
		TRACE("[%05zx] object already exists: %s%s (want type %s, existing is %s)",
		      state->it.offset, scope, path,
		      obj_type_name(type), obj_type_name(obj->type));
		return NULL;
	}
	return obj_alloc(ns, type, name);
}

static int parse_pkglength(struct aml_state *state, size_t *length)
{
	uint8_t bytes[4];
	if (getb(state, &bytes[0]))
	{
		TRACE("length eof");
		return 1;
	}
	switch (bytes[0] & 0xC0)
	{
		case 0x00:
			*length = bytes[0];
			return 0;
		case 0x40:
			if (getb(state, &bytes[1]))
			{
				TRACE("length 1 eof");
				return 1;
			}
			*length = (bytes[0] & 0x0F)
			        | (bytes[1] << 4);
			return 0;
		case 0x80:
			if (getb(state, &bytes[1])
			 || getb(state, &bytes[2]))
			{
				TRACE("length 2 eof");
				return 1;
			}
			*length = (bytes[0] & 0x0F)
			        | (bytes[1] << 4)
			        | (bytes[2] << 12);
			return 0;
		case 0xC0:
			if (getb(state, &bytes[1])
			 || getb(state, &bytes[2])
			 || getb(state, &bytes[3]))
			{
				TRACE("length 3 eof");
				return 1;
			}
			*length = (bytes[0] & 0x0F)
			        | (bytes[1] << 4)
			        | (bytes[2] << 12)
			        | (bytes[3] << 20);
			return 0;
	}
	return 1;
}

static int parse_nameseg(struct aml_state *state, char *nameseg)
{
	uint8_t chars[4];
	if (gets(state, chars, 4))
	{
		TRACE("nameseg eof");
		return 1;
	}
	if (!isupper(chars[0])
	 && chars[0] != '_')
	{
		TRACE("invalid leadnamechar: %02" PRIx8, chars[0]);
		return 1;
	}
	*(nameseg++) = chars[0];
	for (size_t n = 1; n < 4; ++n)
	{
		if (!isupper(chars[n])
		 && !isdigit(chars[n])
		 && chars[n] != '_')
		{
			TRACE("invalid namechar %zu: %02" PRIx8,
			      n, chars[n]);
			return 1;
		}
		*(nameseg++) = chars[n];
	}
	return 0;
}

static int parse_namestring(struct aml_state *state, char *name)
{
	uint8_t c;
	if (getb(state, &c))
	{
		TRACE("namepath eof");
		return 1;
	}
	if (c == '\\')
	{
		*name = '\\';
		name++;
		if (getb(state, &c))
		{
			TRACE("rootpath eof");
			return 1;
		}
	}
	else if (c == '^')
	{
		do
		{
			*name = '^';
			name++;
			if (getb(state, &c))
			{
				TRACE("prefixpath eof");
				return 1;
			}
		} while (c == '^');
	}
	if (!c)
	{
		name[0] = '_';
		name[1] = '_';
		name[2] = '_';
		name[3] = '_';
		name[4] = '\0';
		return 0;
	}
	uint8_t count;
	if (c == 0x2E)
	{
		count = 2;
	}
	else if (c == 0x2F)
	{
		if (getb(state, &count))
		{
			TRACE("multinamepath eof");
			return 1;
		}
	}
	else
	{
		count = 1;
		if (ungetb(state))
		{
			TRACE("namestring ungetb failed");
			return 1;
		}
	}
	for (size_t i = 0; i < count; ++i)
	{
		if (parse_nameseg(state, name))
			return 1;
		name += 4;
	}
	*name = '\0';
	return 0;
}

static int parse_termlist(struct aml_state *state, size_t size);
static int parse_supername(struct aml_state *state);
static int parse_termarg(struct aml_state *state);
static int parse_target(struct aml_state *state);
static int parse_datarefobject(struct aml_state *state, struct acpi_data **data);
static int parse_packageelementlist(struct aml_state *state, size_t size,
                                    struct acpi_data_head *elements);

static int parse_reservedfield(struct aml_state *state)
{
	size_t length;
	if (parse_pkglength(state, &length))
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
	{
		int prev_inline;
		inline_push(state, "ReservedField", &prev_inline);
		print_length(state, length);
		inline_pop(state, prev_inline);
	}
	return 0;
}

static int parse_accessfield(struct aml_state *state)
{
	uint8_t access_type;
	if (getb(state, &access_type))
	{
		TRACE("access type eof");
		return 1;
	}
	uint8_t access_attrib;
	if (getb(state, &access_attrib))
	{
		TRACE("access attrib eof");
		return 1;
	}
	if (state->flags & AML_STATE_F_PRINT)
	{
		int prev_inline;
		inline_push(state, "AccessField", &prev_inline);
		print_x8(state, access_type);
		inline_sep(state);
		print_x8(state, access_attrib);
		inline_pop(state, prev_inline);
	}
	return 0;
}

static int parse_connectfield(struct aml_state *state)
{
	(void)state;
	/* XXX */
	TRACE("unhandled connectfield");
	return 1;
}

static int parse_extendedaccessfield(struct aml_state *state)
{
	(void)state;
	/* XXX */
	TRACE("unhandled extendedaccessfield");
	return 1;
}

static int parse_namedfield(struct aml_state *state)
{
	if (ungetb(state))
	{
		TRACE("namedfield ungetb failed");
		return 1;
	}
	char name[4 + 1];
	if (parse_nameseg(state, name))
		return 1;
	name[4] = '\0';
	size_t length;
	if (parse_pkglength(state, &length))
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
	{
		int prev_inline;
		inline_push(state, "NamedField", &prev_inline);
		print_path(state->uio, name);
		inline_sep(state);
		print_length(state, length);
		inline_pop(state, prev_inline);
	}
	if (state->flags & AML_STATE_F_PARSE)
	{
		/*struct acpi_obj *object = register_obj(state,
		                                      ACPI_OBJ_NAMED_FIELD,
		                                      name);
		if (!object)
			return 1;*/
	}
	return 0;
}

static int parse_fieldelement(struct aml_state *state, uint8_t opcode)
{
	switch (opcode)
	{
		case 0x00:
			return parse_reservedfield(state);
		case 0x01:
			return parse_accessfield(state);
		case 0x02:
			return parse_connectfield(state);
		case 0x03:
			return parse_extendedaccessfield(state);
		default:
			return parse_namedfield(state);
	}
}

static int parse_fieldlist(struct aml_state *state, size_t size)
{
	size_t next_size = state->it.remaining - size;
	state->it.remaining = size;
	if (state->flags & AML_STATE_F_PRINT)
		indent_inc(state);
	while (1)
	{
		uint8_t opcode;
		if (getb(state, &opcode))
			break;
		if (parse_fieldelement(state, opcode))
			return 1;
	}
	if (state->flags & AML_STATE_F_PRINT)
		indent_dec(state);
	state->it.remaining = next_size;
	return 0;
}

#define PARSE_INTCONST(size, name, type, get, str) \
static int parse_##name##const(struct aml_state *state, struct acpi_data **datap) \
{ \
	uint##size##_t v; \
	if (get(state, &v)) \
	{ \
		TRACE(str " eof"); \
		return 1; \
	} \
	if (state->flags & AML_STATE_F_PRINT) \
	{ \
		if (state->display_inline) \
		{ \
			print_x##size(state, v); \
		} \
		else \
		{ \
			int prev_inline; \
			inline_push(state, str, &prev_inline); \
			print_x##size(state, v); \
			inline_pop(state, prev_inline); \
		} \
	} \
	if (state->flags & AML_STATE_F_PARSE) \
	{ \
		struct acpi_data *data = data_alloc(ACPI_DATA_##type); \
		if (!data) \
			return 1; \
		data->name.value = v; \
		if (datap) /* XXX */ \
			*datap = data; \
	} \
	return 0; \
}

PARSE_INTCONST(8, byte, BYTE, getb, "ByteConst");
PARSE_INTCONST(16, word, WORD, getw, "WordConst");
PARSE_INTCONST(32, dword, DWORD, getd, "DWordConst");
PARSE_INTCONST(64, qword, QWORD, getq, "QWordConst");

static int parse_string(struct aml_state *state, uint8_t opcode,
                        struct acpi_data **datap)
{
	char s[256];
	size_t n = 0;
	do
	{
		if (n == sizeof(s))
		{
			TRACE("string too long");
			return 1;
		}
		if (getb(state, &opcode))
		{
			TRACE("string eof");
			return 1;
		}
		s[n++] = opcode;
	} while (opcode);
	if (state->flags & AML_STATE_F_PRINT)
	{
		int prev_inline;
		inline_push(state, "String", &prev_inline);
		print_s(state, s);
		inline_pop(state, prev_inline);
	}
	if (state->flags & AML_STATE_F_PARSE)
	{
		struct acpi_data *data = data_alloc(ACPI_DATA_STRING);
		if (!data)
			return 1;
		data->string.len = n - 1;
		data->string.data = malloc(n, 0);
		if (!data->string.data)
		{
			TRACE("string allocation failed");
			data_free(data);
			return 1;
		}
		memcpy(data->string.data, s, n);
		if (datap) /* XXX */
			*datap = data;
	}
	return 0;
}

static int parse_zeroop(struct aml_state *state, struct acpi_data **datap)
{
	if (state->flags & AML_STATE_F_PRINT)
	{
		if (state->display_inline)
		{
			uprintf(state->uio, "0");
		}
		else
		{
			print_indent(state);
			uprintf(state->uio, "Zero\n");
		}
	}
	if (state->flags & AML_STATE_F_PARSE)
	{
		struct acpi_data *data = data_alloc(ACPI_DATA_ZERO);
		if (!data)
			return 1;
		if (datap) /* XXX */
			*datap = data;
	}
	return 0;
}

static int parse_oneop(struct aml_state *state, struct acpi_data **datap)
{
	if (state->flags & AML_STATE_F_PRINT)
	{
		if (state->display_inline)
		{
			uprintf(state->uio, "1");
		}
		else
		{
			print_indent(state);
			uprintf(state->uio, "One\n");
		}
	}
	if (state->flags & AML_STATE_F_PARSE)
	{
		struct acpi_data *data = data_alloc(ACPI_DATA_ONE);
		if (!data)
			return 1;
		if (datap) /* XXX */
			*datap = data;
	}
	return 0;
}

static int parse_onesop(struct aml_state *state, struct acpi_data **datap)
{
	if (state->flags & AML_STATE_F_PRINT)
	{
		if (state->display_inline)
		{
			uprintf(state->uio, "-1");
		}
		else
		{
			print_indent(state);
			uprintf(state->uio, "Ones\n");
		}
	}
	if (state->flags & AML_STATE_F_PARSE)
	{
		struct acpi_data *data = data_alloc(ACPI_DATA_ONES);
		if (!data)
			return 1;
		if (datap) /* XXX */
			*datap = data;
	}
	return 0;
}

static int parse_notifyop(struct aml_state *state)
{
	int prev_inline = 0;
	if (state->flags & AML_STATE_F_PRINT)
		inline_push(state, "Notify", &prev_inline);
	if (parse_supername(state)) /* NotifyObject */
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_sep(state);
	if (parse_termarg(state)) /* NotifyValue */
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_pop(state, prev_inline);
	return 0;
}

static int parse_sizeofop(struct aml_state *state)
{
	int prev_inline = 0;
	if (state->flags & AML_STATE_F_PRINT)
		inline_push(state, "SizeOf", &prev_inline);
	if (parse_supername(state))
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_pop(state, prev_inline);
	return 0;
}

static int parse_argobj(struct aml_state *state, uint8_t opcode)
{
	if (state->flags & AML_STATE_F_PRINT)
	{
		if (state->display_inline)
		{
			uprintf(state->uio, "Arg%" PRIu8, opcode - 0x68);
		}
		else
		{
			print_indent(state);
			uprintf(state->uio, "Arg%" PRIu8 "\n", opcode - 0x68);
		}
	}
	return 0;
}

static int parse_localobj(struct aml_state *state, uint8_t opcode)
{
	if (state->flags & AML_STATE_F_PRINT)
	{
		if (state->display_inline)
		{
			uprintf(state->uio, "Local%" PRIu8, opcode - 0x60);
		}
		else
		{
			print_indent(state);
			uprintf(state->uio, "Local%" PRIu8 "\n", opcode - 0x60);
		}
	}
	return 0;
}

static int parse_findsetleftbitop(struct aml_state *state)
{
	int prev_inline = 0;
	if (state->flags & AML_STATE_F_PRINT)
		inline_push(state, "FindSetLeftBit", &prev_inline);
	if (parse_termarg(state)) /* Operand */
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_sep(state);
	if (parse_target(state))
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_pop(state, prev_inline);
	return 0;
}

static int parse_findsetrightbitop(struct aml_state *state)
{
	int prev_inline = 0;
	if (state->flags & AML_STATE_F_PRINT)
		inline_push(state, "FindSetLeftBit", &prev_inline);
	if (parse_termarg(state)) /* Operand */
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_sep(state);
	if (parse_target(state))
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_pop(state, prev_inline);
	return 0;
}

static int parse_derefofop(struct aml_state *state)
{
	int prev_inline = 0;
	if (state->flags & AML_STATE_F_PRINT)
		inline_push(state, "DerefOf", &prev_inline);
	if (parse_termarg(state)) /* ObjReference */
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_pop(state, prev_inline);
	return 0;
}

static int parse_concatresop(struct aml_state *state)
{
	int prev_inline = 0;
	if (state->flags & AML_STATE_F_PRINT)
		inline_push(state, "ConcatResOf", &prev_inline);
	if (parse_termarg(state)) /* BufData */
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_sep(state);
	if (parse_termarg(state)) /* BufData */
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_sep(state);
	if (parse_target(state))
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_pop(state, prev_inline);
	return 0;
}

static int parse_indexop(struct aml_state *state)
{
	int prev_inline = 0;
	if (state->flags & AML_STATE_F_PRINT)
		inline_push(state, "Index", &prev_inline);
	if (parse_termarg(state)) /* BuffPkgStrObj */
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_sep(state);
	if (parse_termarg(state)) /* IndexValue */
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_sep(state);
	if (parse_target(state))
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_pop(state, prev_inline);
	return 0;
}

static int parse_matchop(struct aml_state *state)
{
	int prev_inline = 0;
	if (state->flags & AML_STATE_F_PRINT)
		inline_push(state, "Match", &prev_inline);
	if (parse_termarg(state)) /* SearchPkg */
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_sep(state);
	uint8_t opcode1;
	if (getb(state, &opcode1))
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_sep(state);
	if (parse_termarg(state)) /* Operand */
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_sep(state);
	uint8_t opcode2;
	if (getb(state, &opcode2))
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_sep(state);
	if (parse_termarg(state)) /* Operand */
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_sep(state);
	if (parse_termarg(state)) /* StartIndex */
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_pop(state, prev_inline);
	return 0;
}

static int parse_objecttypeop(struct aml_state *state)
{
	int prev_inline = 0;
	if (state->flags & AML_STATE_F_PRINT)
		inline_push(state, "ObjectType", &prev_inline);
	if (parse_supername(state))
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_pop(state, prev_inline);
	return 0;
}

static int parse_simplename(struct aml_state *state)
{
	if (ungetb(state))
	{
		TRACE("simplename ungetb");
		return 1;
	}
	char name[NAME_MAX_LEN + 1];
	if (parse_namestring(state, name))
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
	{
		int prev_inline;
		inline_push(state, "NameString", &prev_inline);
		print_path(state->uio, name);
		inline_pop(state, prev_inline);
	}
	return 0;
}

static int parse_methodinvocation(struct aml_state *state)
{
	if (ungetb(state))
	{
		TRACE("methodinvocation ungetb");
		return 1;
	}
	char name[NAME_MAX_LEN + 1];
	if (parse_namestring(state, name))
		return 1;
	int prev_inline = 0;
	if (state->flags & AML_STATE_F_PRINT)
	{
		if (!state->display_inline)
			print_indent(state);
		print_path(state->uio, name);
	}
	struct acpi_obj *obj = get_obj(state, name);
	if (obj && obj->type == ACPI_OBJ_METHOD)
	{
		if (state->flags & AML_STATE_F_PRINT)
		{
			uprintf(state->uio, "(");
			prev_inline = state->display_inline;
			state->display_inline = 1;
		}
		for (size_t i = 0; i < (obj->method.flags & 0x7); ++i)
		{
			if (state->flags & AML_STATE_F_PRINT)
			{
				if (i)
					inline_sep(state);
			}
			if (parse_termarg(state))
				return 1;
		}
		if (state->flags & AML_STATE_F_PRINT)
			inline_pop(state, prev_inline);
	}
	return 0;
}

static int parse_scopeop(struct aml_state *state)
{
	size_t org_size = state->it.remaining;
	size_t length;
	if (parse_pkglength(state, &length))
		return 1;
	if (length > org_size)
	{
		TRACE("length too big (%zu > %zu)", length, org_size);
		return 1;
	}
	char name[NAME_MAX_LEN + 1];
	if (parse_namestring(state, name))
		return 1;
	int prev_inline = 0;
	if (state->flags & AML_STATE_F_PRINT)
	{
		inline_push(state, "Scope", &prev_inline);
		print_path(state->uio, name);
		inline_pop(state, prev_inline);
	}
	struct acpi_ns *prev_ns = state->cur_ns;
	struct acpi_obj *obj = get_obj(state, name);
	if (!obj)
	{
		if (state->flags & AML_STATE_F_PARSE)
		{
			obj = register_obj(state, ACPI_OBJ_SCOPE, name, 0);
			if (!obj)
				return 1;
			ns_init(&obj->scope.ns, prev_ns, obj);
			state->cur_ns = &obj->scope.ns;
		}
	}
	else
	{
		state->cur_ns = obj_get_ns(obj);
		if (!state->cur_ns)
		{
			state->cur_ns = prev_ns;
			TRACE("already existing non-namespaced object %s", name);
			return 1;
		}
	}
	int ret = parse_termlist(state, length - (org_size - state->it.remaining));
	state->cur_ns = prev_ns;
	return ret;
}

static int parse_aliasop(struct aml_state *state)
{
	char name[NAME_MAX_LEN + 1];
	if (parse_namestring(state, name))
		return 1;
	char alias[NAME_MAX_LEN + 1];
	if (parse_namestring(state, alias))
		return 1;
	int prev_inline = 0;
	if (state->flags & AML_STATE_F_PRINT)
	{
		inline_push(state, "Alias", &prev_inline);
		print_path(state->uio, name);
		inline_sep(state);
		print_path(state->uio, alias);
		inline_pop(state, prev_inline);
	}
	if (state->flags & AML_STATE_F_PARSE)
	{
		struct acpi_obj *obj = register_obj(state, ACPI_OBJ_ALIAS,
		                                    alias, 0);
		if (!obj)
			return 1;
	}
	return 0;
}

static int parse_nameop(struct aml_state *state)
{
	char name[NAME_MAX_LEN + 1];
	if (parse_namestring(state, name))
		return 1;
	int prev_inline = 0;
	if (state->flags & AML_STATE_F_PRINT)
	{
		inline_push(state, "Name", &prev_inline);
		print_path(state->uio, name);
		inline_sep(state);
	}
	struct acpi_data *data = NULL;
	if (parse_datarefobject(state, &data))
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_pop(state, prev_inline);
	if (state->flags & AML_STATE_F_PARSE)
	{
		struct acpi_obj *obj = register_obj(state, ACPI_OBJ_NAME,
		                                    name, 0);
		if (!obj)
		{
			data_free(data);
			return 1;
		}
		obj->namedef.data = data;
	}
	return 0;
}

static const char *region_space_str(uint8_t v)
{
	static const char *names[] =
	{
		"SystemMemory",
		"SystemIO",
		"PCI_Config",
		"EmbeddedControl",
		"SMBus",
		"System CMOS",
		"PciBarTarget",
		"IPMI",
		"GeneralPurposeIO",
		"GenericSerialBus",
		"PCC",
	};
	if (v >= sizeof(names) / sizeof(*names))
		return "EOM";
	return names[v];
}

static int parse_opregionop(struct aml_state *state)
{
	char name[NAME_MAX_LEN + 1];
	if (parse_namestring(state, name))
		return 1;
	uint8_t region_space;
	if (getb(state, &region_space))
	{
		TRACE("region space eof");
		return 1;
	}
	int prev_inline = 0;
	if (state->flags & AML_STATE_F_PRINT)
	{
		inline_push(state, "OperationRegion", &prev_inline);
		print_path(state->uio, name);
		inline_sep(state);
		print_s(state, region_space_str(region_space));
		inline_sep(state);
	}
	if (parse_termarg(state)) /* Offset */
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_sep(state);
	if (parse_termarg(state)) /* Length */
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_pop(state, prev_inline);
	if (state->flags & AML_STATE_F_PARSE)
	{
		struct acpi_obj *obj = register_obj(state,
		                                    ACPI_OBJ_OPERATION_REGION,
		                                    name, 0);
		if (!obj)
			return 1;
	}
	return 0;
}

static int parse_mutexop(struct aml_state *state)
{
	int prev_inline = 0;
	if (state->flags & AML_STATE_F_PRINT)
		inline_push(state, "Mutex", &prev_inline);
	char name[NAME_MAX_LEN + 1];
	if (parse_namestring(state, name))
		return 1;
	uint8_t sync_flags;
	if (getb(state, &sync_flags))
	{
		TRACE("sync flags eof");
		return 1;
	}
	if (state->flags & AML_STATE_F_PRINT)
	{
		print_path(state->uio, name);
		inline_sep(state);
		print_x8(state, sync_flags);
		inline_pop(state, prev_inline);
	}
	if (state->flags & AML_STATE_F_PARSE)
	{
		struct acpi_obj *obj = register_obj(state, ACPI_OBJ_MUTEX,
		                                    name, 0);
		if (!obj)
			return 1;
	}
	return 0;
}

static int parse_eventop(struct aml_state *state)
{
	int prev_inline = 0;
	if (state->flags & AML_STATE_F_PRINT)
		inline_push(state, "Event", &prev_inline);
	char name[NAME_MAX_LEN + 1];
	if (parse_namestring(state, name))
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
	{
		print_path(state->uio, name);
		inline_pop(state, prev_inline);
	}
	if (state->flags & AML_STATE_F_PARSE)
	{
		struct acpi_obj *obj = register_obj(state, ACPI_OBJ_EVENT,
		                                    name, 0);
		if (!obj)
			return 1;
	}
	return 0;
}

static int parse_createfieldop(struct aml_state *state)
{
	int prev_inline = 0;
	if (state->flags & AML_STATE_F_PRINT)
		inline_push(state, "CreateField", &prev_inline);
	if (parse_termarg(state)) /* SourceBuff */
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_sep(state);
	if (parse_termarg(state)) /* BitIndex */
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_sep(state);
	if (parse_termarg(state)) /* NumBits */
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_sep(state);
	char name[NAME_MAX_LEN + 1];
	if (parse_namestring(state, name))
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
	{
		print_path(state->uio, name);
		inline_pop(state, prev_inline);
	}
	return 0;
}

static int parse_acquireop(struct aml_state *state)
{
	int prev_inline = 0;
	if (state->flags & AML_STATE_F_PRINT)
		inline_push(state, "Acquire", &prev_inline);
	if (parse_supername(state)) /* MutexObject */
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_sep(state);
	uint16_t v;
	if (getw(state, &v))
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
	{
		print_x16(state, v);
		inline_pop(state, prev_inline);
	}
	return 0;
}

static int parse_releaseop(struct aml_state *state)
{
	int prev_inline = 0;
	if (state->flags & AML_STATE_F_PRINT)
		inline_push(state, "Release", &prev_inline);
	if (parse_supername(state)) /* MutexObject */
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_pop(state, prev_inline);
	return 0;
}

static int parse_frombcdop(struct aml_state *state)
{
	int prev_inline = 0;
	if (state->flags & AML_STATE_F_PRINT)
		inline_push(state, "FromBCD", &prev_inline);
	if (parse_termarg(state)) /* BCDValue */
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_sep(state);
	if (parse_target(state))
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_pop(state, prev_inline);
	return 0;
}

static int parse_tobcdop(struct aml_state *state)
{
	int prev_inline = 0;
	if (state->flags & AML_STATE_F_PRINT)
		inline_push(state, "ToBCD", &prev_inline);
	if (parse_termarg(state)) /* Operand */
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_sep(state);
	if (parse_target(state))
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_pop(state, prev_inline);
	return 0;
}

static int parse_stallop(struct aml_state *state)
{
	int prev_inline = 0;
	if (state->flags & AML_STATE_F_PRINT)
		inline_push(state, "Stall", &prev_inline);
	if (parse_termarg(state)) /* UsecTime */
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_pop(state, prev_inline);
	return 0;
}

static int parse_sleepop(struct aml_state *state)
{
	int prev_inline = 0;
	if (state->flags & AML_STATE_F_PRINT)
		inline_push(state, "Sleep", &prev_inline);
	if (parse_termarg(state)) /* MsecTime */
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_pop(state, prev_inline);
	return 0;
}

static int parse_timerop(struct aml_state *state)
{
	int prev_inline = 0;
	if (state->flags & AML_STATE_F_PRINT)
		inline_push(state, "Timer", &prev_inline);
	if (state->flags & AML_STATE_F_PRINT)
		inline_pop(state, prev_inline);
	return 0;
}

static int parse_fieldop(struct aml_state *state)
{
	size_t org_size = state->it.remaining;
	size_t length;
	if (parse_pkglength(state, &length))
		return 1;
	if (length > org_size)
	{
		TRACE("length too big (%zu > %zu)", length, org_size);
		return 1;
	}
	char name[NAME_MAX_LEN + 1];
	if (parse_namestring(state, name))
		return 1;
	uint8_t field_flags;
	if (getb(state, &field_flags))
	{
		TRACE("field flags eof");
		return 1;
	}
	if (state->flags & AML_STATE_F_PRINT)
	{
		int prev_inline;
		inline_push(state, "Field", &prev_inline);
		print_path(state->uio, name);
		inline_sep(state);
		print_x8(state, field_flags);
		inline_pop(state, prev_inline);
	}
	/* XXX set fields part of OperationRegion */
	return parse_fieldlist(state, length - (org_size - state->it.remaining));
}

static int parse_deviceop(struct aml_state *state)
{
	size_t org_size = state->it.remaining;
	size_t length;
	if (parse_pkglength(state, &length))
		return 1;
	if (length > org_size)
	{
		TRACE("length too big (%zu > %zu)", length, org_size);
		return 1;
	}
	char name[NAME_MAX_LEN + 1];
	if (parse_namestring(state, name))
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
	{
		int prev_inline;
		inline_push(state, "Device", &prev_inline);
		print_path(state->uio, name);
		inline_pop(state, prev_inline);
	}
	struct acpi_ns *prev_ns = state->cur_ns;
	if (state->flags & AML_STATE_F_PARSE)
	{
		struct acpi_obj *obj = register_obj(state, ACPI_OBJ_DEVICE,
		                                    name, 0);
		if (!obj)
			return 1;
		ns_init(&obj->device.ns, prev_ns, obj);
		state->cur_ns = &obj->device.ns;
	}
	else
	{
		struct acpi_obj *obj = get_obj(state, name);
		if (obj && obj->type == ACPI_OBJ_DEVICE)
			state->cur_ns = &obj->device.ns;
	}
	int ret = parse_termlist(state, length - (org_size - state->it.remaining));
	state->cur_ns = prev_ns;
	return ret;
}

static int parse_processorop(struct aml_state *state)
{
	size_t org_size = state->it.remaining;
	size_t length;
	if (parse_pkglength(state, &length))
		return 1;
	if (length > org_size)
	{
		TRACE("length too big (%zu > %zu)", length, org_size);
		return 1;
	}
	char name[NAME_MAX_LEN + 1];
	if (parse_namestring(state, name))
		return 1;
	uint8_t procid;
	if (getb(state, &procid))
	{
		TRACE("procid eof");
		return 1;
	}
	uint32_t pblk_addr;
	if (getd(state, &pblk_addr))
	{
		TRACE("pblk addr eof");
		return 1;
	}
	uint8_t pblk_len;
	if (getb(state, &pblk_len))
	{
		TRACE("pblk len eof");
		return 1;
	}
	if (state->flags & AML_STATE_F_PRINT)
	{
		int prev_inline;
		inline_push(state, "Processor", &prev_inline);
		print_path(state->uio, name);
		inline_sep(state);
		print_x8(state, procid);
		inline_sep(state);
		print_x32(state, pblk_addr);
		inline_sep(state);
		print_x8(state, pblk_len);
		inline_pop(state, prev_inline);
	}
	struct acpi_ns *prev_ns = state->cur_ns;
	if (state->flags & AML_STATE_F_PARSE)
	{
		struct acpi_obj *obj = register_obj(state, ACPI_OBJ_PROCESSOR,
		                                    name, 0);
		if (!obj)
			return 1;
		ns_init(&obj->processor.ns, prev_ns, obj);
		state->cur_ns = &obj->processor.ns;
	}
	else
	{
		struct acpi_obj *obj = get_obj(state, name);
		if (obj && obj->type == ACPI_OBJ_PROCESSOR)
			state->cur_ns = &obj->processor.ns;
	}
	int ret = parse_termlist(state, length - (org_size - state->it.remaining));
	state->cur_ns = prev_ns;
	return ret;
}

static int parse_powerresop(struct aml_state *state)
{
	size_t org_size = state->it.remaining;
	size_t length;
	if (parse_pkglength(state, &length))
		return 1;
	if (length > org_size)
	{
		TRACE("length too big (%zu > %zu)", length, org_size);
		return 1;
	}
	char name[NAME_MAX_LEN + 1];
	if (parse_namestring(state, name))
		return 1;
	uint8_t system_level;
	if (getb(state, &system_level))
	{
		TRACE("system level eof");
		return 1;
	}
	uint16_t resource_order;
	if (getw(state, &resource_order))
	{
		TRACE("resource order eof");
		return 1;
	}
	if (state->flags & AML_STATE_F_PRINT)
	{
		int prev_inline;
		inline_push(state, "PowerResource", &prev_inline);
		print_path(state->uio, name);
		inline_sep(state);
		print_x8(state, system_level);
		inline_sep(state);
		print_x16(state, resource_order);
		inline_pop(state, prev_inline);
	}
	struct acpi_ns *prev_ns = state->cur_ns;
	if (state->flags & AML_STATE_F_PARSE)
	{
		struct acpi_obj *obj = register_obj(state,
		                                    ACPI_OBJ_POWER_RESOURCE,
		                                    name, 0);
		if (!obj)
			return 1;
		ns_init(&obj->power_resource.ns, prev_ns, obj);
		state->cur_ns = &obj->power_resource.ns;
	}
	else
	{
		struct acpi_obj *obj = get_obj(state, name);
		if (obj && obj->type == ACPI_OBJ_POWER_RESOURCE)
			state->cur_ns = &obj->power_resource.ns;
	}
	int ret = parse_termlist(state, length - (org_size - state->it.remaining));
	state->cur_ns = prev_ns;
	return ret;
}

static int parse_thermalzoneop(struct aml_state *state)
{
	size_t org_size = state->it.remaining;
	size_t length;
	if (parse_pkglength(state, &length))
		return 1;
	if (length > org_size)
	{
		TRACE("length too big (%zu > %zu)", length, org_size);
		return 1;
	}
	char name[NAME_MAX_LEN + 1];
	if (parse_namestring(state, name))
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
	{
		int prev_inline;
		inline_push(state, "ThermalZone", &prev_inline);
		print_path(state->uio, name);
		inline_pop(state, prev_inline);
	}
	struct acpi_ns *prev_ns = state->cur_ns;
	if (state->flags & AML_STATE_F_PARSE)
	{
		struct acpi_obj *obj = register_obj(state,
		                                    ACPI_OBJ_THERMAL_ZONE,
		                                    name, 0);
		if (!obj)
			return 1;
		ns_init(&obj->thermal_zone.ns, prev_ns, obj);
		state->cur_ns = &obj->thermal_zone.ns;
	}
	else
	{
		struct acpi_obj *obj = get_obj(state, name);
		if (obj && obj->type == ACPI_OBJ_THERMAL_ZONE)
			state->cur_ns = &obj->thermal_zone.ns;
	}
	int ret = parse_termlist(state, length - (org_size - state->it.remaining));
	state->cur_ns = prev_ns;
	return ret;
}

static int parse_indexfieldop(struct aml_state *state)
{
	size_t org_size = state->it.remaining;
	size_t length;
	if (parse_pkglength(state, &length))
		return 1;
	if (length > org_size)
	{
		TRACE("length too big (%zu > %zu)", length, org_size);
		return 1;
	}
	char index_name[NAME_MAX_LEN + 1];
	if (parse_namestring(state, index_name))
		return 1;
	char data_name[NAME_MAX_LEN + 1];
	if (parse_namestring(state, data_name))
		return 1;
	uint8_t field_flags;
	if (getb(state, &field_flags))
	{
		TRACE("field flags eof");
		return 1;
	}
	if (state->flags & AML_STATE_F_PRINT)
	{
		int prev_inline;
		inline_push(state, "IndexField", &prev_inline);
		print_path(state->uio, index_name);
		inline_sep(state);
		print_path(state->uio, data_name);
		inline_sep(state);
		print_x8(state, field_flags);
		inline_pop(state, prev_inline);
	}
	if (state->flags & AML_STATE_F_PARSE)
	{
		struct acpi_obj *obj = register_obj(state,
		                                    ACPI_OBJ_INDEX_FIELD,
		                                    data_name, 0);
		if (!obj)
			return 1;
	}
	return parse_fieldlist(state, length - (org_size - state->it.remaining));
}

static int parse_methodop(struct aml_state *state)
{
	size_t org_size = state->it.remaining;
	size_t length;
	if (parse_pkglength(state, &length))
		return 1;
	if (length > org_size)
	{
		TRACE("length too big (%zu > %zu)", length, org_size);
		return 1;
	}
	char name[NAME_MAX_LEN + 1];
	if (parse_namestring(state, name))
		return 1;
	uint8_t method_flags;
	if (getb(state, &method_flags))
	{
		TRACE("method flags eof");
		return 1;
	}
	if (state->flags & AML_STATE_F_PRINT)
	{
		int prev_inline;
		inline_push(state, "Method", &prev_inline);
		print_path(state->uio, name);
		inline_sep(state);
		print_x8(state, method_flags);
		inline_pop(state, prev_inline);
	}
	if (state->flags & AML_STATE_F_PARSE)
	{
		struct acpi_obj *obj = register_obj(state,
		                                    ACPI_OBJ_METHOD,
		                                    name, 0);
		if (!obj)
			return 1;
		obj->method.length = length - (org_size - state->it.remaining);
		obj->method.offset = state->it.offset;
		obj->method.flags = method_flags;
	}
	if (state->flags & AML_STATE_F_PRINT)
	{
		struct aml_state meth_st;
		meth_st.uio = state->uio;
		meth_st.it.ptr = state->it.ptr;
		meth_st.it.offset = 0;
		meth_st.it.remaining = length - (org_size - state->it.remaining);
		meth_st.indent = state->indent;
		meth_st.display_inline = 0;
		meth_st.cur_ns = state->cur_ns;
		meth_st.root = state->root;
		meth_st.flags = AML_STATE_F_PRINT;
		parse_termlist(&meth_st, meth_st.it.remaining);
	}
	skip(state, length - (org_size - state->it.remaining));
	return 0;
}

static int parse_externalop(struct aml_state *state)
{
	char name[NAME_MAX_LEN + 1];
	if (parse_namestring(state, name))
		return 1;
	uint8_t object_type;
	if (getb(state, &object_type))
	{
		TRACE("object type eof");
		return 1;
	}
	uint8_t arguments_count;
	if (getb(state, &arguments_count))
	{
		TRACE("arguments count eof");
		return 1;
	}
	if (state->flags & AML_STATE_F_PRINT)
	{
		int prev_inline;
		inline_push(state, "External", &prev_inline);
		print_path(state->uio, name);
		inline_sep(state);
		print_x8(state, object_type);
		inline_sep(state);
		print_x8(state, arguments_count);
		inline_pop(state, prev_inline);
	}
	if (state->flags & AML_STATE_F_PARSE)
	{
		switch (object_type)
		{
			case 0:
				TRACE("unhandled external uninitialized");
				/* XXX uninitialized */
				break;
			case 1:
				TRACE("unhandled external integer");
				/* XXX integer */
				break;
			case 2:
				TRACE("unhandled external string");
				/* XXX string */
				break;
			case 3:
				TRACE("unhandled external buffer");
				/* XXX buffer */
				break;
			case 4:
				TRACE("unhandled external package");
				/* XXX package */
				break;
			case 5:
				TRACE("unhandled external field unit");
				/* XXX field unit */
				break;
			case 6:
			{
				struct acpi_obj *obj = register_obj(state, ACPI_OBJ_DEVICE, name, 1);
				if (!obj)
					return 1;
				ns_init(&obj->device.ns, state->cur_ns, obj);
				break;
			}
			case 7:
				TRACE("unhandled external event");
				/* XXX event */
				break;
			case 8:
				struct acpi_obj *obj = register_obj(state, ACPI_OBJ_METHOD, name, 1);
				if (!obj)
					return 1;
				obj->method.length = 0;
				obj->method.offset = 0;
				obj->method.flags = arguments_count;
				break;
			case 9:
				TRACE("unhandled external mutex");
				/* XXX mutex */
				break;
			case 10:
				TRACE("unhandled external region");
				/* XXX operation region */
				break;
			case 11:
				TRACE("unhandled external power resource");
				/* XXX power resource */
				break;
			case 12:
				TRACE("unhandled external processor");
				/* XXX processor */
				break;
			case 13:
				TRACE("unhandled external thermal zone");
				/* XXX thermal zone */
				break;
			case 14:
				TRACE("unhandled external buffer field");
				/* XXX buffer field */
				break;
			case 16:
				TRACE("unhandled external debug");
				/* XXX debug */
				break;
			default:
				TRACE("unknown external type: %u", object_type);
				break;
		}
	}
	return 0;
}

static int parse_debugobj(struct aml_state *state)
{
	if (state->flags & AML_STATE_F_PRINT)
	{
		int prev_inline;
		inline_push(state, "DebugObj", &prev_inline);
		inline_pop(state, prev_inline);
	}
	return 0;
}

static int parse_createdwordfield(struct aml_state *state)
{
	int prev_inline = 0;
	if (state->flags & AML_STATE_F_PRINT)
		inline_push(state, "CreateDWordField", &prev_inline);
	if (parse_termarg(state)) /* SourceBuff */
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_sep(state);
	if (parse_termarg(state)) /* ByteIndex */
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_sep(state);
	char name[NAME_MAX_LEN + 1];
	if (parse_namestring(state, name))
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
	{
		print_path(state->uio, name);
		inline_pop(state, prev_inline);
	}
	if (state->flags & AML_STATE_F_PARSE)
	{
		struct acpi_obj *obj = register_obj(state, ACPI_OBJ_DWORD_FIELD,
		                                    name, 0);
		if (!obj)
			return 1;
	}
	return 0;
}

static int parse_createwordfield(struct aml_state *state)
{
	int prev_inline = 0;
	if (state->flags & AML_STATE_F_PRINT)
		inline_push(state, "CreateWordField", &prev_inline);
	if (parse_termarg(state)) /* SourceBuff */
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_sep(state);
	if (parse_termarg(state)) /* ByteIndex */
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
	inline_sep(state);
	char name[NAME_MAX_LEN + 1];
	if (parse_namestring(state, name))
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
	{
		print_path(state->uio, name);
		inline_pop(state, prev_inline);
	}
	if (state->flags & AML_STATE_F_PARSE)
	{
		struct acpi_obj *obj = register_obj(state, ACPI_OBJ_WORD_FIELD,
		                                    name, 0);
		if (!obj)
			return 1;
	}
	return 0;
}

static int parse_createbytefield(struct aml_state *state)
{
	int prev_inline = 0;
	if (state->flags & AML_STATE_F_PRINT)
		inline_push(state, "CreateByteField", &prev_inline);
	if (parse_termarg(state)) /* SourceBuff */
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_sep(state);
	if (parse_termarg(state)) /* ByteIndex */
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_sep(state);
	char name[NAME_MAX_LEN + 1];
	if (parse_namestring(state, name))
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
	{
		print_path(state->uio, name);
		inline_pop(state, prev_inline);
	}
	if (state->flags & AML_STATE_F_PARSE)
	{
		struct acpi_obj *obj = register_obj(state, ACPI_OBJ_BYTE_FIELD,
		                                    name, 0);
		if (!obj)
			return 1;
	}
	return 0;
}

static int parse_createbitfield(struct aml_state *state)
{
	int prev_inline = 0;
	if (state->flags & AML_STATE_F_PRINT)
		inline_push(state, "CreateBitField", &prev_inline);
	if (parse_termarg(state)) /* SourceBuff */
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_sep(state);
	if (parse_termarg(state)) /* BitIndex */
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_sep(state);
	char name[NAME_MAX_LEN + 1];
	if (parse_namestring(state, name))
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
	{
		print_path(state->uio, name);
		inline_pop(state, prev_inline);
	}
	if (state->flags & AML_STATE_F_PARSE)
	{
		struct acpi_obj *obj = register_obj(state, ACPI_OBJ_BIT_FIELD,
		                                    name, 0);
		if (!obj)
			return 1;
	}
	return 0;
}

static int parse_createqwordfield(struct aml_state *state)
{
	int prev_inline = 0;
	if (state->flags & AML_STATE_F_PRINT)
		inline_push(state, "CreateQWordField", &prev_inline);
	if (parse_termarg(state)) /* SourceBuff */
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_sep(state);
	if (parse_termarg(state)) /* ByteIndex */
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_sep(state);
	char name[NAME_MAX_LEN + 1];
	if (parse_namestring(state, name))
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
	{
		print_path(state->uio, name);
		inline_pop(state, prev_inline);
	}
	if (state->flags & AML_STATE_F_PARSE)
	{
		struct acpi_obj *obj = register_obj(state, ACPI_OBJ_QWORD_FIELD,
		                                    name, 0);
		if (!obj)
			return 1;
	}
	return 0;
}

static int parse_supername(struct aml_state *state)
{
	uint8_t opcode;
	if (getb(state, &opcode))
	{
		TRACE("supername opcode");
		return 1;
	}
	switch (opcode)
	{
		case 0x5B:
		{
			uint8_t ext_opcode;
			if (getb(state, &ext_opcode))
			{
				TRACE("supername ext opcode eof");
				return 1;
			}
			switch (ext_opcode)
			{
				case 0x31:
					return parse_debugobj(state);
				default:
					TRACE("unknown supername ext opcode: 0x%02" PRIx8,
					      ext_opcode);
					return 1;
			}
			break;
		}
		CASE_SIMPLENAME(state, opcode)
		CASE_REFERENCETYPEOPCODE(state, opcode)
		default:
			TRACE("unknown supername opcode: 0x%02" PRIx8,
			      opcode);
			return 1;
	}
}

static int parse_target(struct aml_state *state)
{
	uint8_t byte;
	if (getb(state, &byte))
	{
		TRACE("target eof");
		return 1;
	}
	if (!byte)
	{
		if (state->flags & AML_STATE_F_PRINT)
		{
			if (state->display_inline)
			{
				uprintf(state->uio, "(nil)");
			}
			else
			{
				print_indent(state);
				uprintf(state->uio, "(nil)\n");
			}
		}
		return 0;
	}
	if (ungetb(state))
	{
		TRACE("target ungetb");
		return 1;
	}
	return parse_supername(state);
}

static int parse_condrefofop(struct aml_state *state)
{
	int prev_inline = 0;
	if (state->flags & AML_STATE_F_PRINT)
		inline_push(state, "CondRefOf", &prev_inline);
	if (parse_supername(state))
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_sep(state);
	if (parse_target(state))
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_pop(state, prev_inline);
	return 0;
}

static int parse_todecimalstringop(struct aml_state *state)
{
	int prev_inline = 0;
	if (state->flags & AML_STATE_F_PRINT)
		inline_push(state, "ToDecimalString", &prev_inline);
	if (parse_termarg(state)) /* Operand */
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_sep(state);
	if (parse_target(state))
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_pop(state, prev_inline);
	return 0;
}

static int parse_tohexstringop(struct aml_state *state)
{
	int prev_inline = 0;
	if (state->flags & AML_STATE_F_PRINT)
		inline_push(state, "ToHexString", &prev_inline);
	if (parse_termarg(state)) /* Operand */
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_sep(state);
	if (parse_target(state))
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_pop(state, prev_inline);
	return 0;
}

static int parse_tointegerop(struct aml_state *state)
{
	int prev_inline = 0;
	if (state->flags & AML_STATE_F_PRINT)
		inline_push(state, "ToInteger", &prev_inline);
	if (parse_termarg(state)) /* Operand */
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_sep(state);
	if (parse_target(state))
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_pop(state, prev_inline);
	return 0;
}

static int parse_tobufferop(struct aml_state *state)
{
	int prev_inline = 0;
	if (state->flags & AML_STATE_F_PRINT)
		inline_push(state, "ToBuffer", &prev_inline);
	if (parse_termarg(state)) /* Operand */
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_sep(state);
	if (parse_target(state))
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_pop(state, prev_inline);
	return 0;
}

static int parse_tostringop(struct aml_state *state)
{
	int prev_inline = 0;
	if (state->flags & AML_STATE_F_PRINT)
		inline_push(state, "ToString", &prev_inline);
	if (parse_termarg(state)) /* TermArg */
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_sep(state);
	if (parse_termarg(state)) /* LengthArg */
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_sep(state);
	if (parse_target(state))
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_pop(state, prev_inline);
	return 0;
}

static int parse_refofop(struct aml_state *state)
{
	int prev_inline = 0;
	if (state->flags & AML_STATE_F_PRINT)
		inline_push(state, "RefOf", &prev_inline);
	if (parse_supername(state))
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_pop(state, prev_inline);
	return 0;
}

static int parse_addop(struct aml_state *state)
{
	int prev_inline = 0;
	if (state->flags & AML_STATE_F_PRINT)
		inline_push(state, "Add", &prev_inline);
	if (parse_termarg(state)) /* Operand */
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_sep(state);
	if (parse_termarg(state)) /* Operand */
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_sep(state);
	if (parse_target(state))
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_pop(state, prev_inline);
	return 0;
}

static int parse_concatop(struct aml_state *state)
{
	int prev_inline = 0;
	if (state->flags & AML_STATE_F_PRINT)
		inline_push(state, "Concat", &prev_inline);
	if (parse_termarg(state)) /* Operand */
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_sep(state);
	if (parse_termarg(state)) /* Operand */
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_sep(state);
	if (parse_target(state))
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_pop(state, prev_inline);
	return 0;
}

static int parse_subtractop(struct aml_state *state)
{
	int prev_inline = 0;
	if (state->flags & AML_STATE_F_PRINT)
		inline_push(state, "Subtract", &prev_inline);
	if (parse_termarg(state)) /* Operand */
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_sep(state);
	if (parse_termarg(state)) /* Operand */
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_sep(state);
	if (parse_target(state))
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_pop(state, prev_inline);
	return 0;
}

#define LLOGICALOP_DEF(str, op) \
do \
{ \
	int prev_inline = 0; \
	if (state->flags & AML_STATE_F_PRINT) \
		inline_push(state, str, &prev_inline); \
	if (parse_termarg(state)) /* Operand */ \
		return 1; \
	if (state->flags & AML_STATE_F_PRINT) \
		inline_sep(state); \
	if (parse_termarg(state)) /* Operand */ \
		return 1; \
	if (state->flags & AML_STATE_F_PRINT) \
		inline_pop(state, prev_inline); \
	return 0; \
} while (0)

static int parse_landop(struct aml_state *state)
{
	LLOGICALOP_DEF("Land", &&);
}

static int parse_lorop(struct aml_state *state)
{
	LLOGICALOP_DEF("Lor", ||);
}

#define LCMPOP_DEF(str, op) \
do \
{ \
	int prev_inline = 0; \
	if (state->flags & AML_STATE_F_PRINT) \
		inline_push(state, str, &prev_inline); \
	if (parse_termarg(state)) /* Operand */ \
		return 1; \
	if (state->flags & AML_STATE_F_PRINT) \
		inline_sep(state); \
	if (parse_termarg(state)) /* Operand */ \
		return 1; \
	if (state->flags & AML_STATE_F_PRINT) \
		inline_pop(state, prev_inline); \
	return 0; \
} while (0)

static int parse_lnotequalop(struct aml_state *state)
{
	LCMPOP_DEF("LNotEqual", !=);
}

static int parse_llessequalop(struct aml_state *state)
{
	LCMPOP_DEF("LLessEqual", <=);
}

static int parse_lgreaterequalop(struct aml_state *state)
{
	LCMPOP_DEF("LGreaterEqual", >=);
}

static int parse_lequalop(struct aml_state *state)
{
	LCMPOP_DEF("LEqual", ==);
}

static int parse_lgreaterop(struct aml_state *state)
{
	LCMPOP_DEF("LGreater", >);
}

static int parse_llessop(struct aml_state *state)
{
	LCMPOP_DEF("LLess", <);
}

static int parse_lnotop(struct aml_state *state)
{
	uint8_t next_opcode;
	if (getb(state, &next_opcode))
	{
		TRACE("lnotop eof");
		return 1;
	}
	switch (next_opcode)
	{
		case 0x93:
			return parse_lnotequalop(state);
		case 0x94:
			return parse_llessequalop(state);
		case 0x95:
			return parse_lgreaterequalop(state);
		default:
			if (ungetb(state))
			{
				TRACE("lnotop ungetb");
				return 1;
			}
			break;
	}
	int prev_inline = 0;
	if (state->flags & AML_STATE_F_PRINT)
		inline_push(state, "Lnot", &prev_inline);
	if (parse_termarg(state)) /* Operand */
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_pop(state, prev_inline);
	return 0;
}

static int parse_storeop(struct aml_state *state)
{
	int prev_inline = 0;
	if (state->flags & AML_STATE_F_PRINT)
		inline_push(state, "Store", &prev_inline);
	if (parse_termarg(state))
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_sep(state);
	if (parse_supername(state))
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_pop(state, prev_inline);
	return 0;
}

static int parse_bufferop(struct aml_state *state, struct acpi_data **datap)
{
	size_t org_size = state->it.remaining;
	size_t length;
	if (parse_pkglength(state, &length))
		return 1;
	if (length > org_size)
	{
		TRACE("length too big (%zu > %zu)", length, org_size);
		return 1;
	}
	int prev_inline = 0;
	if (state->flags & AML_STATE_F_PRINT)
		inline_push(state, "Buffer", &prev_inline);
	if (parse_termarg(state)) /* BufferSize */
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_pop(state, prev_inline);
	if (state->flags & AML_STATE_F_PARSE)
	{
		struct acpi_data *data = data_alloc(ACPI_DATA_BUFFER);
		if (!data)
			return 1;
		data->buffer.size = length - (org_size - state->it.remaining);
		if (!data->buffer.size)
			return 0;
		data->buffer.data = malloc(data->buffer.size, 0);
		if (!data->buffer.data)
		{
			TRACE("buffer allocation failed");
			return 1;
		}
		if (gets(state, data->buffer.data, data->buffer.size))
		{
			TRACE("buffer read failed");
			return 1;
		}
		if (datap) /* XXX this should always be set */
			*datap = data;
	}
	else
	{
		if (state->flags & AML_STATE_F_PRINT)
		{
			size_t n = length - (org_size - state->it.remaining);
			for (size_t i = 0; i < n; ++i)
			{
				uint8_t a = 0;
				getb(state, &a);
				uprintf(state->uio, " %02x", a);
			}
		}
		else
		{
			skip(state, length - (org_size - state->it.remaining));
		}
	}
	return 0;
#if 0
	uprintf(state->uio, ": {");
	for (size_t i = 0; i < data->buffer.size;)
	{
		uint8_t v = data->buffer.data[i];
		uint8_t tag = (v >> 3) & 0xF;
		uint8_t len = (v >> 0) & 0x7;
		size_t nxt = i + len + 1;
		if (v & 0x80)
			return 0;
		if (i)
			uprintf(state->uio, ", {");
		else
			uprintf(state->uio, "{");
		if (nxt > data->buffer.size)
		{
			TRACE("invalid length");
			return 1;
		}
		switch (tag)
		{
			case 0x0:
			case 0x1:
			case 0x2:
			case 0x3:
				uprintf(state->uio, "reserved");
				break;
			case 0x4:
				uprintf(state->uio, "IRQ: ");
				if (len != 2 && len != 3)
				{
					TRACE("invalid IRQ length");
					return 1;
				}
				uprintf(state->uio, "0x%02" PRIx8 "%02" PRIx8,
				        data->buffer.data[i + 1],
				        data->buffer.data[i + 2]);
				if (len == 3)
				{
					uprintf(state->uio, " 0x%02" PRIx8,
					        data->buffer.data[i + 3]);
				}
				break;
			case 0x5:
				uprintf(state->uio, "DMA: ");
				if (len != 2)
				{
					TRACE("invalid DMA length");
					return 1;
				}
				uprintf(state->uio, "0x%02" PRIx8 " %02" PRIx8,
				        data->buffer.data[i + 1],
				        data->buffer.data[i + 2]);
				break;
			case 0x8:
				uprintf(state->uio, "PIO: ");
				if (len != 7)
				{
					TRACE("invalid PIO length");
					return 1;
				}
				uprintf(state->uio, "0x%02" PRIx8 " 0x%04" PRIx16 " - 0x%04" PRIx16 " 0x%02" PRIx8 " 0x%02" PRIx8,
				        data->buffer.data[i + 1],
				        data->buffer.data[i + 2] | ((uint16_t)data->buffer.data[i + 3] << 8),
				        data->buffer.data[i + 4] | ((uint16_t)data->buffer.data[i + 5] << 8),
				        data->buffer.data[i + 6],
				        data->buffer.data[i + 7]);
				break;
			case 0xF:
				break;
			default:
				uprintf(state->uio, "unknown");
				break;
		}
		uprintf(state->uio, "}");
		i = nxt;
	}
	uprintf(state->uio, "\n");
	return 0;
#endif
}

static int parse_varpackageop(struct aml_state *state, struct acpi_data **datap)
{
	size_t org_size = state->it.remaining;
	size_t length;
	if (parse_pkglength(state, &length))
		return 1;
	if (length > org_size)
	{
		TRACE("length too big (%zu > %zu)", length, org_size);
		return 1;
	}
	int prev_inline = 0;
	if (state->flags & AML_STATE_F_PRINT)
		inline_push(state, "VarPackage", &prev_inline);
	if (parse_termarg(state)) /* NumElements */
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_pop(state, prev_inline);
	int ret;
	if (state->flags & AML_STATE_F_PARSE)
	{
		struct acpi_data *data = data_alloc(ACPI_DATA_PACKAGE);
		if (!data)
			return 1;
		TAILQ_INIT(&data->package.elements);
		ret = parse_packageelementlist(state, length - (org_size - state->it.remaining), &data->package.elements);
		if (datap) /* XXX */
			*datap = data;
	}
	else
	{
		ret = parse_packageelementlist(state, length - (org_size - state->it.remaining), NULL);
	}
	skip(state, length - (org_size - state->it.remaining));
	return ret;
}

static int parse_packageop(struct aml_state *state, struct acpi_data **datap)
{
	size_t org_size = state->it.remaining;
	size_t length;
	if (parse_pkglength(state, &length))
		return 1;
	if (length > org_size)
	{
		TRACE("length too big (%zu > %zu)", length, org_size);
		return 1;
	}
	uint8_t num_elements;
	if (getb(state, &num_elements))
	{
		TRACE("num elements eof");
		return 1;
	}
	if (state->flags & AML_STATE_F_PRINT)
	{
		int prev_inline;
		inline_push(state, "Package", &prev_inline);
		print_x8(state, num_elements);
		prev_inline = 0;
		inline_pop(state, prev_inline);
	}
	int ret;
	if (state->flags & AML_STATE_F_PARSE)
	{
		struct acpi_data *data = data_alloc(ACPI_DATA_PACKAGE);
		if (!data)
			return 1;
		TAILQ_INIT(&data->package.elements);
		ret = parse_packageelementlist(state, length - (org_size - state->it.remaining), &data->package.elements);
		if (datap) /* XXX */
			*datap = data;
	}
	else
	{
		ret = parse_packageelementlist(state, length - (org_size - state->it.remaining), NULL);
	}
	skip(state, length - (org_size - state->it.remaining));
	return ret;
}

static int parse_whileop(struct aml_state *state)
{
	size_t org_size = state->it.remaining;
	size_t length;
	if (parse_pkglength(state, &length))
		return 1;
	if (length > org_size)
	{
		TRACE("length too big (%zu > %zu)", length, org_size);
		return 1;
	}
	int prev_inline = 0;
	if (state->flags & AML_STATE_F_PRINT)
		inline_push(state, "While", &prev_inline);
	if (parse_termarg(state)) /* Predicate */
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_pop(state, prev_inline);
	return parse_termlist(state, length - (org_size - state->it.remaining));
}

static int parse_elseop(struct aml_state *state)
{
	size_t org_size = state->it.remaining;
	size_t length;
	if (parse_pkglength(state, &length))
		return 1;
	if (length > org_size)
	{
		TRACE("length too big (%zu > %zu)", length, org_size);
		return 1;
	}
	if (state->flags & AML_STATE_F_PRINT)
	{
		int prev_inline;
		inline_push(state, "Else", &prev_inline);
		inline_pop(state, prev_inline);
	}
	return parse_termlist(state, length - (org_size - state->it.remaining));
}

static int parse_ifop(struct aml_state *state)
{
	size_t org_size = state->it.remaining;
	size_t length;
	if (parse_pkglength(state, &length))
		return 1;
	if (length > org_size)
	{
		TRACE("length too big (%zu > %zu)", length, org_size);
		return 1;
	}
	int prev_inline = 0;
	if (state->flags & AML_STATE_F_PRINT)
		inline_push(state, "If", &prev_inline);
	if (parse_termarg(state)) /* Predicate */
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_pop(state, prev_inline);
	if (parse_termlist(state, length - (org_size - state->it.remaining)))
		return 1;
	uint8_t opcode;
	if (getb(state, &opcode))
		return 0;
	if (opcode != 0xA1)
	{
		if (ungetb(state))
		{
			TRACE("elseop ungetb");
			return 1;
		}
		return 0;
	}
	return parse_elseop(state);
}

static int parse_returnop(struct aml_state *state)
{
	int prev_inline = 0;
	if (state->flags & AML_STATE_F_PRINT)
		inline_push(state, "Return", &prev_inline);
	if (parse_termarg(state)) /* ArgObject */
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_pop(state, prev_inline);
	return 0;
}

static int parse_continueop(struct aml_state *state)
{
	if (state->flags & AML_STATE_F_PRINT)
	{
		int prev_inline;
		inline_push(state, "Continue", &prev_inline);
		inline_pop(state, prev_inline);
	}
	return 0;
}

static int parse_breakop(struct aml_state *state)
{
	if (state->flags & AML_STATE_F_PRINT)
	{
		int prev_inline;
		inline_push(state, "Break", &prev_inline);
		inline_pop(state, prev_inline);
	}
	return 0;
}

static int parse_incrementop(struct aml_state *state)
{
	int prev_inline = 0;
	if (state->flags & AML_STATE_F_PRINT)
		inline_push(state, "Increment", &prev_inline);
	if (parse_supername(state))
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_pop(state, prev_inline);
	return 0;
}

static int parse_decrementop(struct aml_state *state)
{
	int prev_inline = 0;
	if (state->flags & AML_STATE_F_PRINT)
		inline_push(state, "Decrement", &prev_inline);
	if (parse_supername(state))
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_pop(state, prev_inline);
	return 0;
}

static int parse_multiplyop(struct aml_state *state)
{
	int prev_inline = 0;
	if (state->flags & AML_STATE_F_PRINT)
		inline_push(state, "Multiply", &prev_inline);
	if (parse_termarg(state)) /* Operand */
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_sep(state);
	if (parse_termarg(state)) /* Operand */
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_sep(state);
	if (parse_target(state))
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_pop(state, prev_inline);
	return 0;
}

static int parse_divideop(struct aml_state *state)
{
	int prev_inline = 0;
	if (state->flags & AML_STATE_F_PRINT)
		inline_push(state, "Divide", &prev_inline);
	if (parse_termarg(state)) /* Dividend */
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_sep(state);
	if (parse_termarg(state)) /* Divisor */
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_sep(state);
	if (parse_target(state)) /* Remainder */
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_sep(state);
	if (parse_target(state)) /* Quotient */
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_pop(state, prev_inline);
	return 0;
}

static int parse_shiftleftop(struct aml_state *state)
{
	int prev_inline = 0;
	if (state->flags & AML_STATE_F_PRINT)
		inline_push(state, "ShiftLeft", &prev_inline);
	if (parse_termarg(state)) /* Operand */
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_sep(state);
	if (parse_termarg(state)) /* ShiftCount */
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_sep(state);
	if (parse_target(state))
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_pop(state, prev_inline);
	return 0;
}

static int parse_shiftrightop(struct aml_state *state)
{
	int prev_inline = 0;
	if (state->flags & AML_STATE_F_PRINT)
		inline_push(state, "ShiftRight", &prev_inline);
	if (parse_termarg(state)) /* Operand */
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_sep(state);
	if (parse_termarg(state)) /* ShiftCount */
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_sep(state);
	if (parse_target(state))
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_pop(state, prev_inline);
	return 0;
}

#define BITOP_DEF(str, op, neg) \
do \
{ \
	int prev_inline = 0; \
	if (state->flags & AML_STATE_F_PRINT) \
		inline_push(state, str, &prev_inline); \
	if (parse_termarg(state)) /* Operand */ \
		return 1; \
	if (state->flags & AML_STATE_F_PRINT) \
		inline_sep(state); \
	if (parse_termarg(state)) /* Operand */ \
		return 1; \
	if (state->flags & AML_STATE_F_PRINT) \
		inline_sep(state); \
	if (parse_target(state)) \
		return 1; \
	if (state->flags & AML_STATE_F_PRINT) \
		inline_pop(state, prev_inline); \
	return 0; \
} while (0)

static int parse_andop(struct aml_state *state)
{
	BITOP_DEF("And", &, 0);
}

static int parse_nandop(struct aml_state *state)
{
	BITOP_DEF("Nand", &, 1);
}

static int parse_orop(struct aml_state *state)
{
	BITOP_DEF("Or", |, 0);
}

static int parse_norop(struct aml_state *state)
{
	BITOP_DEF("Nor", |, 1);
}

static int parse_xorop(struct aml_state *state)
{
	BITOP_DEF("Xor", ^, 0);
}

static int parse_notop(struct aml_state *state)
{
	int prev_inline = 0;
	if (state->flags & AML_STATE_F_PRINT)
		inline_push(state, "Not", &prev_inline);
	if (parse_termarg(state)) /* Operand */
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_sep(state);
	if (parse_target(state))
		return 1;
	if (state->flags & AML_STATE_F_PRINT)
		inline_pop(state, prev_inline);
	return 0;
}

static int parse_datarefobject(struct aml_state *state, struct acpi_data **data)
{
	uint8_t opcode;
	if (getb(state, &opcode))
	{
		TRACE("datarefobject eof");
		return 1;
	}
	switch (opcode)
	{
		CASE_DATAOBJECT(state, opcode, data)
		case 0x11:
			return parse_bufferop(state, data);
		case 0x13:
			return parse_varpackageop(state, data);
		default:
			TRACE("unknown datarefobject 0x%02" PRIx8,
			      opcode);
			return 1;
	}
	return 0;
}

static int parse_packageelement(struct aml_state *state, uint8_t opcode,
                                struct acpi_data **data)
{
	switch (opcode)
	{
		CASE_NAMESTRING(state, opcode)
		default:
			if (ungetb(state))
			{
				TRACE("packageelement ungetb");
				return 1;
			}
			return parse_datarefobject(state, data);
	}
}

static int parse_packageelementlist(struct aml_state *state, size_t size,
                                    struct acpi_data_head *elements)
{
	size_t next_size = state->it.remaining - size;
	state->it.remaining = size;
	if (state->flags & AML_STATE_F_PRINT)
		indent_inc(state);
	while (1)
	{
		uint8_t opcode;
		if (getb(state, &opcode))
			break;
		struct acpi_data *element = NULL;
		if (parse_packageelement(state, opcode, &element))
			return 1;
		if (element)
		{
			if (elements)
				TAILQ_INSERT_TAIL(elements, element, chain);
			else
				data_free(element);
		}
	}
	if (state->flags & AML_STATE_F_PRINT)
		indent_dec(state);
	state->it.remaining = next_size;
	return 0;
}

static int parse_termarg(struct aml_state *state)
{
	uint8_t opcode;
	if (getb(state, &opcode))
	{
		TRACE("termarg opcode eof");
		return 1;
	}
	switch (opcode)
	{
		CASE_COMPUTATIONALDATA(state, opcode, NULL)
		CASE_LOCALOBJ(state, opcode)
		CASE_ARGOBJ(state, opcode)
		CASE_EXPRESSION(state, opcode)
		case 0x11:
			return parse_bufferop(state, NULL);
		case 0x5B:
		{
			uint8_t ext_opcode;
			if (getb(state, &ext_opcode))
			{
				TRACE("ext opcode eof");
				return 1;
			}
			switch (ext_opcode)
			{
				case 0x12:
					return parse_condrefofop(state);
				case 0x23:
					return parse_acquireop(state);
				case 0x27:
					return parse_releaseop(state);
				case 0x28:
					return parse_frombcdop(state);
				case 0x29:
					return parse_tobcdop(state);
				case 0x33:
					return parse_timerop(state);
				case 0x81:
					return parse_fieldop(state);
				default:
					TRACE("unknown term arg ext opcode: 0x%02" PRIx8,
					      ext_opcode);
					return 1;
			}
			break;
		}
		default:
			TRACE("unknown term arg 0x%02" PRIx8, opcode);
			return 1;
	}
}

static int parse_termobj(struct aml_state *state, uint8_t opcode)
{
	switch (opcode)
	{
		case 0x1:
			/* workaround for bogus ACPI */
			return 0;
		CASE_OBJECT(state, opcode)
		CASE_EXPRESSION(state, opcode)
		CASE_STATEMENT(state, opcode)
		case 0x11:
			return parse_bufferop(state, NULL);
		case 0x14:
			return parse_methodop(state);
		case 0x5B:
		{
			uint8_t ext_opcode;
			if (getb(state, &ext_opcode))
			{
				TRACE("ext opcode eof");
				return 1;
			}
			switch (ext_opcode)
			{
				case 0x01:
					return parse_mutexop(state);
				case 0x02:
					return parse_eventop(state);
				case 0x13:
					return parse_createfieldop(state);
				case 0x21:
					return parse_stallop(state);
				case 0x22:
					return parse_sleepop(state);
				case 0x23:
					return parse_acquireop(state);
				case 0x27:
					return parse_releaseop(state);
				case 0x28:
					return parse_frombcdop(state);
				case 0x29:
					return parse_tobcdop(state);
				case 0x33:
					return parse_timerop(state);
				case 0x80:
					return parse_opregionop(state);
				case 0x81:
					return parse_fieldop(state);
				case 0x82:
					return parse_deviceop(state);
				case 0x83:
					return parse_processorop(state);
				case 0x84:
					return parse_powerresop(state);
				case 0x85:
					return parse_thermalzoneop(state);
				case 0x86:
					return parse_indexfieldop(state);
				default:
					TRACE("unknown ext opcode: 0x%02" PRIx8,
					      ext_opcode);
					return 1;
			}
			break;
		}
		default:
			TRACE("unknown opcode 0x%02" PRIx8 " @ 0x%zx",
			      opcode, state->it.offset);
			return 1;
	}
	return 0;
}

static int parse_termlist(struct aml_state *state, size_t size)
{
	size_t next_size = state->it.remaining - size;
	state->it.remaining = size;
	if (state->flags & AML_STATE_F_PRINT)
		indent_inc(state);
	while (1)
	{
		uint8_t opcode;
		if (getb(state, &opcode))
			break;
		if (parse_termobj(state, opcode))
			return 1;
	}
	if (state->flags & AML_STATE_F_PRINT)
		indent_dec(state);
	state->it.remaining = next_size;
	return 0;
}

int aml_alloc(struct aml_state **statep)
{
	struct aml_state *state = sma_alloc(&aml_state_sma, M_ZERO);
	if (!state)
		return -ENOMEM;
	state->root = obj_alloc(NULL, ACPI_OBJ_SCOPE, 0x5F5F5F5F);
	if (!state->root)
	{
		sma_free(&aml_state_sma, state);
		return -ENOMEM;
	}
	ns_init(&state->root->scope.ns, NULL, state->root);
	TAILQ_INSERT_TAIL(&state->root->scope.ns.obj_list, state->root, chain);
	state->root->ns = &state->root->scope.ns;
	*statep = state;
	return 0;
}

void aml_free(struct aml_state *state)
{
	if (!state)
		return;
	obj_free(state->root);
	sma_free(&aml_state_sma, state);
}

int aml_parse(struct aml_state *state, const uint8_t *data, size_t size)
{
	state->it.ptr = data;
	state->it.remaining = size;
	state->it.offset = 0;
	state->cur_ns = &state->root->scope.ns;
	state->flags = AML_STATE_F_PARSE;
	return parse_termlist(state, state->it.remaining);
}

int aml_exec(struct aml_state *state, struct acpi_obj *method)
{
	if (!method || method->type != ACPI_OBJ_METHOD)
		return -EINVAL;
	struct aml_state meth_st;
	meth_st.uio = state->uio;
	meth_st.it.ptr = state->it.ptr;
	meth_st.it.offset = 0;
	meth_st.it.remaining = method->method.length;
	meth_st.indent = state->indent;
	meth_st.display_inline = 0;
	meth_st.cur_ns = state->cur_ns;
	meth_st.root = state->root;
	meth_st.flags = AML_STATE_F_EXEC;
	parse_termlist(&meth_st, meth_st.it.remaining);
	return 0;
}

static void print_obj(struct uio *uio, struct acpi_obj *obj, size_t indent)
{
	uprintf(uio, "%.*s", (int)indent, tabs);
	print_name(uio, obj->name);
	uprintf(uio, ": %s", obj_type_name(obj->type));
	switch (obj->type)
	{
		case ACPI_OBJ_NAME:
			uprintf(uio, " %s",
			        obj->namedef.data
			      ? data_type_name(obj->namedef.data->type)
			      : "Empty");
			break;
		default:
			break;
	}
	if (obj->external)
		uprintf(uio, " (external)");
	uprintf(uio, "\n");
}

static void print_ns(struct uio *uio, struct aml_state *state,
                     struct acpi_ns *ns, size_t indent)
{
	uprintf(uio, "%.*s{\n", (int)indent, tabs);
	struct acpi_obj *obj;
	TAILQ_FOREACH(obj, &ns->obj_list, chain)
	{
		print_obj(uio, obj, indent + 1);
		struct acpi_ns *child = obj_get_ns(obj);
		if (child && child != ns)
			print_ns(uio, state, child, indent + 1);
	}
	uprintf(uio, "%.*s}\n", (int)indent, tabs);
}

void aml_print(struct uio *uio, struct aml_state *state)
{
	print_ns(uio, state, &state->root->scope.ns, 0);
}

void aml_print_asl(struct uio *uio, struct aml_state *state,
                   const uint8_t *data, size_t size)
{
	state->uio = uio;
	state->it.ptr = data;
	state->it.offset = 0;
	state->it.remaining = size;
	state->indent = 0;
	state->cur_ns = &state->root->scope.ns;
	state->flags = AML_STATE_F_PRINT;
	parse_termlist(state, state->it.remaining);
}

struct acpi_obj *aml_get_obj(struct aml_state *state, const char *name)
{
	return get_obj(state, name);
}

struct acpi_obj *aml_get_child(struct acpi_ns *ns, const char *name)
{
	uint32_t nameid = 0;
	if (name[0])
	{
		nameid |= (name[0] << 0);
		if (name[1])
		{
			nameid |= (name[1] << 8);
			if (name[2])
			{
				nameid |= (name[2] << 16);
				if (name[3])
					nameid |= (name[3] << 24);
				else
					nameid |= 0x5F000000;
			}
			else
			{
				nameid |= 0x5F5F0000;
			}
		}
		else
		{
			nameid |= 0x5F5F5F00;
		}
	}
	else
	{
		nameid = 0x5F5F5F5F;
	}
	return ns_get_obj(ns, htole32(nameid));
}
