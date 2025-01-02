import typing

from util import world_version_is_vanilla, world_version_is_tbc, \
    world_version_is_wrath, world_version_is_all, is_cpp, version_to_module_name
from writer import Writer
import model


def print_aura_mask(s: Writer, h: Writer, v: model.WorldVersion):
    if world_version_is_all(v):
        return

    if is_cpp():
        print_aura_mask_cpp(s, h, v)
    else:
        print_aura_mask_c(s, h, v)

def print_aura_mask_cpp(s: Writer, h: Writer, v: model.WorldVersion):
    aura_type, header_size, member_size, not_set, write_aura, read_aura, write_prefix, init_field = version_parameters(v)
    one_suffix = "ULL" if header_size > 4 else ""

    h.write_block(f"""
        constexpr int AURA_MASK_SIZE = {header_size * 8};
        struct AuraMask {{
            {aura_type} auras[AURA_MASK_SIZE];
        }};
    """)
    h.newline()

    s.write_block(f"""
        static size_t aura_mask_size(const AuraMask& mask) {{
            size_t size = {header_size}; /* uint{header_size * 8}_t header */
            
            for(const auto v : mask.auras) {{
                if(v{not_set}) {{
                    size += {member_size}; /* {aura_type} members */
                }}
            }}
            
            return size;
        }}

        static AuraMask aura_mask_read(Reader& reader) {{
            const uint{header_size * 8}_t header = reader.read_u{header_size * 8}();
            
            AuraMask mask{{}};
            
            for(int i = 0; i < AURA_MASK_SIZE; ++i) {{
                if ((header & (1{one_suffix} << i)) != 0) {{
                    mask.auras[i] = {read_aura};
                }}
            }}

            return mask;
        }}

        static void aura_mask_write(Writer& writer, const AuraMask& mask) {{
            uint{header_size*8}_t header = 0;
            
            for(int i = 0; i < AURA_MASK_SIZE; ++i) {{
                if (mask.auras[i]{not_set}) {{
                    header |= 1{one_suffix} << i;
                }}
            }}
            
            writer.write_u{header_size * 8}(header);
            
            for(int i = 0; i < AURA_MASK_SIZE; ++i) {{
                if (mask.auras[i]{not_set}) {{
                    {write_aura};
                }}
            }}
        }}
    """)
    s.newline()


def print_aura_mask_c(s: Writer, h: Writer, v: model.WorldVersion):
    module_name = version_to_module_name(v)
    aura_type, header_size, member_size, not_set, write_aura, read_aura, write_prefix, init_field = version_parameters(v)
    one_cast = "(uint64_t)" if header_size > 4 else ""

    h.write_block(f"""
        #define {module_name.upper()}_AURA_MASK_SIZE {header_size * 8}
        typedef struct {{
            {aura_type} auras[{module_name.upper()}_AURA_MASK_SIZE];
        }} {module_name}_AuraMask;
    """)

    s.write_block(f"""
        static size_t {module_name}_aura_mask_size(const {module_name}_AuraMask* object) {{
            size_t size = {header_size}; /* uint{header_size * 8}_t header */
            size_t i;
            
            for(i = 0; i < {module_name.upper()}_AURA_MASK_SIZE; ++i) {{
                if(object->auras[i]{not_set}) {{
                    size += {member_size}; /* {aura_type} members */
                }}
            }}
            
            return size;
        }}

        static WowWorldResult {module_name}_aura_mask_read(WowWorldReader* reader, {module_name}_AuraMask* mask) {{
            int _return_value = 1;
            uint{header_size * 8}_t header;
            uint{header_size * 8}_t i;
            WWM_CHECK_RETURN_CODE(wwm_read_u{header_size * 8}(reader, &header));
            
            for(i = 0; i < {module_name.upper()}_AURA_MASK_SIZE; ++i) {{
                mask->auras[i]{init_field} = 0; /* initialize to 0 */
                
                if ((header & ({one_cast}1 << i)) != 0) {{
                    WWM_CHECK_RETURN_CODE({read_aura}(reader, &mask->auras[i]));
                }}
            }}

            return WWM_RESULT_SUCCESS;
        cleanup: return _return_value;
        }}

        static WowWorldResult {module_name}_aura_mask_write(WowWorldWriter* writer, const {module_name}_AuraMask* mask) {{
            int _return_value = 1;
            uint{header_size * 8}_t header = 0;
            uint{header_size * 8}_t i;
            
            for(i = 0; i < {module_name.upper()}_AURA_MASK_SIZE; ++i) {{
                if (mask->auras[i]{not_set}) {{
                    header |= {one_cast}1 << i;
                }}
            }}
            
            WWM_CHECK_RETURN_CODE(wwm_write_u{header_size * 8}(writer, header));
            
            for(i = 0; i < {module_name.upper()}_AURA_MASK_SIZE; ++i) {{
                if (mask->auras[i]{not_set}) {{
                    WWM_CHECK_RETURN_CODE({write_aura}(writer, {write_prefix}mask->auras[i]));
                }}
            }}
            
            return WWM_RESULT_SUCCESS;
        cleanup: return _return_value;
        }}
    """)


def version_parameters(v: model.WorldVersion) -> typing.Tuple[str, int, int, str, str, str, str, str]:
    module_name = version_to_module_name(v)

    init_field = ".aura"
    aura_type = f"{module_name}::Aura" if is_cpp() else f"{module_name}_Aura"
    header_size = 8
    not_set = ".aura != 0"
    write_aura = "Aura_write(writer, mask.auras[i])" if is_cpp() else f"{module_name}_Aura_write"
    read_aura = "Aura_read(reader)" if is_cpp() else f"{module_name}_Aura_read"
    write_prefix = "&"
    if world_version_is_vanilla(v):
        aura_type = "uint16_t"
        header_size = 4
        member_size = 2
        not_set = " != 0"
        write_aura = "writer.write_u16(mask.auras[i])" if is_cpp() else "wwm_write_u16"
        read_aura = "reader.read_u16()" if is_cpp() else "wwm_read_u16"
        init_field = ""
        write_prefix = ""
    elif world_version_is_tbc(v):
        member_size = 3
    elif world_version_is_wrath(v):
        member_size = 5
    else:
        raise Exception(f"incorrect version {v}")

    return aura_type, header_size, member_size, not_set, write_aura, read_aura, write_prefix, init_field
