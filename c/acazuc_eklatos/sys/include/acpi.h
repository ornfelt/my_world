#ifndef DEV_ACPI_H
#define DEV_ACPI_H

#include <queue.h>
#include <types.h>

enum acpi_data_type
{
	ACPI_DATA_BUFFER,
	ACPI_DATA_BYTE,
	ACPI_DATA_DWORD,
	ACPI_DATA_ONE,
	ACPI_DATA_ONES,
	ACPI_DATA_PACKAGE,
	ACPI_DATA_QWORD,
	ACPI_DATA_STRING,
	ACPI_DATA_WORD,
	ACPI_DATA_ZERO,
};

enum acpi_obj_type
{
	ACPI_OBJ_ALIAS,
	ACPI_OBJ_BIT_FIELD,
	ACPI_OBJ_BYTE_FIELD,
	ACPI_OBJ_DEVICE,
	ACPI_OBJ_DWORD_FIELD,
	ACPI_OBJ_EVENT,
	ACPI_OBJ_FIELD,
	ACPI_OBJ_INDEX_FIELD,
	ACPI_OBJ_NAMED_FIELD,
	ACPI_OBJ_METHOD,
	ACPI_OBJ_MUTEX,
	ACPI_OBJ_NAME,
	ACPI_OBJ_OPERATION_REGION,
	ACPI_OBJ_POWER_RESOURCE,
	ACPI_OBJ_PROCESSOR,
	ACPI_OBJ_QWORD_FIELD,
	ACPI_OBJ_RAW_DATA_BUFFER,
	ACPI_OBJ_SCOPE,
	ACPI_OBJ_THERMAL_ZONE,
	ACPI_OBJ_WORD_FIELD,
};

struct aml_state;
struct acpi_data;
struct acpi_obj;
struct acpi_ns;
struct uio;

TAILQ_HEAD(acpi_data_head, acpi_data);
TAILQ_HEAD(acpi_obj_head, acpi_obj);
TAILQ_HEAD(acpi_ns_head, acpi_ns);

struct acpi_ns
{
	struct acpi_ns *parent;
	struct acpi_obj *obj;
	struct acpi_obj_head obj_list;
};

struct acpi_data
{
	enum acpi_data_type type;
	union
	{
		struct
		{
			size_t size;
			uint8_t *data;
		} buffer;
		struct
		{
			uint8_t value;
		} byte;
		struct
		{
			uint32_t value;
		} dword;
		struct
		{
			uint64_t value;
		} qword;
		struct
		{
			uint16_t value;
		} word;
		struct
		{
			size_t len;
			char *data;
		} string;
		struct
		{
			struct acpi_data_head elements;
		} package;
	};
	TAILQ_ENTRY(acpi_data) chain;
};

struct acpi_obj
{
	struct acpi_ns *ns;
	enum acpi_obj_type type;
	uint32_t name;
	int external;
	union
	{
		/* NB: acpi_ns must be the first element in case of scoped
		 * object to allow safe upgrade from scope type
		 */
		struct
		{
			struct acpi_ns ns;
		} device;
		struct
		{
			size_t offset;
			size_t length;
			uint8_t flags;
		} method;
		struct
		{
			struct acpi_data *data;
		} namedef;
		struct
		{
			struct acpi_ns ns;
		} power_resource;
		struct
		{
			struct acpi_ns ns;
		} processor;
		struct
		{
			struct acpi_ns ns;
		} scope;
		struct
		{
			struct acpi_ns ns;
		} thermal_zone;
	};
	TAILQ_ENTRY(acpi_obj) chain;
};

typedef void (*acpi_probe_t)(struct acpi_obj *device, void *userdata);

int acpi_init(void);
void acpi_hpet_init(void);
int acpi_get_ecam_addr(uint8_t bus, uint8_t slot, uint8_t func,
                       uintptr_t *poffp);
const void *acpi_find_table(const char *name);
int acpi_shutdown(void);
int acpi_reboot(void);
int acpi_suspend(void);
int acpi_hibernate(void);
void acpi_probe_devices(acpi_probe_t probe, void *userdata);

int acpi_resource_get_fixed_memory_range_32(struct acpi_data *data,
                                            uint8_t *info,
                                            uint32_t *base,
                                            uint32_t *size);
int acpi_resource_get_ext_interrupt(struct acpi_data *data,
                                    uint8_t *flags,
                                    uint32_t *interrupts,
                                    uint8_t *interrupts_count);

int aml_alloc(struct aml_state **state);
void aml_free(struct aml_state *state);
int aml_parse(struct aml_state *state, const uint8_t *data, size_t size);
int aml_exec(struct aml_state *state, struct acpi_obj *method);
void aml_print(struct uio *uio, struct aml_state *state);
void aml_print_asl(struct uio *uio, struct aml_state *state,
                   const uint8_t *data, size_t size);
struct acpi_obj *aml_get_obj(struct aml_state *state, const char *name);
struct acpi_obj *aml_get_child(struct acpi_ns *ns, const char *name);

#endif
