from print_struct import container_has_c_members

import model
from model import Container
from print_struct.print_free import print_free_struct_member, container_has_free
from print_struct.print_write import array_size_inner_action
from print_struct.struct_util import (
    integer_type_to_size,
    print_if_statement_header, all_members_from_container, container_module_prefix,
)
from print_struct.struct_util import integer_type_to_short, integer_type_to_c_str
from util import container_needs_size_in_read, get_type_prefix, is_world, is_cpp, library_type, \
    snake_case_to_pascal_case
from print_struct.struct_util import array_type_to_c_str
from writer import Writer


def container_find_definition_by_name(e: model.Container, name: str) -> model.Definition:
    for d, _ in all_members_from_container(e):
        if d.name == name:
            return d

    raise Exception(f"unable to find {name} in {e.name}")


def print_read_struct_member(s: Writer, d: model.Definition, needs_size: bool, container_is_compressed: bool,
                             module_name: str, extra_indirection: str, container: model.Container):
    variable_name = f"object->{extra_indirection}{d.name}"
    reader = "reader"
    if is_cpp():
        variable_name = f"obj.{extra_indirection}{d.name}"

        if d.constant_value is not None or d.size_of_fields_before_size is not None:
            s.w("(void)")
        elif d.used_as_size_in is not None and is_cpp():
            s.w(f"auto {d.name} = ")
        else:
            if type(d.data_type) is not model.DataTypeArray:
                s.w(f"{variable_name} = ")

    if container_is_compressed:
        reader = "byte_reader"

    util_namespace = f"::wow_world_messages::util::" if is_cpp() else ""
    wlm_prefix = "WWM" if is_world(container.tags) else "WLM"

    match d.data_type:
        case model.DataTypeInteger(integer_type=integer_type):
            size = integer_type_to_size(integer_type)
            short = integer_type_to_short(integer_type)
            if is_cpp():
                s.wln_no_indent(f"{reader}.read_{short}();")
            elif d.constant_value is not None or d.size_of_fields_before_size is not None:
                s.wln(f"SKIP_FORWARD_BYTES({size});")
            else:
                s.wln(f"READ_{short.upper()}({variable_name});")

            if needs_size:
                s.wln(f"_size += {size};")
        case model.DataTypeBool(integer_type=integer_type):
            size = integer_type_to_size(integer_type)
            if is_cpp():
                s.wln_no_indent(f"{reader}.read_bool{size * 8}();")
            else:
                s.wln(f"READ_BOOL{size * 8}({variable_name});")

            if needs_size:
                s.wln(f"_size += {size};")

        case model.DataTypeFlag(
            type_name=type_name, integer_type=integer_type
        ) | model.DataTypeEnum(integer_type=integer_type, type_name=type_name):
            short = integer_type_to_short(integer_type)
            if is_cpp():
                s.wln_no_indent(f"static_cast<{type_name}>({reader}.read_{short}());")
            else:
                # enums can only be int in C, so we need to set upper bytes to 0
                s.wln(f"{variable_name} = 0;")
                s.wln(f"READ_{short.upper()}({variable_name});")

            size = integer_type_to_size(integer_type)
            if needs_size:
                s.wln(f"_size += {size};")

        case model.DataTypeString():
            if is_cpp():
                s.wln_no_indent(f"{reader}.read_string();")
            else:
                s.wln(f"READ_STRING({variable_name});")

            if needs_size:
                s.wln(
                    f"_size += {variable_name}.size() + 1;" if is_cpp() else f"_size += STRING_SIZE({variable_name}) + 1;")
        case model.DataTypeCstring():
            if is_cpp():
                s.wln_no_indent(f"{reader}.read_cstring();")
            else:
                s.wln(f"READ_CSTRING({variable_name});")

            if needs_size:
                s.wln(
                    f"_size += {variable_name}.size() + 1;" if is_cpp() else f"_size += STRING_SIZE({variable_name}) + 1;")

        case model.DataTypeSizedCstring():
            if is_cpp():
                s.wln_no_indent(f"{reader}.read_sized_cstring();")
            else:
                s.wln(f"READ_SIZED_CSTRING({variable_name});")

            if needs_size:
                s.wln(f"_size += STRING_SIZE({variable_name}) + 5;")

        case model.DataTypeLevel32() | model.DataTypeSpell() | model.DataTypeItem() \
             | model.DataTypeDateTime() | model.DataTypeGold() | model.DataTypeSeconds() \
             | model.DataTypeMilliseconds() | model.DataTypeIPAddress():
            if is_cpp():
                s.wln_no_indent(f"{reader}.read_u32();")
            else:
                s.wln(f"READ_U32({variable_name});")

            if needs_size:
                s.wln(f"_size += 4;")

        case model.DataTypeGUID():
            if is_cpp():
                s.wln_no_indent(f"{reader}.read_u64();")
            else:
                s.wln(f"READ_U64({variable_name});")

            if needs_size:
                s.wln(f"_size += 8;")

        case model.DataTypeLevel():
            if is_cpp():
                s.wln_no_indent(f"{reader}.read_u8();")
            else:
                s.wln(f"READ_U8({variable_name});")

            if needs_size:
                s.wln(f"_size += 1;")

        case model.DataTypeLevel16() | model.DataTypeSpell16():
            if is_cpp():
                s.wln_no_indent(f"{reader}.read_u16();")
            else:
                s.wln(f"READ_U16({variable_name});")

            if needs_size:
                s.wln(f"_size += 2;")

        case model.DataTypePackedGUID():
            if is_cpp():
                s.wln_no_indent(f"{reader}.read_packed_guid();")
            else:
                s.wln(f"READ_PACKED_GUID({variable_name});")

            if needs_size:
                prefix = "::wow_world_messages::util::" if is_cpp() else ""
                s.wln(f"_size += {prefix}wwm_packed_guid_size({variable_name});")

        case model.DataTypeFloatingPoint() | model.DataTypePopulation():
            if is_cpp():
                s.wln_no_indent(f"{reader}.read_float();")
            else:
                s.wln(f"READ_FLOAT({variable_name});")

            if needs_size:
                s.wln(f"_size += 4;")

        case model.DataTypeStruct(struct_data=e):
            version = container_module_prefix(e.tags, module_name)
            if is_cpp():
                s.wln_no_indent(f"::wow_{library_type(e.tags)}_messages::{version}::{e.name}_read({reader});")
            else:
                wlm_prefix = "WWM" if is_world(e.tags) else "WLM"

                s.wln(f"{wlm_prefix}_CHECK_RETURN_CODE({version}_{e.name}_read({reader}, &{variable_name}));")

            if needs_size:
                if e.sizes.constant_sized:
                    s.wln(f"_size += {e.sizes.maximum_size};")
                else:
                    s.wln(f"_size += {d.name}.size();")

        case model.DataTypeUpdateMask():
            if is_cpp():
                s.wln_no_indent(f"{module_name}::update_mask_read({reader});")
            else:
                s.wln(f"WWM_CHECK_RETURN_CODE({module_name}_update_mask_read({reader}, &{variable_name}));")

            if needs_size:
                s.wln(f"_size += {d.name}.size();")

        case model.DataTypeAuraMask():
            if is_cpp():
                this_namespace = f"::wow_world_messages::{module_name}::"
                s.wln_no_indent(f"{this_namespace}aura_mask_read({reader});")
            else:
                s.wln(f"WWM_CHECK_RETURN_CODE({module_name}_aura_mask_read({reader}, &{variable_name}));")

            if needs_size:
                prefix = "" if is_cpp() else f"{module_name}_"
                s.wln(f"_size += {prefix}aura_mask_size({variable_name});")

        case model.DataTypeMonsterMoveSpline():
            if is_cpp():
                s.wln_no_indent(f"{util_namespace}wwm_read_monster_move_spline({reader});")
            else:
                s.wln(f"READ_MONSTER_MOVE_SPLINE({variable_name});")

            if needs_size:
                s.wln(f"_size += {util_namespace}wwm_monster_move_spline_size({variable_name})")

        case model.DataTypeEnchantMask():
            if is_cpp():
                s.wln_no_indent(f"{module_name}::enchant_mask_read({reader});")
            else:
                s.wln(f"WWM_CHECK_RETURN_CODE({module_name}_enchant_mask_read({reader}, &{variable_name}));")

            if needs_size:
                prefix = "" if is_cpp() else f"{module_name}_"
                s.wln(f"_size += {prefix}enchant_mask_size({variable_name});")

        case model.DataTypeNamedGUID():
            if is_cpp():
                s.wln_no_indent(f"{util_namespace}wwm_read_named_guid({reader});")
            else:
                s.wln(f"READ_NAMED_GUID({variable_name});")

            if needs_size:
                s.wln(f"_size += {util_namespace}wwm_named_guid_size({variable_name});")

        case model.DataTypeInspectTalentGearMask():
            if is_cpp():
                s.wln_no_indent(f"{module_name}::inspect_talent_gear_mask_read({reader});")
            else:
                s.wln(
                    f"WWM_CHECK_RETURN_CODE({module_name}_inspect_talent_gear_mask_read({reader}, &{variable_name}));")

            if needs_size:
                prefix = "" if is_cpp() else f"{module_name}_"
                s.wln(f"_size += {prefix}inspect_talent_gear_mask_size({variable_name});")

        case model.DataTypeVariableItemRandomProperty():
            if is_cpp():
                s.wln_no_indent(f"{util_namespace}wwm_read_variable_item_random_property({reader});")
            else:
                s.wln(f"READ_VARIABLE_ITEM_RANDOM_PROPERTY({variable_name});")

            if needs_size:
                s.wln(f"_size += {util_namespace}wwm_variable_item_random_property_size({variable_name});")

        case model.DataTypeCacheMask():
            if is_cpp():
                s.wln_no_indent(f"{module_name}::cache_mask_read({reader});")
            else:
                s.wln(f"WWM_CHECK_RETURN_CODE({module_name}_cache_mask_read({reader}, &{variable_name}));")

            if needs_size:
                s.wln(f"_size += {util_namespace}cache_mask_size({variable_name});")

        case model.DataTypeAddonArray():
            if is_cpp():
                s.wln_no_indent(f"::wow_world_messages::{module_name}::addon_array_read({reader});")
            else:
                s.wln(f"WWM_CHECK_RETURN_CODE({module_name}_addon_array_read({reader}, &{variable_name}));")

            if needs_size:
                s.wln(f"_size += {module_name}_addon_array_size({variable_name});")

        case model.DataTypeAchievementDoneArray():
            if is_cpp():
                s.wln_no_indent(f"{module_name}::achievement_done_array_read({reader});")
            else:
                s.wln(f"WWM_CHECK_RETURN_CODE({module_name}_achievement_done_array_read({reader}, &{variable_name}));")

            if needs_size:
                s.wln(f"_size += {util_namespace}achievement_done_array_size({variable_name});")

        case model.DataTypeAchievementInProgressArray():
            if is_cpp():
                s.wln_no_indent(f"{module_name}::achievement_in_progress_array_read({reader});")
            else:
                s.wln(
                    f"WWM_CHECK_RETURN_CODE({module_name}_achievement_in_progress_array_read({reader}, &{variable_name}));")

            if needs_size:
                s.wln(f"_size += {util_namespace}achievement_in_progress_array_size({variable_name});")

        case model.DataTypeArray(compressed=compressed, size=array_size, inner_type=inner_type):
            if is_cpp():
                if compressed:
                    s.open_curly("if((body_size - _size) == 0)")
                    s.wln("return obj;")
                    s.closing_curly()
                    s.newline()

                    s.wln(f"/* {d.name}_decompressed_size: u32 */")
                    s.wln(f"auto {d.name}_decompressed_size = reader.read_u32();")
                    s.wln(f"(void){d.name}_decompressed_size;")
                    s.wln("_size += 4;")
                    s.newline()

                    s.wln(f"auto {d.name}_compressed_data = std::vector<unsigned char>(body_size - _size, 0);")
                    s.open_curly("for(size_t i = 0; i < body_size - _size; ++i)")
                    s.wln(f"{d.name}_compressed_data[i] = reader.read_u8();")
                    s.closing_curly()
                    s.newline()

                    s.open_curly(f"if ({d.name}_decompressed_size == 0)")
                    s.wln("return obj;")
                    s.closing_curly()  # if ()
                    s.newline()

                    s.wln(
                        f"auto {d.name}_decompressed_data = ::wow_world_messages::util::decompress_data({d.name}_compressed_data);")
                    s.newline()

                    s.open_curly(f"if ({d.name}_decompressed_data.empty())")
                    s.wln("return obj;")
                    s.closing_curly()  # if ()
                    s.newline()

                    s.wln(f"auto {d.name}_new_reader = ByteReader({d.name}_decompressed_data);")
                    reader = f"{d.name}_new_reader"

                    s.newline()

                match array_size:
                    case model.ArraySizeFixed(size=size_fixed_size):
                        s.open_curly(f"for (auto i = 0; i < {size_fixed_size}; ++i)")
                    case model.ArraySizeVariable(size=size_variable_size):
                        definition: model.Definition = container_find_definition_by_name(container, size_variable_size)
                        loop_type = integer_type_to_c_str(
                            definition.data_type.integer_type)  # type: ignore[attr-defined]
                        s.open_curly(f"for ({loop_type} i = 0; i < {size_variable_size}; ++i)")
                    case model.ArraySizeEndless():
                        if compressed:
                            s.open_curly(f"while (!{d.name}_new_reader.is_at_end())")
                        elif container_is_compressed:
                            s.open_curly(f"while (!byte_reader.is_at_end())")
                        else:
                            s.open_curly("while (_size < body_size)")
                    case _:
                        raise Exception("unknown array size")

                inner: str = ""
                match inner_type:
                    case model.ArrayTypeInteger(integer_type=integer_type):
                        short = integer_type_to_short(integer_type)
                        inner = f"{reader}.read_{short}()"

                    case model.ArrayTypeSpell():
                        inner = f"{reader}.read_u32()"

                    case model.ArrayTypeGUID():
                        inner = f"{reader}.read_u64()"

                    case model.ArrayTypeCstring():
                        inner = f"{reader}.read_cstring()"

                    case model.ArrayTypePackedGUID():
                        inner = f"{reader}.read_packed_guid()"

                    case model.ArrayTypeStruct(struct_data=e):
                        inner = f"::wow_{library_type(e.tags)}_messages::{container_module_prefix(e.tags, module_name)}::{e.name}_read({reader})"

                    case v2:
                        raise Exception(f"{v2}")

                match array_size:
                    case model.ArraySizeFixed():
                        s.wln(f"{variable_name}[i] = {inner};")
                    case model.ArraySizeVariable() | model.ArraySizeEndless():
                        s.wln(f"{variable_name}.push_back({inner});")
                    case _:
                        raise Exception("unknown array size")

                if needs_size or isinstance(array_size, model.ArraySizeEndless):
                    s.w("_size += ")
                    match inner_type:
                        case model.ArrayTypeInteger(integer_type=integer_type):
                            size = integer_type_to_size(integer_type)
                            s.w_no_indent(str(size))

                        case model.ArrayTypeGUID():
                            s.w_no_indent(str(8))

                        case model.ArrayTypeSpell():
                            s.w_no_indent(str(4))

                        case model.ArrayTypeStruct(struct_data=e):
                            if e.sizes.constant_sized:
                                s.w_no_indent(str(e.sizes.maximum_size))
                            else:
                                s.w_no_indent(f"{module_name}::{e.name}_size({variable_name}.back())")

                        case model.ArrayTypeCstring():
                            s.w_no_indent(f"{variable_name}.back().size() + 1")

                        case model.ArrayTypePackedGUID():
                            s.w_no_indent(f"{module_name}::wwm_packed_guid_size({variable_name}.back())")

                        case v3:
                            raise Exception(f"{v3}")
                    s.wln_no_indent(";")
                s.closing_curly()  # for (auto i


            else:
                if compressed:
                    s.open_curly("if((body_size - _size) > (reader->length - reader->index))")
                    s.wln("_return_value = WWM_RESULT_NOT_ENOUGH_BYTES;")
                    s.wln("goto cleanup;")
                    s.closing_curly()
                    s.newline()

                    s.wln(f"/* {d.name}_decompressed_size: u32 */")
                    s.wln(f"READ_U32({d.name}_decompressed_size);")
                    s.wln("_size += 4;")
                    s.newline()

                    s.wln(f"{variable_name} = NULL;")
                    s.wln(f"object->{extra_indirection}amount_of_{d.name} = 0;")
                    s.open_curly(f"if({d.name}_decompressed_size)")
                    s.wln(f"const size_t {d.name}_compressed_data_size = body_size - _size;")

                    s.open_curly(f"if ({d.name}_compressed_data_size > reader->length - reader->index)")
                    s.wln("_return_value = WWM_RESULT_NOT_ENOUGH_BYTES;")
                    s.wln("goto cleanup;")
                    s.closing_curly()

                    s.wln(f"{d.name}_decompressed_data = malloc({d.name}_decompressed_size);")
                    s.open_curly(f"if ({d.name}_decompressed_data == NULL)")
                    s.wln("_return_value = WWM_RESULT_MALLOC_FAIL;")
                    s.wln("goto cleanup;")
                    s.closing_curly()
                    s.newline()

                    s.wln(
                        f"{d.name}_decompressed_size = (uint32_t)wwm_decompress_data(&reader->source[reader->index], {d.name}_compressed_data_size, {d.name}_decompressed_data, {d.name}_decompressed_size);")
                    s.open_curly(
                        f"if ({d.name}_decompressed_size == 0)")
                    s.wln("_return_value = WWM_RESULT_COMPRESSION_ERROR;")
                    s.wln("goto cleanup;")
                    s.closing_curly()

                    s.wln(f"new_reader = wwm_create_reader({d.name}_decompressed_data, {d.name}_decompressed_size);")
                    s.wln("reader = &new_reader;")

                    s.newline()

                inner = ""
                match inner_type:
                    case model.ArrayTypeInteger(integer_type=integer_type):
                        short = integer_type_to_short(integer_type).upper()
                        inner = f"READ_{short}({variable_name}[i])"

                    case model.ArrayTypeSpell():
                        inner = f"READ_U32({variable_name}[i])"

                    case model.ArrayTypeGUID():
                        inner = f"READ_U64({variable_name}[i])"

                    case model.ArrayTypeStruct(struct_data=e):
                        version = container_module_prefix(e.tags, module_name)
                        inner = f"{wlm_prefix}_CHECK_RETURN_CODE({version}_{e.name}_read(reader, &{variable_name}[i]))"

                    case model.ArrayTypeCstring():
                        inner = f"READ_CSTRING({variable_name}[i])"

                    case model.ArrayTypePackedGUID():
                        if is_cpp():
                            inner = f"reader.read_packed_guid()"
                        else:
                            inner = f"READ_PACKED_GUID({variable_name}[i])"

                    case v2:
                        raise Exception(f"{v2}")

                match array_size:
                    case model.ArraySizeFixed(size=fixed_size):
                        extra = ""
                        if needs_size:
                            extra = f";_size += {array_size_inner_action(inner_type, variable_name, 'i', module_name)};"
                        s.wln(f"READ_ARRAY({variable_name}, {fixed_size}, {inner}{extra});")

                    case model.ArraySizeVariable(size=variable_size):
                        extra = ""
                        if needs_size:
                            extra = f";_size += {array_size_inner_action(inner_type, variable_name, 'i', module_name)};"
                        s.write_block(f"""
                            {variable_name} = calloc(object->{extra_indirection}{variable_size}, sizeof({array_type_to_c_str(inner_type, module_name)}));
                            if ({variable_name} == NULL) {{
                                return {wlm_prefix}_RESULT_MALLOC_FAIL;
                            }}
                            READ_ARRAY({variable_name}, object->{extra_indirection}{variable_size}, {inner}{extra});
                        """)

                    case model.ArraySizeEndless():
                        s.wln(f"object->amount_of_{d.name} = 0;")
                        s.open_curly("/* C89 scope to allow variable declarations */")
                        s.wln("int i = 0;")
                        s.wln(f"size_t _current_size = 8 * sizeof(*{variable_name});")
                        s.newline()
                        s.wln(f"{variable_name} = malloc(_current_size);")
                        s.open_curly(f"if ({variable_name} == NULL)")
                        s.wln(f"_return_value = {wlm_prefix}_RESULT_MALLOC_FAIL;")
                        s.wln("goto cleanup;")
                        s.closing_curly()

                        if compressed or container.tags.compressed:
                            s.open_curly("while (reader->index < reader->length)")
                        else:
                            s.open_curly("while (_size < body_size)")
                        s.wln(f"{inner};")

                        if (needs_size or isinstance(array_size,
                                                     model.ArraySizeEndless)) and not compressed and not container.tags.compressed:
                            s.wln(
                                f"_size += {array_size_inner_action(inner_type, variable_name, 'i', module_name)};")

                        s.wln("++i;")
                        s.newline()

                        s.open_curly(f"if (i * sizeof(*{variable_name}) >= _current_size)")
                        s.wln(f"_current_size *= 2;")
                        s.wln(f"{variable_name} = realloc({variable_name}, _current_size);")
                        s.open_curly(f"if ({variable_name} == NULL)")
                        s.wln(f"free({variable_name});")
                        s.wln("_return_value = WWM_RESULT_MALLOC_FAIL;")
                        s.wln("goto cleanup;")
                        s.closing_curly()
                        s.closing_curly()  # if (i * sizeof)

                        s.closing_curly()  # while
                        s.newline()

                        s.wln(f"object->amount_of_{d.name} = i;")

                        s.closing_curly()  # C89 scope
                    case v:
                        raise Exception(f"{v}")

        case v:
            raise Exception(f"{v}")

    s.newline()


