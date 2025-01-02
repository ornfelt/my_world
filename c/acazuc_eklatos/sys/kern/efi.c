#define ENABLE_TRACE

#include <multiboot.h>
#include <errno.h>
#include <types.h>
#include <efi.h>
#include <mem.h>

#define EFI_SUCCESS              0
#define EFI_LOAD_ERROR           1
#define EFI_INVALID_PARAMETER    2
#define EFI_UNSUPPORTED          3
#define EFI_BAD_BUFFER_SIZE      4
#define EFI_BUFFER_TOO_SMALL     5
#define EFI_NOT_READY            6
#define EFI_DEVICE_ERROR         7
#define EFI_WRITE_PROTECTED      8
#define EFI_OUT_OF_RESOURCES     9
#define EFI_VOLUME_CORRUPTED     10
#define EFI_VOLUME_FULL          11
#define EFI_NO_MEDIA             12
#define EFI_MEDIA_CHANGED        13
#define EFI_NOT_FOUND            14
#define EFI_ACCESS_DENIED        15
#define EFI_NO_RESPONSE          16
#define EFI_NO_MAPPING           17
#define EFI_TIMEOUT              18
#define EFI_NOT_STARTED          19
#define EFI_ALREADY_STARTED      20
#define EFI_ABORTED              21
#define EFI_ICMP_ERROR           22
#define EFI_TFTP_ERROR           23
#define EFI_PROTOCOL_ERROR       24
#define EFI_INCOMPATIBLE_VERSION 25
#define EFI_SECURITY_VIOLATION   26
#define EFI_CRC_ERROR            27
#define EFI_END_OF_MEDIA         28
#define EFI_END_OF_FILE          31
#define EFI_INVALID_LANGUAGE     32
#define EFI_COMPROMISED_DATA     33
#define EFI_IP_ADDRESS_CONFLICT  34
#define EFI_HTTP_ERROR           35

#define EFI_SHIFT_STATE_VALID          0x80000000
#define EFI_RIGHT_SHIFT_PRESSED        0x00000001
#define EFI_LEFT_SHIFT_PRESSED         0x00000002
#define EFI_RIGHT_CONTROL_PRESSED      0x00000004
#define EFI_LEFT_CONTROL_PRESSED       0x00000008
#define EFI_RIGHT_ALT_PRESSED          0x00000010
#define EFI_LEFT_ALT_PRESSED           0x00000020
#define EFI_RIGHT_LOGO_PRESSED         0x00000040
#define EFI_LEFT_LOGO_PRESSED          0x00000080
#define EFI_MENU_KEY_PRESSED           0x00000100
#define EFI_SYS_REQ_PRESSED            0x00000200

#define EFI_SCROLL_LOCK_ACTIVE 0x01
#define EFI_NUM_LOCK_ACTIVE    0x02
#define EFI_CAPS_LOCK_ACTIVE   0x04
#define EFI_KEY_STATE_EXPOSED  0x40
#define EFI_TOGGLE_STATE_VALID 0x80

#define EFI_SYSTEM_TABLE_SIGNATURE      0x5453595320494249
#define EFI_2_100_SYSTEM_TABLE_REVISION ((2 << 16) | (100))
#define EFI_2_90_SYSTEM_TABLE_REVISION  ((2 << 16) | (90))
#define EFI_2_80_SYSTEM_TABLE_REVISION  ((2 << 16) | (80))
#define EFI_2_70_SYSTEM_TABLE_REVISION  ((2 << 16) | (70))
#define EFI_2_60_SYSTEM_TABLE_REVISION  ((2 << 16) | (60))
#define EFI_2_50_SYSTEM_TABLE_REVISION  ((2 << 16) | (50))
#define EFI_2_40_SYSTEM_TABLE_REVISION  ((2 << 16) | (40))
#define EFI_2_31_SYSTEM_TABLE_REVISION  ((2 << 16) | (31))
#define EFI_2_30_SYSTEM_TABLE_REVISION  ((2 << 16) | (30))
#define EFI_2_20_SYSTEM_TABLE_REVISION  ((2 << 16) | (20))
#define EFI_2_10_SYSTEM_TABLE_REVISION  ((2 << 16) | (10))
#define EFI_2_00_SYSTEM_TABLE_REVISION  ((2 << 16) | (00))
#define EFI_1_10_SYSTEM_TABLE_REVISION  ((1 << 16) | (10))
#define EFI_1_02_SYSTEM_TABLE_REVISION  ((1 << 16) | (02))
#define EFI_SPECIFICATION_VERSION       EFI_SYSTEM_TABLE_REVISION
#define EFI_SYSTEM_TABLE_REVISION       EFI_2_100_SYSTEM_TABLE_REVISION

