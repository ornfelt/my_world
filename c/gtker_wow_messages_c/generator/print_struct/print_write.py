import typing

from print_struct import container_has_c_members

import model
from model import Container
from print_struct.struct_util import print_if_statement_header, \
    integer_type_to_size, integer_type_to_short, integer_type_to_c_str, all_members_from_container, \
    container_module_prefix
from util import container_is_unencrypted, first_version_as_module, login_version_matches, is_world, get_type_prefix, \
    get_export_define, is_cpp
from writer import Writer


def print_write_struct_member(s: Writer, d: model.Definition, module_name: str, container_name: str,
                              extra_indirection: str):
    variable_name = f"object->{extra_indirection}{d.name}"
    if is_cpp():
        variable_name = f"obj.{extra_indirection}{d.name}"

    match d.data_type:
        case model.DataTypeInteger(integer_type=integer_type):
            c_type = integer_type_to_c_str(integer_type)
            if d.constant_value is not None:
                variable_name = d.constant_value.value
            if d.size_of_fields_before_size is not None:
                variable_name = f"({c_type}){module_name}_{container_name}_size(object)"
                if is_cpp():
                    variable_name = f"static_cast<{c_type}>({container_name}_size(obj))"
            if d.used_as_size_in is not None and is_cpp():
                variable_name = f"static_cast<{c_type}>(obj.{extra_indirection}{d.used_as_size_in}.size())"

            short = integer_type_to_short(integer_type)

            if is_cpp():
                s.wln(f"writer.write_{short}({variable_name});")
            else:
                s.wln(f"WRITE_{short.upper()}({variable_name});")

        case model.DataTypeBool(integer_type=integer_type):
            size = integer_type_to_size(integer_type)
            if is_cpp():
                s.wln(f"writer.write_bool{size * 8}({variable_name});")
            else:
                s.wln(f"WRITE_BOOL{size * 8}({variable_name});")

        case model.DataTypeFlag(
            type_name=type_name, integer_type=integer_type
        ) | model.DataTypeEnum(integer_type=integer_type, type_name=type_name):
            short = integer_type_to_short(integer_type)
            c_type = integer_type_to_c_str(integer_type)
            if is_cpp():
                s.wln(f"writer.write_{short}(static_cast<{c_type}>({variable_name}));")
            else:
                s.wln(f"WRITE_{short.upper()}({variable_name});")

        case model.DataTypeString():
            if is_cpp():
                s.wln(f"writer.write_string({variable_name});")
            else:
                s.wln(f"WRITE_STRING({variable_name});")

        case model.DataTypeCstring():
            if is_cpp():
                s.wln(f"writer.write_cstring({variable_name});")
            else:
                s.wln(f"WRITE_CSTRING({variable_name});")

        case model.DataTypeSizedCstring():
            if is_cpp():
                s.wln(f"writer.write_sized_cstring({variable_name});")
            else:
                s.wln(f"WRITE_SIZED_CSTRING({variable_name});")

        case model.DataTypeLevel32() | model.DataTypeSpell() | model.DataTypeItem() \
             | model.DataTypeDateTime() | model.DataTypeGold() | model.DataTypeSeconds() \
             | model.DataTypeMilliseconds() | model.DataTypeIPAddress():
            if is_cpp():
                s.wln(f"writer.write_u32({variable_name});")
            else:
                s.wln(f"WRITE_U32({variable_name});")

        case model.DataTypeGUID():
            if is_cpp():
                s.wln(f"writer.write_u64({variable_name});")
            else:
                s.wln(f"WRITE_U64({variable_name});")

        case model.DataTypeLevel():
            if is_cpp():
                s.wln(f"writer.write_u8({variable_name});")
            else:
                s.wln(f"WRITE_U8({variable_name});")

        case model.DataTypeLevel16() | model.DataTypeSpell16():
            if is_cpp():
                s.wln(f"writer.write_u16({variable_name});")
            else:
                s.wln(f"WRITE_U16({variable_name});")

        case model.DataTypePackedGUID():
            if is_cpp():
                s.wln(f"writer.write_packed_guid({variable_name});")
            else:
                s.wln(f"WRITE_PACKED_GUID({variable_name});")

        case model.DataTypeFloatingPoint() | model.DataTypePopulation():
            if is_cpp():
                s.wln(f"writer.write_float({variable_name});")
            else:
                s.wln(f"WRITE_FLOAT({variable_name});")

        case model.DataTypeStruct(struct_data=e):
            if is_cpp():
                s.wln(f"{e.name}_write(writer, {variable_name});")
            else:
                version = container_module_prefix(e.tags, module_name)
                wlm_prefix = "WWM" if is_world(e.tags) else "WLM"

                s.wln(f"{wlm_prefix}_CHECK_RETURN_CODE({version}_{e.name}_write(writer, &object->{d.name}));")

        case model.DataTypeUpdateMask():
            if is_cpp():
                s.wln(f"{module_name}::update_mask_write(writer, {variable_name});")
            else:
                s.wln(f"{module_name}_update_mask_write(writer, &{variable_name});")

        case model.DataTypeAuraMask():
            if is_cpp():
                s.wln(f"aura_mask_write(writer, {variable_name});")
            else:
                s.wln(f"WWM_CHECK_RETURN_CODE({module_name}_aura_mask_write(writer, &{variable_name}));")

        case model.DataTypeMonsterMoveSpline():
            if is_cpp():
                s.wln(f"::wow_world_messages::util::wwm_write_monster_move_spline(writer, {variable_name});")
            else:
                s.wln(f"WRITE_MONSTER_MOVE_SPLINE({variable_name});")

        case model.DataTypeEnchantMask():
            if is_cpp():
                s.wln(f"{module_name}::enchant_mask_write(writer, {variable_name});")
            else:
                s.wln(f"{module_name}_enchant_mask_write(writer, &{variable_name});")

        case model.DataTypeNamedGUID():
            if is_cpp():
                s.wln(f"::wow_world_messages::util::wwm_write_named_guid(writer, {variable_name});")
            else:
                s.wln(f"WRITE_NAMED_GUID({variable_name});")

        case model.DataTypeInspectTalentGearMask():
            if is_cpp():
                s.wln(f"{module_name}::inspect_talent_gear_mask_write(writer, {variable_name});")
            else:
                s.wln(f"{module_name}_inspect_talent_gear_mask_write(writer, &{variable_name});")

        case model.DataTypeVariableItemRandomProperty():
            if is_cpp():
                s.wln(f"::wow_world_messages::util::wwm_write_variable_item_random_property(writer, {variable_name});")
            else:
                s.wln(f"WRITE_VARIABLE_ITEM_RANDOM_PROPERTY({variable_name});")

        case model.DataTypeCacheMask():
            if is_cpp():
                s.wln(f"{module_name}::cache_mask_write(writer, {variable_name});")
            else:
                s.wln(f"{module_name}_cache_mask_write(writer, &{variable_name});")

        case model.DataTypeAddonArray():
            if is_cpp():
                s.wln(f"{module_name}::addon_array_write(writer, {variable_name});")
            else:
                s.wln(f"{module_name}_addon_array_write(writer, &{variable_name});")

        case model.DataTypeAchievementDoneArray():
            if is_cpp():
                s.wln(f"{module_name}::achievement_done_array_write(writer, {variable_name});")
            else:
                s.wln(f"{module_name}_achievement_done_array_write(writer, &{variable_name});")

        case model.DataTypeAchievementInProgressArray():
            if is_cpp():
                s.wln(f"{module_name}::achievement_in_progress_array_write(writer, {variable_name});")
            else:
                s.wln(f"{module_name}_achievement_in_progress_array_write(writer, &{variable_name});")

        case model.DataTypeArray(compressed=compressed, size=size, inner_type=inner_type):
            if is_cpp():
                if compressed:
                    s.wln("auto old_writer = writer;")
                    s.wln("writer = Writer(0);")
                    s.newline()

                s.open_curly(f"for (const auto& v : {variable_name})")
                print_array_inner_write(d, extra_indirection, inner_type, s, size, variable_name, module_name)
                s.closing_curly()  # for (const auto& v

                if compressed:
                    s.newline()
                    s.open_curly("if (!writer.m_buf.empty())")

                    s.wln(f"auto {d.name}_compressed_data = ::wow_world_messages::util::compress_data(writer.m_buf);")
                    s.wln(f"old_writer.write_u32(static_cast<uint32_t>({d.name}_compressed_data.size()));")
                    s.newline()

                    s.open_curly(f"for (const auto v : {d.name}_compressed_data)")
                    s.wln("old_writer.write_u8(v);")
                    s.closing_curly()  # for (const auto v )

                    s.closing_curly()  # if (!writer.m_buf.empty())

                    s.open_curly("else")
                    s.wln("old_writer.write_u32(0);")
                    s.closing_curly()  # else
                    s.newline()

                    s.wln("writer = old_writer;")

            else:
                if compressed:
                    s.open_curly("/* C89 Scope for compressed */")
                    s.wln(f"unsigned char* {d.name}_uncompressed_data = NULL;")
                    s.newline()
                    s.wln("uint32_t compressed_i;")
                    s.wln("size_t _size = 0;")
                    s.wln("WowWorldWriter new_writer;")
                    s.wln("WowWorldWriter* old_writer = writer;")
                    s.newline()

                    s.open_curly(
                        f"for(compressed_i = 0; compressed_i < object->amount_of_{extra_indirection}{d.name}; ++compressed_i)")
                    s.wln(
                        f"_size += {array_size_inner_action(inner_type, f'object->{extra_indirection}{d.name}', 'compressed_i', module_name)};")
                    s.closing_curly()  # for

                    s.open_curly("if (_size)")
                    s.wln(f"size_t {d.name}_compressed_data_size;")

                    s.wln("WRITE_U32((uint32_t)_size);")
                    s.newline()

                    s.wln(f"{d.name}_uncompressed_data = malloc(_size);")
                    s.open_curly(f"if ({d.name}_uncompressed_data == NULL)")
                    s.wln("_return_value = WWM_RESULT_MALLOC_FAIL;")
                    s.wln("goto cleanup;")
                    s.closing_curly()
                    s.wln(f"new_writer = wwm_create_writer({d.name}_uncompressed_data, _size);")
                    s.wln("writer = &new_writer;")

                print_array_inner_write(d, extra_indirection, inner_type, s, size, variable_name, module_name)

                if compressed:
                    s.newline()
                    s.wln(
                        f"{d.name}_compressed_data_size = wwm_compress_data({d.name}_uncompressed_data, _size, &old_writer->destination[old_writer->index], old_writer->length - old_writer->index);")
                    s.open_curly(
                        f"if ({d.name}_compressed_data_size == 0)")
                    s.wln("return WWM_RESULT_COMPRESSION_ERROR;")
                    s.closing_curly()
                    s.wln(f"old_writer->index += {d.name}_compressed_data_size;")
                    s.wln(f"free({d.name}_uncompressed_data);")

                    s.closing_curly()  # if (_size)
                    s.closing_curly()  # C89 scope for compressed

    s.newline()