def print_read_member(s: Writer, m: model.StructMember, container: model.Container, needs_size: bool, module_name: str,
                      extra_indirection: str):
    match m:
        case model.StructMemberDefinition(_tag, definition):
            compressed = container.tags.compressed is not None and container.tags.compressed
            print_read_struct_member(s, definition, needs_size, compressed, module_name, extra_indirection, container)

        case model.StructMemberIfStatement(_tag, statement):
            print_read_if_statement(s, statement, container, False, needs_size, module_name, extra_indirection)

        case _:
            raise Exception("invalid struct member")


def print_read_if_statement(
        s: Writer,
        statement: model.IfStatement,
        container: model.Container,
        is_else_if: bool,
        needs_size: bool,
        module_name: str,
        extra_indirection: str,
):
    extra_elseif = ""
    if is_else_if:
        extra_elseif = "else "

    print_if_statement_header(s, statement, extra_elseif, "", module_name)

    for member in statement.members:
        print_read_member(s, member, container, needs_size, module_name, extra_indirection)

    s.closing_curly()  # if

    for elseif in statement.else_if_statements:
        print_read_if_statement(s, elseif, container, True, needs_size, module_name, extra_indirection)


def print_read(s: Writer, container: Container, module_name: str):
    if not container_has_c_members(container):
        return

    if container.name == "Addon" and (module_name == "wrath" or module_name == "tbc"):
        return

    wlm_prefix = "WWM" if is_world(container.tags) else "WLM"
    needs_size = container_needs_size_in_read(container)

    body_size = ", size_t body_size" if container_needs_size_in_read(container) else ""

    if is_cpp():
        s.open_curly(f"{container.name} {container.name}_read(Reader& reader{body_size})")
        s.wln(f"{container.name} obj{{}};")
        if needs_size:
            s.wln("size_t _size = 0;")
        s.newline()

        if container.tags.compressed:
            s.write_block("""
                const auto decompressed_size = reader.read_u32();

                if (decompressed_size == 0) {
                    return obj;
                }

                auto compressed_data = std::vector<unsigned char>{};

                for (size_t i = 0; i < (body_size - 4); ++i)
                {
                   compressed_data.push_back(reader.read_u8());
                }

                auto decompressed_data = ::wow_world_messages::util::decompress_data(compressed_data);
                auto byte_reader = ByteReader(decompressed_data);
            """)

        for m in container.members:
            print_read_member(s, m, container, needs_size, module_name, "")

        if container.optional is not None:
            s.open_curly("if (_size < body_size)")
            ty = f"{module_name}::{container.name}::{snake_case_to_pascal_case(container.optional.name)}"
            s.wln(
                f"obj.{container.optional.name} = std::shared_ptr<{ty}>(new {ty}());")
            s.newline()

            for member in container.optional.members:
                print_read_member(s, member, container, False, module_name, f"{container.optional.name}->")

            s.closing_curly()

        if container.tags.compressed:
            s.wln("(void)_size;")
        s.wln("return obj;")

        s.closing_curly()  # * *_read(Reader&)
        s.newline()
    else:
        export = "" if is_world(container.tags) and module_name == "all" else "static "

        result_type = get_type_prefix(container.tags)
        s.open_curly(
            f"{export}{result_type}Result {module_name}_{container.name}_read({result_type}Reader* reader, {container_module_prefix(container.tags, module_name)}_{container.name}* object{body_size})")

        s.wln("int _return_value = 1;")
        s.newline()

        if needs_size:
            s.wln("size_t _size = 0;")
            s.newline()

        container_has_compressed_array = False
        for d, _ in all_members_from_container(container):
            match d.data_type:
                case model.DataTypeArray(compressed=compressed):
                    if compressed:
                        container_has_compressed_array = True
                        s.wln(f"unsigned char* {d.name}_decompressed_data = NULL;")
                        s.wln(f"unsigned char* {d.name}_compressed_data = NULL;")
                        s.wln(f"uint32_t {d.name}_decompressed_size;")
                        s.newline()

        if container_has_compressed_array:
            s.wln("WowWorldReader new_reader;")
            s.newline()

        if container.tags.compressed and not is_cpp():
            s.write_block("""
                WowWorldReader stack_reader;
                unsigned char* _compressed_data = NULL;
                uint32_t _decompressed_size;
            """)

        if container.sizes.minimum_size != 0:
            s.write_block(f"""
                if ({container.sizes.minimum_size} > (reader->length - reader->index)) {{
                    _return_value = (int)({container.sizes.minimum_size} - (reader->length - reader->index));
                    goto cleanup;
                }}
            """)

        needs_newline = False
        free_body_members = Writer()

        if container.optional:
            s.wln(f"object->{container.optional.name} = NULL;")

        def write_set_null_for_datatype(writer: Writer, data_type: model.DataType, var_name: str):
            match data_type:
                case model.DataTypeArray(size=model.ArraySizeFixed(size=fixed_size), inner_type=inner_type):
                    if type(inner_type) is model.ArrayTypeCstring:
                        for i in range(0, int(fixed_size)):
                            writer.wln(f"{var_name}[{i}] = NULL;")
                case model.DataTypeMonsterMoveSpline():
                    writer.wln(f"{var_name}.amount_of_splines = 0;")
                    writer.wln(f"{var_name}.splines = NULL;")
                case model.DataTypeNamedGUID():
                    writer.wln(f"{var_name}.guid = 0;")
                    writer.wln(f"{var_name}.name = NULL;")
                case model.DataTypeAddonArray():
                    writer.wln(f"{var_name}.amount_of_addons = 0;")
                    writer.wln(f"{var_name}.addons = NULL;")
                case model.DataTypeAchievementInProgressArray() | model.DataTypeAchievementDoneArray():
                    writer.wln(f"{var_name}.amount_of_achievements = 0;")
                    writer.wln(f"{var_name}.achievements = NULL;")
                case model.DataTypeStruct():
                    pass  # their own functions null init and clean up
                case _:
                    writer.wln(f"{var_name} = NULL;")

        for d, extra_indirection in all_members_from_container(container):
            if extra_indirection != "":
                continue
            variable_name = f"object->{d.name}"
            if print_free_struct_member(free_body_members, d, module_name, ""):
                needs_newline = True
                write_set_null_for_datatype(s, d.data_type, variable_name)

        if needs_newline:
            s.newline()

        if container.tags.compressed and not is_cpp():
            s.write_block("""
                READ_U32(_decompressed_size);
                _size += 4;

                _compressed_data = malloc(_decompressed_size);
                if (_compressed_data == NULL) {
                    _return_value = WWM_RESULT_MALLOC_FAIL;
                    goto cleanup;
                }

                if (!wwm_decompress_data(&reader->source[reader->index], body_size - _size, _compressed_data, _decompressed_size)) {
                    return WWM_RESULT_COMPRESSION_ERROR;
                }

                stack_reader = wwm_create_reader(_compressed_data, _decompressed_size);
                reader = &stack_reader;
            """)

        for m in container.members:
            print_read_member(s, m, container, needs_size, module_name, "")

        if container.optional is not None:
            s.wln(f"object->{container.optional.name} = NULL;")
            s.open_curly("if (_size < body_size)")
            s.wln(
                f"object->{container.optional.name} = malloc(sizeof({module_name}_{container.name}_{container.optional.name}));")
            s.open_curly(f"if (object->{container.optional.name} == NULL)")
            s.wln("_return_value = WWM_RESULT_MALLOC_FAIL;")
            s.wln("goto cleanup;")
            s.closing_curly()
            s.newline()

            needs_newline = False
            for m in container.optional.members:
                match m:
                    case model.StructMemberDefinition(struct_member_content=d):
                        if print_free_struct_member(free_body_members, d, module_name, f"{container.optional.name}->"):
                            needs_newline = True
                            write_set_null_for_datatype(s, d.data_type, f"object->{container.optional.name}->{d.name}")

            if needs_newline:
                s.newline()

            for member in container.optional.members:
                print_read_member(s, member, container, False, module_name, f"{container.optional.name}->")

            s.closing_curly()

        for d, _ in all_members_from_container(container):
            match d.data_type:
                case model.DataTypeArray(compressed=compressed):
                    if compressed:
                        s.wln(f"free({d.name}_decompressed_data);")
                        s.wln(f"free({d.name}_compressed_data);")
                        s.closing_curly()  # if ({} compressed_data)
                        s.newline()

        if container.tags.compressed and not is_cpp():
            s.wln("free(_compressed_data);")
            s.newline()

        if is_world(container.tags):
            s.wln("return WWM_RESULT_SUCCESS;")
        else:
            s.wln("return WLM_RESULT_SUCCESS;")

        s.wln_no_indent(f"cleanup:")
        if container_has_free(container, module_name):
            s.wln(f"{module_name}_{container.name}_free(object);")

        s.wln("return _return_value;")

        s.closing_curly()  # enum Result
        s.newline()
