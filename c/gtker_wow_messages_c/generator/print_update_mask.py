import model
from util import is_cpp
from writer import Writer


def print_update_mask(s: Writer, h: Writer, update_mask: list[model.UpdateMask], module_name: str):
    if module_name == "all":
        return

    if is_cpp():
        print_update_mask_cpp(s, h, update_mask, module_name)
    else:
        print_update_mask_c(s, h, update_mask, module_name)


def print_update_mask_cpp(s: Writer, h: Writer, update_mask: list[model.UpdateMask], module_name: str):
    h.open_curly("class UpdateMask")
    h.wln_no_indent("public:")
    export_define = "WOW_WORLD_MESSAGES_CPP_EXPORT"

    highest_offset: int = 0
    for index, value in enumerate(update_mask):
        highest_offset = max(value.offset, highest_offset)
        name = f"{value.object_type.name.lower()}_{value.name.lower()}"

        match value.data_type:
            case model.UpdateMaskDataTypeGUIDArrayUsingEnum(content=content):
                offset = f"{value.offset} + (static_cast<uint32_t>(def) * 2)"

                h.wln(f"{export_define} void {name}({content.definer.name} def, uint64_t value);")
                s.open_curly(f"{export_define} void UpdateMask::{name}({content.definer.name} def, uint64_t value)")
                s.wln(f"::wow_world_messages::util::update_mask_set_u64(headers, values, {offset}, value);")
                s.closing_curly()

                h.wln(f"{export_define} uint64_t {name}({content.definer.name} def) const;")
                s.open_curly(f"{export_define} uint64_t UpdateMask::{name}({content.definer.name} def) const")
                s.wln(f"return ::wow_world_messages::util::update_mask_get_u64(headers, values, {offset});")
                s.closing_curly()

                continue

            case model.UpdateMaskDataTypeArrayOfStruct(content=content):
                mask: model.UpdateMaskStruct = content.update_mask_struct

                for word_index, word in enumerate(mask.members):
                    member: model.UpdateMaskStructMember
                    for member_index, member in enumerate(word):
                        if member.size == 4:
                            value_type = "uint32_t"
                            function_type = "u32"
                        elif member.size == 8:
                            value_type = "uint64_t"
                            function_type = "u64"
                        else:
                            value_type = "uint32_t"
                            function_type = "u32"

                        h.wln(f"{export_define} void {name}_{member.member.name}(uint32_t index, {value_type} value);")
                        s.open_curly(f"{export_define} void UpdateMask::{name}_{member.member.name}(uint32_t index, {value_type} value)")
                        s.wln(f"::wow_world_messages::util::update_mask_set_{function_type}(headers, values, {value.offset} + index * {content.size} + {word_index}, value);")
                        s.closing_curly()

                        h.wln(f"{export_define} {value_type} {name}_{member.member.name}(uint32_t index) const;")
                        s.open_curly(f"{export_define} {value_type} UpdateMask::{name}_{member.member.name}(uint32_t index) const")
                        s.wln(f"return ::wow_world_messages::util::update_mask_get_{function_type}(headers, values, {value.offset} + index * {content.size} + {word_index});")
                        s.closing_curly()
                continue

            case model.UpdateMaskDataTypeInt():
                value_type = "uint32_t"
                function_type = "u32"

            case model.UpdateMaskDataTypeGUID():
                value_type = "uint64_t"
                function_type = "u64"

            case model.UpdateMaskDataTypeFloat():
                value_type = "float"
                function_type = "float"

            case model.UpdateMaskDataTypeBytes():
                value_type = "std::array<uint8_t, 4>"
                function_type = "bytes"

            case model.UpdateMaskDataTypeTwoShort():
                value_type = "std::pair<uint16_t, uint16_t>"
                function_type = "two_shorts"

            case _:
                raise Exception(f"unknown update mask type {value.data_type}")

        h.wln(f"{export_define} void {name}({value_type} value);")
        s.open_curly(f"{export_define} void UpdateMask::{name}({value_type} value)")
        s.wln(f"::wow_world_messages::util::update_mask_set_{function_type}(headers, values, {value.offset}, value);")
        s.closing_curly()

        h.wln(f"{export_define} {value_type} {name}() const;")
        s.open_curly(f"{export_define} {value_type} UpdateMask::{name}() const")
        s.wln(f"return ::wow_world_messages::util::update_mask_get_{function_type}(headers, values, {value.offset});")
        s.closing_curly()

    max_headers = highest_offset // 32
    if max_headers % 32 != 0:
        max_headers += 1
    max_values = highest_offset

    #h.wln_no_indent("private:") TODO Private?
    h.wln(f"uint32_t headers[{max_headers}];")
    h.wln(f"uint32_t values[{max_values}];")

    h.closing_curly(";")
    h.newline()

    one = "static_cast<uint32_t>(1)"

    s.write_block(f"""
        constexpr auto UPDATE_MASK_HEADERS_LENGTH = {max_headers};
    
        static void update_mask_write(Writer& writer, const UpdateMask& mask) {{
            uint8_t amount_of_headers = 0;

            for (uint8_t i = 0; i < UPDATE_MASK_HEADERS_LENGTH; ++i) {{
                const uint32_t header = mask.headers[i];
                if (header != 0) {{
                    amount_of_headers = i + 1;
                }}
            }}

            writer.write_u8(amount_of_headers);

            for (int i = 0; i < amount_of_headers; ++i) {{
                writer.write_u32(mask.headers[i]);
            }}
            
            for (int i = 0; i < amount_of_headers; ++i) {{
                const uint32_t header = mask.headers[i];
                for (int j = 0; j < 32; ++j) {{
                    if ((header & ({one} << j)) != 0) {{
                        writer.write_u32(mask.values[i * 32 + j]);
                    }}
                }}
            }}
        }}

        static UpdateMask update_mask_read(Reader& reader) {{
            UpdateMask mask{{}};

            uint8_t amount_of_headers = reader.read_u8();

            for (int i = 0; i < amount_of_headers; ++i) {{
                mask.headers[i] = reader.read_u32();
            }}

            for (int i = 0; i < amount_of_headers; ++i) {{
                uint32_t header = mask.headers[i];
                for (int j = 0; j < 32; ++j) {{
                    if ((header & ({one} << j)) != 0) {{
                        mask.values[i * 32 + j] = reader.read_u32();
                    }}
                }}
            }}
            
            return mask;
        }}

        static size_t update_mask_size(const UpdateMask& mask) {{
            size_t max_header = 0;
            size_t amount_of_values = 0;
            
            size_t size = 1; /* initial u8 */
            
            for(int i = 0; i < UPDATE_MASK_HEADERS_LENGTH; ++i) {{
                uint32_t header = mask.headers[i];
                for(int j = 0; j < 32; ++j) {{
                    if((header & ({one} << j)) != 0) {{
                        max_header = i + 1;
                        amount_of_values += 4;
                    }}
                }}
            }}
            
            return size + amount_of_values + (max_header * 4);
        }}
        """)


