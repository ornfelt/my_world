import model
from util import is_cpp, world_version_is_wrath
from writer import Writer

def print_inspect_talent_gear_mask_c(s: Writer, h: Writer):
    h.write_block(f"""
        #define WRATH_INSPECT_TALENT_GEAR_MASK_LENGTH 32

        typedef struct {{
            wrath_InspectTalentGear values[WRATH_INSPECT_TALENT_GEAR_MASK_LENGTH];
        }} wrath_InspectTalentGearMask;
    """)

    s.write_block(f"""
        static WowWorldResult wrath_inspect_talent_gear_mask_write(WowWorldWriter* stream, const wrath_InspectTalentGearMask* mask) {{
            int _return_value = 1;
            int i;
            uint32_t header = 0;

            for (i = 0; i < WRATH_INSPECT_TALENT_GEAR_MASK_LENGTH; ++i) {{
                if (mask->values[i].item != 0) {{
                    header |= (uint32_t)1 << i;
                }}
            }}

            WWM_CHECK_RETURN_CODE(wwm_write_u32(stream, header));

            for (i = 0; i < WRATH_INSPECT_TALENT_GEAR_MASK_LENGTH; ++i) {{
                if (mask->values[i].item != 0) {{
                    WWM_CHECK_RETURN_CODE(wrath_InspectTalentGear_write(stream, &mask->values[i]));
                }}
            }}

            return WWM_RESULT_SUCCESS;
        cleanup: return _return_value;
        }}

        static WowWorldResult wrath_inspect_talent_gear_mask_read(WowWorldReader* stream, wrath_InspectTalentGearMask* mask) {{
            int _return_value = 1;
            int i = 0;
            uint32_t header = 0;
            WWM_CHECK_RETURN_CODE(wwm_read_u32(stream, &header));

            for (i = 0; i < WRATH_INSPECT_TALENT_GEAR_MASK_LENGTH; ++i) {{
                if ((header & (uint32_t)1 << i) != 0) {{
                    WWM_CHECK_RETURN_CODE(wrath_InspectTalentGear_read(stream, &mask->values[i]));
                }} else {{
                    memset(&mask->values[i], 0, sizeof(mask->values[i]));
                }}
            }}

            return WWM_RESULT_SUCCESS;
        cleanup: return _return_value;
        }}

        static size_t wrath_inspect_talent_gear_mask_size(const wrath_InspectTalentGearMask* mask) {{
            int i;
            size_t size = 4; /* u32 header */
            for (i = 0; i < WRATH_INSPECT_TALENT_GEAR_MASK_LENGTH; ++i) {{
                if (mask->values[i].item != 0) {{
                    size += wrath_InspectTalentGear_size(&mask->values[i]);
                }}
            }}

            return size;
        }}
    """)


def print_inspect_talent_gear_mask_cpp(s: Writer, h: Writer):
    h.write_block(f"""
        constexpr auto WRATH_INSPECT_TALENT_GEAR_MASK_LENGTH = 32;
        struct InspectTalentGearMask {{
            InspectTalentGear values[WRATH_INSPECT_TALENT_GEAR_MASK_LENGTH];
        }};
    """)

    s.write_block(f"""
        static void inspect_talent_gear_mask_write(Writer& writer, const InspectTalentGearMask& mask) {{
            uint32_t header = 0;
            for (int i = 0; i < WRATH_INSPECT_TALENT_GEAR_MASK_LENGTH; ++i) {{
                if (mask.values[i].item != 0) {{
                    header |= static_cast<uint32_t>(1) << i;
                }}
            }}

            writer.write_u32(header);

            for (const auto& v : mask.values) {{
                if (v.item != 0) {{
                    InspectTalentGear_write(writer, v);
                }}
            }}
        }}

        static InspectTalentGearMask inspect_talent_gear_mask_read(Reader& reader) {{
            uint32_t header = reader.read_u32();
            InspectTalentGearMask mask{{}};

            for (int i = 0; i < WRATH_INSPECT_TALENT_GEAR_MASK_LENGTH; ++i) {{
                if ((header & static_cast<uint32_t>(1) << i) != 0) {{
                    mask.values[i] = InspectTalentGear_read(reader);
                }}
            }}

            return mask;
        }}

        static size_t inspect_talent_gear_mask_size(const InspectTalentGearMask& mask) {{
            size_t size = 4; /* u32 header */
            for (int i = 0; i < WRATH_INSPECT_TALENT_GEAR_MASK_LENGTH; ++i) {{
                if (mask.values[i].item != 0) {{
                    size += InspectTalentGear_size(mask.values[i]);;
                }}
            }}

            return size;
        }}
    """)

def print_inspect_talent_gear_mask(s: Writer, h: Writer, v: model.WorldVersion):
    if not world_version_is_wrath(v):
        return


    if is_cpp():
        print_inspect_talent_gear_mask_cpp(s, h)
    else:
        print_inspect_talent_gear_mask_c(s, h)