#define EFI_BLACK                              0x00
#define EFI_BLUE                               0x01
#define EFI_GREEN                              0x02
#define EFI_CYAN                               0x03
#define EFI_RED                                0x04
#define EFI_MAGENTA                            0x05
#define EFI_BROWN                              0x06
#define EFI_LIGHTGRAY                          0x07
#define EFI_BRIGHT                             0x08
#define EFI_DARKGRAY                           0x08
#define EFI_LIGHTBLUE                          0x09
#define EFI_LIGHTGREEN                         0x0A
#define EFI_LIGHTCYAN                          0x0B
#define EFI_LIGHTRED                           0x0C
#define EFI_LIGHTMAGENTA                       0x0D
#define EFI_YELLOW                             0x0E
#define EFI_WHITE                              0x0F


#define EFI_BACKGROUND_BLACK                   0x00
#define EFI_BACKGROUND_BLUE                    0x10
#define EFI_BACKGROUND_GREEN                   0x20
#define EFI_BACKGROUND_CYAN                    0x30
#define EFI_BACKGROUND_RED                     0x40
#define EFI_BACKGROUND_MAGENTA                 0x50
#define EFI_BACKGROUND_BROWN                   0x60
#define EFI_BACKGROUND_LIGHTGRAY               0x70

#define EFI_TEXT_ATTR(fb, bg)  ((fb) | ((bg) << 4))

typedef void *efi_handle_t;
typedef void *efi_event_t;
typedef size_t efi_status_t;
typedef uint8_t efi_boolean_t;
typedef int8_t efi_int8_t;
typedef uint8_t efi_uint8_t;
typedef uint16_t efi_char16_t;
typedef int16_t efi_int16_t;
typedef uint16_t efi_uint16_t;
typedef int32_t efi_int32_t;
typedef uint32_t efi_uint32_t;
typedef int64_t efi_int64_t;
typedef uint64_t efi_uint64_t;
typedef ssize_t efi_intn_t;
typedef size_t efi_uintn_t;
typedef uint32_t efi_toogle_state_t;

struct efi_table_header
{
	efi_uint64_t signature;
	efi_uint32_t revision;
	efi_uint32_t header_size;
	efi_uint32_t crc32;
	efi_uint32_t reserved;
};

struct efi_guid
{
	efi_uint32_t data1;
	efi_uint16_t data2;
	efi_uint16_t data3;
	efi_uint8_t data4[8];
};

struct efi_simple_text_input_protocol;

struct efi_input_key
{
	efi_uint16_t scan_code;
	efi_char16_t unicode_char;
};

struct efi_key_state
{
	efi_uint32_t key_shift_state;
	efi_toogle_state_t key_toggle_state;
};

struct efi_key_data
{
	struct efi_input_key key;
	struct efi_key_state key_state;
};

struct efi_simple_text_input_protocol;

typedef efi_status_t (*efi_input_reset_t)(struct efi_simple_text_input_protocol *this, efi_boolean_t extended_verification);
typedef efi_status_t (*efi_input_read_key_t)(struct efi_simple_text_input_protocol *this, struct efi_input_key *key);

struct efi_simple_text_input_protocol
{
	efi_input_reset_t reset;
	efi_input_read_key_t read_key_stroke;
	efi_event_t wait_for_key;
};

struct efi_simple_text_output_protocol;

typedef efi_status_t (*efi_text_reset_t)(struct efi_simple_text_output_protocol *this, efi_boolean_t extended_verification);
typedef efi_status_t (*efi_text_string_t)(struct efi_simple_text_output_protocol *this, efi_char16_t *string);
typedef efi_status_t (*efi_text_test_string_t)(struct efi_simple_text_output_protocol *this, efi_char16_t *string);
typedef efi_status_t (*efi_text_query_mode_t)(struct efi_simple_text_output_protocol *this, efi_uintn_t mode_number, efi_uintn_t *columns, efi_uintn_t *rows);
typedef efi_status_t (*efi_text_set_mode_t)(struct efi_simple_text_output_protocol *this, efi_uintn_t mode_number);
typedef efi_status_t (*efi_text_set_attribute_t)(struct efi_simple_text_output_protocol *this, efi_uintn_t attribute);
typedef efi_status_t (*efi_text_clear_screen_t)(struct efi_simple_text_output_protocol *this);
typedef efi_status_t (*efi_text_set_cursor_position_t)(struct efi_simple_text_output_protocol *this, efi_uintn_t column, efi_uintn_t row);
typedef efi_status_t (*efi_text_enable_cursor_t)(struct efi_simple_text_output_protocol *this, efi_boolean_t visible);

struct efi_simple_text_output_mode
{
	efi_int32_t max_mode;
	efi_int32_t mode;
	efi_int32_t attribute;
	efi_int32_t cursor_column;
	efi_int32_t cursor_row;
	efi_boolean_t cursor_visible;
};

struct efi_simple_text_output_protocol
{
	efi_text_reset_t reset;
	efi_text_string_t output_string;
	efi_text_test_string_t test_string;
	efi_text_query_mode_t query_mory;
	efi_text_set_mode_t set_mode;
	efi_text_set_attribute_t set_attribute;
	efi_text_clear_screen_t clear_screen;
	efi_text_set_cursor_position_t set_cursor_position;
	efi_text_enable_cursor_t enable_cursor;
	struct simple_text_output_mode *mode;
};