def print_update_mask_c(s: Writer, h: Writer, update_mask: list[model.UpdateMask], module_name: str):
    one = "(uint32_t)1"
    highest_offset = 0
    for index, value in enumerate(update_mask):
        highest_offset = max(value.offset, highest_offset)

    max_headers = highest_offset // 32
    if max_headers % 32 != 0:
        max_headers += 1
    max_values = highest_offset
    h.write_block(f"""
#define {module_name.upper()}_HEADERS_LENGTH {max_headers}
#define {module_name.upper()}_VALUES_LENGTH {max_values}
    
typedef struct {{
    uint32_t headers[{module_name.upper()}_HEADERS_LENGTH];
    uint32_t values[{module_name.upper()}_VALUES_LENGTH];
}} {module_name}_UpdateMask;

""")

    for index, value in enumerate(update_mask):
        name = f"{module_name.lower()}_update_mask_{value.object_type.name.lower()}_{value.name.lower()}"
        export_define = "WOW_WORLD_MESSAGES_C_EXPORT"

        match value.data_type:
            case model.UpdateMaskDataTypeGUIDArrayUsingEnum(content=content):
                function_declaration_set = f"{export_define} void {name}_set({module_name}_UpdateMask* mask, {module_name}_{content.definer.name} def, uint64_t value)"
                function_declaration_get = f"{export_define} uint64_t {name}_get(const {module_name}_UpdateMask* mask, {module_name}_{content.definer.name} def)"

                offset = f"{value.offset} + ((uint32_t)def * 2)"

                h.wln(f"{function_declaration_get};")
                s.open_curly(f"{function_declaration_get}")
                s.wln(f"return wwm_update_mask_get_u64(mask->headers, mask->values, {offset});")
                s.closing_curly()

                h.wln(f"{function_declaration_set};")
                s.open_curly(f"{function_declaration_set}")
                s.wln(f"wwm_update_mask_set_u64(mask->headers, mask->values, {offset}, value);")
                s.closing_curly()
                continue

            case model.UpdateMaskDataTypeArrayOfStruct(content=content):
                mask: model.UpdateMaskStruct = content.update_mask_struct

                for word_index, word in enumerate(mask.members):
                    member: model.UpdateMaskStructMember
                    for member_index, member in enumerate(word):
                        if member.size == 4:
                            value_type = "uint32_t"
                            function_type = "u32"
                        elif member.size == 8:
                            value_type = "uint64_t"
                            function_type = "u64"
                        else:
                            value_type = "uint32_t"
                            function_type = "u32"

                        function_declaration_set = f"{export_define} void {name}_{member.member.name}_set({module_name}_UpdateMask* mask, uint32_t index, {value_type} value)"
                        function_declaration_get = f"{export_define} {value_type} {name}_{member.member.name}_get(const {module_name}_UpdateMask* mask, uint32_t index)"
                        h.wln(f"{function_declaration_set};")
                        s.open_curly(function_declaration_set)
                        s.wln(f"wwm_update_mask_set_{function_type}(mask->headers, mask->values, {value.offset} + index * {content.size} + {word_index}, value);")
                        s.closing_curly()

                        h.wln(f"{function_declaration_get};")
                        s.open_curly(function_declaration_get)
                        s.wln(f"return wwm_update_mask_get_{function_type}(mask->headers, mask->values, {value.offset} + index * {content.size} + {word_index});")
                        s.closing_curly()
                continue

            case model.UpdateMaskDataTypeInt():
                value_type = "uint32_t"
                function_type = "u32"

            case model.UpdateMaskDataTypeGUID():
                value_type = "uint64_t"
                function_type = "u64"

            case model.UpdateMaskDataTypeFloat():
                value_type = "float"
                function_type = "float"

            case model.UpdateMaskDataTypeBytes():
                value_type = "WowBytes"
                function_type = "bytes"

            case model.UpdateMaskDataTypeTwoShort():
                value_type = "WowTwoShorts"
                function_type = "two_shorts"

            case _:
                raise Exception(f"unknown update mask type {value.data_type}")

        function_declaration_set = f"{export_define} void {name}_set({module_name}_UpdateMask* mask, {value_type} value)"
        function_declaration_get = f"{export_define} {value_type} {name}_get(const {module_name}_UpdateMask* mask)"

        h.wln(f"{function_declaration_set};")
        s.open_curly(f"{function_declaration_set}")
        s.wln(f"wwm_update_mask_set_{function_type}(mask->headers, mask->values, {value.offset}, value);")
        s.closing_curly()

        h.wln(f"{function_declaration_get};")
        s.open_curly(f"{function_declaration_get}")
        s.wln(f"return wwm_update_mask_get_{function_type}(mask->headers, mask->values, {value.offset});")
        s.closing_curly()
    h.newline()
    s.newline()

    s.write_block(f"""
static WowWorldResult {module_name}_update_mask_write(WowWorldWriter* stream, const {module_name}_UpdateMask* mask) {{
    int _return_value = 1;
    uint8_t i;
    uint8_t j;
    uint8_t amount_of_headers = 0;

    for (i = 0; i < {module_name.upper()}_HEADERS_LENGTH; ++i) {{
        uint32_t header = mask->headers[i];
        if (header != 0) {{
            amount_of_headers = i + 1;
        }}
    }}

    WWM_CHECK_RETURN_CODE(wwm_write_u8(stream, amount_of_headers));

    for (i = 0; i < amount_of_headers; ++i) {{
        WWM_CHECK_RETURN_CODE(wwm_write_u32(stream, mask->headers[i]));
    }}
    
    for (i = 0; i < amount_of_headers; ++i) {{
        uint32_t header = mask->headers[i];
        for (j = 0; j < 32; ++j) {{
            if ((header & ({one} << j)) != 0) {{
                WWM_CHECK_RETURN_CODE(wwm_write_u32(stream, mask->values[i * 32 + j]));
            }}
        }}
    }}

    return WWM_RESULT_SUCCESS;
cleanup: return _return_value;
}}

static WowWorldResult {module_name}_update_mask_read(WowWorldReader* stream, {module_name}_UpdateMask* mask) {{
    int _return_value = 1;
    uint8_t i;
    uint8_t j;
    
    uint8_t amount_of_headers;

    memset(mask->headers, 0, sizeof(mask->headers));
    memset(mask->values, 0, sizeof(mask->values));

    WWM_CHECK_RETURN_CODE(wwm_read_u8(stream, &amount_of_headers));

    for (i = 0; i < amount_of_headers; ++i) {{
        WWM_CHECK_RETURN_CODE(wwm_read_u32(stream, &mask->headers[i]));
    }}

    for (i = 0; i < amount_of_headers; ++i) {{
        uint32_t header = mask->headers[i];
        for (j = 0; j < 32; ++j) {{
            if ((header & ({one} << j)) != 0) {{
                WWM_CHECK_RETURN_CODE(wwm_read_u32(stream, &mask->values[i * 32 + j]));
            }}
        }}
    }}
    
    return WWM_RESULT_SUCCESS;
cleanup: return _return_value;
}}

static size_t {module_name}_update_mask_size(const {module_name}_UpdateMask* mask) {{
    size_t i;
    size_t j;
    size_t max_header = 0;
    size_t amount_of_values = 0;
    
    size_t size = 1; /* initial u8 */
    
    for(i = 0; i < {module_name.upper()}_HEADERS_LENGTH; ++i) {{
        uint32_t header = mask->headers[i];
        for(j = 0; j < 32; ++j) {{
            if((header & ({one} << j)) != 0) {{
                max_header = i + 1;
                amount_of_values += 4;
            }}
        }}
    }}
    
    return size + amount_of_values + (max_header * 4);
}}
""")
