import model
from util import version_to_module_name, is_cpp, world_version_is_all, world_version_is_vanilla
from writer import Writer


def print_addon_array(s: Writer, h: Writer, v: model.WorldVersion):
    if world_version_is_all(v) or world_version_is_vanilla(v):
        return

    if is_cpp():
        print_addon_array_cpp(s, v)
    else:
        print_addon_array_c(s, h, v)

def print_addon_array_c(s: Writer, h: Writer, v: model.WorldVersion):
    module_name = version_to_module_name(v)
    h.write_block(f"""
        typedef struct {{
            uint32_t amount_of_addons;
            {module_name}_Addon* addons;
        }} {module_name}_AddonArray;
    """)

    s.write_block(f"""
        static size_t {module_name}_addon_array_size(const {module_name}_AddonArray* array) {{
            return array->amount_of_addons * 8;
        }}

        static WowWorldResult {module_name}_addon_array_read(const WowWorldReader* reader, {module_name}_AddonArray* array) {{
            (void)reader;
            (void)array;
            /* Read for addon array is not implemented. If this is relevant for you create an issue on github. */
            abort();
        }}

        static WowWorldResult {module_name}_addon_array_write(WowWorldWriter* writer, const {module_name}_AddonArray* array) {{
            int _return_value;
            uint32_t i;
            for (i = 0; i < array->amount_of_addons; ++i) {{
                WWM_CHECK_RETURN_CODE({module_name}_Addon_write(writer, &array->addons[i]));
            }}

            return WWM_RESULT_SUCCESS;
        cleanup: return _return_value; /* TODO pre check before writing */
        }}
    """)

def print_addon_array_cpp(s: Writer, v: model.WorldVersion):
    s.write_block(f"""
        static std::vector<Addon> addon_array_read(Reader& reader) {{
            (void)reader;
            /* Read for addon array is not implemented. If this is relevant for you create an issue on github. */
            abort();
        }}

        static void addon_array_write(Writer& writer, const std::vector<Addon>& array) {{
            for (const auto & v : array) {{
                Addon_write(writer, v);
            }}
        }}
    """)