struct efi_configuration_table
{
	struct efi_guid vendor_guid;
	void *vendor_table;
};

struct efi_system_table
{
	struct efi_table_header hdr;
	efi_uint16_t *firmware_vendor;
	efi_uint32_t firmware_revision;
	efi_handle_t console_in_handle;
	struct efi_simple_text_input_protocol *con_in;
	efi_handle_t console_out_handle;
	struct efi_simple_text_output_protocol *con_out;
	efi_handle_t standard_error_handle;
	struct efi_simple_text_output_protocol *std_err;
	struct efi_runtime_services *runtime_services;
	struct efi_boot_services *boot_services;
	efi_uintn_t number_of_table_entries;
	struct efi_configuration_table *configuration_table;
};

static struct page system_table_page;
static struct efi_system_table *system_table;
static struct page configuration_tables_page;
static struct efi_configuration_table *configuration_tables;

int efi_init(void)
{
	uint8_t *configuration_ptr = NULL;
	uint8_t *system_ptr = NULL;
	int ret;

#if __SIZE_WIDTH__ == 32
	const struct multiboot_tag_efi32 *tag = (const struct multiboot_tag_efi32*)multiboot_find_tag(MULTIBOOT_TAG_TYPE_EFI32);
#else
	const struct multiboot_tag_efi64 *tag = (const struct multiboot_tag_efi64*)multiboot_find_tag(MULTIBOOT_TAG_TYPE_EFI64);
#endif

	if (!tag)
		return -ENOENT;
	pm_init_page(&system_table_page, tag->pointer / PAGE_SIZE);
	system_ptr = vm_map(&system_table_page, PAGE_SIZE * 2, VM_PROT_R);
	if (!system_ptr)
	{
		TRACE("efi: failed to map efi system table");
		return -ENOMEM;
	}
	system_table = (struct efi_system_table*)(system_ptr + tag->pointer % PAGE_SIZE);
	if (system_table->hdr.signature != EFI_SYSTEM_TABLE_SIGNATURE)
	{
		TRACE("efi: invalid system table signature: 0x%016" PRIx64,
		       system_table->hdr.signature);
		ret = -ENOENT;
		goto err;
	}
	pm_init_page(&configuration_tables_page, (uintptr_t)system_table->configuration_table / PAGE_SIZE);
	configuration_ptr = vm_map(&configuration_tables_page, PAGE_SIZE, VM_PROT_R);
	configuration_tables = (struct efi_configuration_table*)(configuration_ptr + (uintptr_t)system_table->configuration_table % PAGE_SIZE);
	if (!configuration_tables)
	{
		TRACE("efi: failed to map configuration tables");
		ret = -ENOMEM;
		goto err;
	}
	return 0;

err:
	if (system_ptr)
		vm_unmap(system_ptr, PAGE_SIZE * 2);
	if (configuration_ptr)
		vm_unmap(configuration_ptr, PAGE_SIZE);
	return ret;
}

static uintptr_t get_configuration_table(const struct efi_guid *guid)
{
	if (!system_table)
		return 0;
	for (size_t i = 0; i < system_table->number_of_table_entries; ++i)
	{
#if 0
		printf("table[%02zu] = "
		       "%08" PRIx32 "-"
		       "%04" PRIx16 "-"
		       "%04" PRIx16 "-"
		       "%02" PRIx8
		       "%02" PRIx8
		       "%02" PRIx8
		       "%02" PRIx8
		       "%02" PRIx8
		       "%02" PRIx8
		       "%02" PRIx8
		       "%02" PRIx8
		       " %p\n",
		       i,
		       configuration_tables[i].vendor_guid.data1,
		       configuration_tables[i].vendor_guid.data2,
		       configuration_tables[i].vendor_guid.data3,
		       configuration_tables[i].vendor_guid.data4[0],
		       configuration_tables[i].vendor_guid.data4[1],
		       configuration_tables[i].vendor_guid.data4[2],
		       configuration_tables[i].vendor_guid.data4[3],
		       configuration_tables[i].vendor_guid.data4[4],
		       configuration_tables[i].vendor_guid.data4[5],
		       configuration_tables[i].vendor_guid.data4[6],
		       configuration_tables[i].vendor_guid.data4[7],
		       configuration_tables[i].vendor_table);
#endif
		if (!memcmp(&configuration_tables[i].vendor_guid,
		            guid, sizeof(*guid)))
			return (uintptr_t)configuration_tables[i].vendor_table;
	}
	return 0;
}

uintptr_t efi_get_fdt(void)
{
	static const struct efi_guid fdt_guid =
	{
		0xB1B621D5,
		0xF19C,
		0x41A5,
		{
			0x83,
			0x0B,
			0xD9,
			0x15,
			0x2C,
			0x69,
			0xAA,
			0xE0,
		},
	};
	return get_configuration_table(&fdt_guid);
}