def array_size_inner_action(inner_type: model.ArrayType, variable_name: str, index_variable: str, module_name) -> str:
    match inner_type:
        case model.ArrayTypeInteger(integer_type=integer_type):
            return f"{integer_type_to_size(integer_type)}"

        case model.ArrayTypeSpell():
            return "4"

        case model.ArrayTypeStruct(struct_data=e):
            version = container_module_prefix(e.tags, module_name)
            size = f"{version}_{e.name}_size(&{variable_name}[{index_variable}])" if not e.sizes.constant_sized else str(
                e.sizes.maximum_size)
            return f"{e.name}_size(v)" if is_cpp() and not e.sizes.constant_sized else size

        case model.ArrayTypeCstring():
            return f"v.size() + 1" if is_cpp() else f"STRING_SIZE({variable_name}[{index_variable}]) + 1"

        case model.ArrayTypePackedGUID():
            return f"wow_world_messages::util::wwm_packed_guid_size(v)" if is_cpp() else f"wwm_packed_guid_size({variable_name}[{index_variable}])"

        case _:
            raise Exception(f"array size unknown type {inner_type}")


def print_array_inner_write(d: model.Definition, extra_indirection: str, inner_type: model.ArrayType, s: Writer,
                            size: model.ArraySize, variable_name: str, module_name: str):
    if is_cpp():
        match inner_type:
            case model.ArrayTypeInteger(integer_type=integer_type):
                short = integer_type_to_short(integer_type)
                s.wln(f"writer.write_{short}(v);")

            case model.ArrayTypeSpell():
                s.wln(f"writer.write_u32(v);")

            case model.ArrayTypeGUID():
                s.wln(f"writer.write_u64(v);")

            case model.ArrayTypeStruct(struct_data=e):
                s.wln(f"{e.name}_write(writer, v);")

            case model.ArrayTypeCstring():
                s.wln(f"writer.write_cstring(v);")

            case model.ArrayTypePackedGUID():
                s.wln(f"writer.write_packed_guid(v);")

            case v2:
                raise Exception(f"{v2}")

    else:
        inner = ""
        match inner_type:
            case model.ArrayTypeInteger(integer_type=integer_type):
                short = integer_type_to_short(integer_type).upper()
                inner = f"WRITE_{short}({variable_name}[i])"

            case model.ArrayTypeSpell():
                inner = f"WRITE_U32({variable_name}[i])"

            case model.ArrayTypeGUID():
                inner = f"WRITE_U64({variable_name}[i])"

            case model.ArrayTypeStruct(struct_data=e):
                version = container_module_prefix(e.tags, module_name)
                wlm_prefix = "WWM" if is_world(e.tags) else "WLM"
                inner = f"{wlm_prefix}_CHECK_RETURN_CODE({version}_{e.name}_write(writer, &{variable_name}[i]))"

            case model.ArrayTypeCstring():
                inner = f"WRITE_CSTRING({variable_name}[i])"

            case model.ArrayTypePackedGUID():
                inner = f"WWM_CHECK_RETURN_CODE(wwm_write_packed_guid(writer, {variable_name}[i]))"

            case v2:
                raise Exception(f"{v2}")
        match size:
            case model.ArraySizeFixed(size=ssize):
                s.wln(f"WRITE_ARRAY({variable_name}, {ssize}, {inner});")
            case model.ArraySizeVariable(size=ssize):
                s.wln(f"WRITE_ARRAY({variable_name}, object->{extra_indirection}{ssize}, {inner});")
            case model.ArraySizeEndless():
                s.wln(
                    f"WRITE_ARRAY({variable_name}, object->{extra_indirection}amount_of_{d.name}, {inner});")
            case v:
                raise Exception(f"{v}")


