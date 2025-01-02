import model
from util import is_cpp, world_version_is_wrath
from writer import Writer

def print_cache_mask_c(s: Writer, h: Writer):
    h.write_block(f"""
        #define WRATH_CACHE_MASK_LENGTH 32

        typedef struct {{
            uint32_t values[WRATH_CACHE_MASK_LENGTH];
        }} wrath_CacheMask;
    """)

    s.write_block(f"""
        static WowWorldResult wrath_cache_mask_write(WowWorldWriter* stream, const wrath_CacheMask* mask) {{
            int _return_value = 1;
            int i;
            uint32_t header = 0;

            for (i = 0; i < WRATH_CACHE_MASK_LENGTH; ++i) {{
                if (mask->values[i] != 0) {{
                    header |= (uint32_t)1 << i;
                }}
            }}

            WWM_CHECK_RETURN_CODE(wwm_write_u32(stream, header));

            for (i = 0; i < WRATH_CACHE_MASK_LENGTH; ++i) {{
                if (mask->values[i] != 0) {{
                    WWM_CHECK_RETURN_CODE(wwm_write_u32(stream, mask->values[i]));
                }}
            }}

            return WWM_RESULT_SUCCESS;
        cleanup: return _return_value;
        }}

        static WowWorldResult wrath_cache_mask_read(WowWorldReader* stream, wrath_CacheMask* mask) {{
            int _return_value = 1;
            int i = 0;
            uint32_t header = 0;
            WWM_CHECK_RETURN_CODE(wwm_read_u32(stream, &header));

            for (i = 0; i < WRATH_CACHE_MASK_LENGTH; ++i) {{
                if ((header & (uint32_t)1 << i) != 0) {{
                    WWM_CHECK_RETURN_CODE(wwm_read_u32(stream, &mask->values[i]));
                }} else {{
                    mask->values[i] = 0;
                }}
            }}

            return WWM_RESULT_SUCCESS;
        cleanup: return _return_value;
        }}

        static size_t wrath_cache_mask_size(const wrath_CacheMask* mask) {{
            int i;
            size_t size = 4; /* u32 header */
            for (i = 0; i < WRATH_CACHE_MASK_LENGTH; ++i) {{
                if (mask->values[i] != 0) {{
                    size += 4;
                }}
            }}

            return size;
        }}
    """)


def print_cache_mask_cpp(s: Writer, h: Writer):
    h.write_block(f"""
        constexpr auto WRATH_CACHE_MASK_LENGTH = 32;
        struct CacheMask {{
            uint32_t values[WRATH_CACHE_MASK_LENGTH];
        }};
    """)

    s.write_block(f"""
        static void cache_mask_write(Writer& writer, const CacheMask& mask) {{
            uint32_t header = 0;
            for (int i = 0; i < WRATH_CACHE_MASK_LENGTH; ++i) {{
                if (mask.values[i] != 0) {{
                    header |= static_cast<uint32_t>(1) << i;
                }}
            }}

            writer.write_u32(header);

            for (const auto v : mask.values) {{
                if (v != 0) {{
                    writer.write_u32(v);
                }}
            }}
        }}

        static CacheMask cache_mask_read(Reader& reader) {{
            uint32_t header = reader.read_u32();
            CacheMask mask{{}};

            for (int i = 0; i < WRATH_CACHE_MASK_LENGTH; ++i) {{
                if ((header & static_cast<uint32_t>(1) << i) != 0) {{
                    mask.values[i] = reader.read_u32();
                }}
            }}

            return mask;
        }}

        static size_t cache_mask_size(const CacheMask& mask) {{
            size_t size = 4; /* u32 header */
            for (int i = 0; i < WRATH_CACHE_MASK_LENGTH; ++i) {{
                if (mask.values[i] != 0) {{
                    size += 4;
                }}
            }}

            return size;
        }}
    """)

def print_cache_mask(s: Writer, h: Writer, v: model.WorldVersion):
    if not world_version_is_wrath(v):
        return


    if is_cpp():
        print_cache_mask_cpp(s, h)
    else:
        print_cache_mask_c(s, h)
