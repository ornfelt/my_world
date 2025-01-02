import model
from util import is_cpp, world_version_is_wrath
from writer import Writer


def print_enchant_mask_c(s: Writer, h: Writer):
    h.write_block(f"""
        #define WRATH_ENCHANT_MASK_LENGTH 32

        typedef struct {{
            uint16_t values[WRATH_ENCHANT_MASK_LENGTH];
        }} wrath_EnchantMask;
    """)

    s.write_block(f"""
        static WowWorldResult wrath_enchant_mask_write(WowWorldWriter* stream, const wrath_EnchantMask* mask) {{
            int _return_value = 1;
            int i;
            uint16_t header = 0;

            for (i = 0; i < WRATH_ENCHANT_MASK_LENGTH; ++i) {{
                if (mask->values[i] != 0) {{
                    header |= (uint16_t)1 << i;
                }}
            }}

            WWM_CHECK_RETURN_CODE(wwm_write_u16(stream, header));

            for (i = 0; i < WRATH_ENCHANT_MASK_LENGTH; ++i) {{
                if (mask->values[i] != 0) {{
                    WWM_CHECK_RETURN_CODE(wwm_write_u16(stream, mask->values[i]));
                }}
            }}

            return WWM_RESULT_SUCCESS;
        cleanup: return _return_value; /* TODO Pre check size better */
        }}

        static WowWorldResult wrath_enchant_mask_read(WowWorldReader* stream, wrath_EnchantMask* mask) {{
            int _return_value = 1;
            int i = 0;
            uint16_t header = 0;
            WWM_CHECK_RETURN_CODE(wwm_read_u16(stream, &header));

            for (i = 0; i < WRATH_ENCHANT_MASK_LENGTH; ++i) {{
                if ((header & (uint16_t)1 << i) != 0) {{
                    WWM_CHECK_RETURN_CODE(wwm_read_u16(stream, &mask->values[i]));
                }} else {{
                    mask->values[i] = 0;
                }}
            }}

            return WWM_RESULT_SUCCESS;
        cleanup: return _return_value; /* TODO Pre check size better */
        }}

        static size_t wrath_enchant_mask_size(const wrath_EnchantMask* mask) {{
            int i;
            size_t size = 2; /* u16 header */
            for (i = 0; i < WRATH_ENCHANT_MASK_LENGTH; ++i) {{
                if (mask->values[i] != 0) {{
                    size += 2;
                }}
            }}

            return size;
        }}
    """)


def print_enchant_mask_cpp(s: Writer, h: Writer):
    h.write_block(f"""
        constexpr auto WRATH_ENCHANT_MASK_LENGTH = 32;
        struct EnchantMask {{
            uint16_t values[WRATH_ENCHANT_MASK_LENGTH];
        }};
    """)

    s.write_block(f"""
        static void enchant_mask_write(Writer& writer, const EnchantMask& mask) {{
            uint16_t header = 0;
            for (int i = 0; i < WRATH_ENCHANT_MASK_LENGTH; ++i) {{
                if (mask.values[i] != 0) {{
                    header |= static_cast<uint16_t>(1) << i;
                }}
            }}

            writer.write_u16(header);

            for (const auto v : mask.values) {{
                if (v != 0) {{
                    writer.write_u16(v);
                }}
            }}
        }}

        static EnchantMask enchant_mask_read(Reader& reader) {{
            uint16_t header = reader.read_u16();
            EnchantMask mask{{}};

            for (int i = 0; i < WRATH_ENCHANT_MASK_LENGTH; ++i) {{
                if ((header & static_cast<uint16_t>(1) << i) != 0) {{
                    mask.values[i] = reader.read_u16();
                }}
            }}

            return mask;
        }}

        static size_t enchant_mask_size(const EnchantMask& mask) {{
            size_t size = 2; /* u16 header */
            for (int i = 0; i < WRATH_ENCHANT_MASK_LENGTH; ++i) {{
                if (mask.values[i] != 0) {{
                    size += 2;
                }}
            }}

            return size;
        }}
    """)


def print_enchant_mask(s: Writer, h: Writer, v: model.WorldVersion):
    if not world_version_is_wrath(v):
        return

    if is_cpp():
        print_enchant_mask_cpp(s, h)
    else:
        print_enchant_mask_c(s, h)