def print_write(s: Writer, h: Writer, container: Container, object_type: model.ObjectType, module_name: str,
                extra_indirection: str):
    result_type = get_type_prefix(container.tags)
    export = "static " if type(object_type) is model.ObjectTypeStruct else f"{get_export_define(container.tags)} "
    if is_world(container.tags) and module_name == "all":
        export = ""
    function_suffix = f"_{object_type.container_type_tag.lower()}" if object_type != container.object_type else ""
    module_prefix = container_module_prefix(container.tags, module_name)
    function_declaration = f"{export}{result_type}Result {module_name}_{container.name}{function_suffix}_write({result_type}Writer* writer, const {module_prefix}_{container.name}* object)"
    if not container_has_c_members(container):
        function_declaration = f"{export}{result_type}Result {module_name}_{container.name}{function_suffix}_write({result_type}Writer* writer)"
    if is_cpp():
        if type(object_type) is model.ObjectTypeStruct:
            function_declaration = f"{export}void {container.name}_write{function_suffix}(Writer& writer, const {container.name}& obj)"
        else:
            extra = "const std::function<void(unsigned char*, size_t)>& encrypt" if is_world(container.tags) else ""
            function_declaration = f"{export}std::vector<unsigned char> {container.name}::write{function_suffix}({extra}) const"

    s.open_curly(function_declaration)
    if type(object_type) is not model.ObjectTypeStruct:
        if not is_cpp():
            h.wln(f"{function_declaration};")

    opcode_size: typing.Union[None, int] = None
    if is_cpp():
        size = f"{container.name}_size(obj)"

        if container.sizes.constant_sized:
            size = f"0x{container.sizes.maximum_size:04x}"

        if container.tags.compressed:
            size = "0 /* place holder */"

        if type(object_type) is not model.ObjectTypeStruct:
            if container_has_c_members(container):
                s.wln("const auto& obj = *this;")
            s.wln(f"auto writer = Writer({size});")
            s.newline()

        match object_type:
            case model.ObjectTypeClogin(opcode=opcode):
                s.wln(f"writer.write_u8(0x{opcode:02x}); /* opcode */")
                s.newline()
            case model.ObjectTypeSlogin(opcode=opcode):
                s.wln(f"writer.write_u8(0x{opcode:02x}); /* opcode */")
                s.newline()
            case model.ObjectTypeSmsg(opcode=opcode):
                s.wln(f"writer.write_u16_be(static_cast<uint16_t>({size} + 2)); /* size */")
                s.newline()
                s.wln(f"writer.write_u16(0x{opcode:08x}); /* opcode */")
                s.newline()
                opcode_size = 2
            case model.ObjectTypeCmsg(opcode=opcode):
                size = f"(uint16_t){container.name}_size(obj)"

                if container.sizes.constant_sized:
                    size = f"0x{container.sizes.maximum_size:04x}"

                s.wln(f"writer.write_u16_be(static_cast<uint16_t>({size} + 4)); /* size */")
                s.newline()
                s.wln(f"writer.write_u32(0x{opcode:08x}); /* opcode */")
                s.newline()
                opcode_size = 4

        if is_world(container.tags) and not type(object_type) is model.ObjectTypeStruct:
            s.wln("encrypt(writer.m_buf.data(), writer.m_buf.size());")
            s.newline()

        if container.tags.compressed:
            s.wln(f"writer.write_u32(static_cast<uint32_t>({container.name}_size(obj)));")
            s.newline()
            s.wln("auto old_writer = writer;")
            s.wln("writer = Writer(0);")


    else:
        wlm_prefix = "WWM" if is_world(container.tags) else "WLM"

        if container.tags.compressed:
            s.write_block(f"""
                WowWorldWriter* old_writer = writer;
                unsigned char* _decompressed_data;
                WowWorldWriter stack_writer;
                size_t _compressed_data_length;
                size_t saved_writer_index;
                const uint32_t _decompressed_data_length = (uint32_t){module_name}_{container.name}_size(object);
            """)
            s.newline()

        s.wln("int _return_value = 1;")
        s.newline()

        if type(container.object_type) is not model.ObjectTypeStruct:
            s.write_block(f"""
                if (writer->index > writer->length) {{
                    return {wlm_prefix}_RESULT_INVALID_PARAMETERS;
                }}
            """)
            s.newline()

        match object_type:
            case model.ObjectTypeClogin(opcode=opcode):
                s.wln(f"WRITE_U8(0x{opcode:02x}); /* opcode */")
                s.newline()
            case model.ObjectTypeSlogin(opcode=opcode):
                s.wln(f"WRITE_U8(0x{opcode:02x}); /* opcode */")
                s.newline()
            case model.ObjectTypeSmsg(opcode=opcode):
                size = f"(uint16_t){module_name}_{container.name}_size(object)"

                if container.sizes.constant_sized:
                    size = f"0x{container.sizes.maximum_size:04x}"
                if container.tags.compressed is not None:
                    size = "0 /* place holder */"

                s.wln(f"WRITE_U16_BE({size} + 2); /* size */")
                s.newline()
                s.wln(f"WRITE_U16(0x{opcode:08x}); /* opcode */")
                s.newline()
                opcode_size = 4

            case model.ObjectTypeCmsg(opcode=opcode):
                size = f"(uint16_t){module_name}_{container.name}_size(object)"

                if container.sizes.constant_sized:
                    size = f"0x{container.sizes.maximum_size:04x}"

                s.wln(f"WRITE_U16_BE({size} + 4); /* size */")
                s.newline()
                s.wln(f"WRITE_U32(0x{opcode:08x}); /* opcode */")
                s.newline()
                opcode_size = 2

        if container.tags.compressed:
            s.write_block(f"""
                WRITE_U32(_decompressed_data_length);
                writer = &stack_writer;
                
                if (_decompressed_data_length == 0) {{
                    return WWM_RESULT_SUCCESS;
                }}
                
                _decompressed_data = malloc(_decompressed_data_length);
                if (_decompressed_data == NULL) {{
                    _return_value = WWM_RESULT_MALLOC_FAIL;
                    goto cleanup;
                }}
                stack_writer = wwm_create_writer(_decompressed_data, _decompressed_data_length);
            """)

    for m in container.members:
        print_write_member(s, m, module_name, container.name, extra_indirection)

    if container.optional is not None:
        deref = "obj." if is_cpp() else "object->"
        s.open_curly(f"if({deref}{extra_indirection}{container.optional.name})")

        for m in container.optional.members:
            print_write_member(s, m, module_name, container.name, f"{container.optional.name}->")

        s.closing_curly()  # optional_statement_header

    if is_cpp():
        if type(object_type) is not model.ObjectTypeStruct:
            if container.tags.compressed:
                s.wln("const auto compressed_data = ::wow_world_messages::util::compress_data(writer.m_buf);")
                s.wln(
                    f"old_writer.write_u16_be_at_first_index(static_cast<uint16_t>(compressed_data.size() + 4 + {opcode_size}));")
                s.wln(
                    "old_writer.m_buf.insert(old_writer.m_buf.end(), compressed_data.begin(), compressed_data.end());")
                s.newline()

                s.wln("return old_writer.m_buf;")
            else:
                s.wln("return writer.m_buf;")
    else:
        if container.tags.compressed:
            s.write_block(f"""
                writer = old_writer;
                _compressed_data_length = wwm_compress_data(stack_writer.destination, stack_writer.length, &writer->destination[writer->index], writer->length - writer->index);
                writer->index += _compressed_data_length;
                saved_writer_index = writer->index;
                writer->index = 0;
                
                WRITE_U16_BE((uint16_t)(_compressed_data_length + 4 + {opcode_size})); /* size */
                
                writer->index = saved_writer_index;
                free(_decompressed_data);
            """)

        s.newline()
        if is_world(container.tags):
            s.wln("return WWM_RESULT_SUCCESS;")
        else:
            s.wln("return WLM_RESULT_SUCCESS;")

    if not is_cpp():
        s.wln_no_indent("cleanup: return _return_value;")

    s.closing_curly()
    s.newline()


def print_write_member(s: Writer, m: model.StructMember, module_name: str, container_name: str,
                       extra_indirection: str):
    match m:
        case model.StructMemberDefinition(_tag, definition):
            print_write_struct_member(s, definition, module_name, container_name, extra_indirection)

        case model.StructMemberIfStatement(_tag, statement):
            print_write_if_statement(s, statement, False, module_name, container_name, extra_indirection)

        case _:
            raise Exception("invalid struct member")


def print_write_if_statement(s: Writer,
                             statement: model.IfStatement,
                             is_else_if: bool,
                             module_name: str, container_name: str, extra_indirection: str):
    extra_elseif = ""
    if is_else_if:
        extra_elseif = "else "

    print_if_statement_header(s, statement, extra_elseif, extra_indirection, module_name)

    for m in statement.members:
        print_write_member(s, m, module_name, container_name, extra_indirection)

    s.closing_curly()  # if

    for elseif in statement.else_if_statements:
        print_write_if_statement(s, elseif, True, module_name, container_name, extra_indirection)
