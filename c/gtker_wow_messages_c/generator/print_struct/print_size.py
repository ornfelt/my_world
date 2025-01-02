import typing

import model
from print_struct.print_write import print_array_inner_write, array_size_inner_action
from print_struct.struct_util import (
    integer_type_to_size,
    print_if_statement_header, container_should_have_size_function, container_module_prefix)

from util import is_cpp
from writer import Writer


def print_size(s: Writer, container: model.Container, module_name: str):
    if not container_should_have_size_function(container):
        return

    if is_cpp():
        s.open_curly(f"static size_t {container.name}_size(const {container.name}& obj)")
    else:
        s.open_curly(
            f"static size_t {module_name}_{container.name}_size(const {container_module_prefix(container.tags, module_name)}_{container.name}* object)")

    print_size_until_inner_members(s, container.members, container.manual_size_subtraction,
                                   True, container.optional is not None, module_name, "")
    if container.optional is not None:
        deref = "obj." if is_cpp() else "object->"
        s.open_curly(
            f"if({deref}{container.optional.name})" if is_cpp() else f"if({deref}{container.optional.name})")
        print_size_until_inner_members(s, container.optional.members, container.manual_size_subtraction, False,
                                       False, module_name, f"{container.optional.name}->")
        s.closing_curly()  # optional_statement_header
        s.newline()

        s.wln("return _size;")

    s.closing_curly()
    s.newline()


def print_size_until_inner_members(s: Writer, members: list[model.StructMember],
                                   manual_size_subtraction: typing.Optional[int], return_early: bool,
                                   has_optional: bool, module_name: str, extra_indirection: str):
    count, strings, uncounted_members = get_size_and_remaining_members(members, module_name, extra_indirection)

    if manual_size_subtraction is not None:
        count -= manual_size_subtraction

    size = str(count)
    if len(strings) != 0:
        size += f" + {' + '.join(strings)}"

    if len(uncounted_members) == 0 and return_early and not has_optional:
        s.wln(f"return {size};")
    else:
        if not return_early:
            s.wln(f"_size += {size};")
        else:
            s.wln(f"size_t _size = {size};")

        if len(uncounted_members) != 0 or has_optional:
            s.newline()

        for m in uncounted_members:
            print_size_inner(s, m, module_name, extra_indirection, count)

        if return_early and not has_optional:
            s.wln(f"return _size;")

def array_size_inner_values(
        array: model.DataTypeArray, name: str, extra_indirection: str,
) -> typing.Union[str, int, None]:
    size = 0
    match array.inner_type:
        case model.ArrayTypeGUID():
            size = 8
        case model.ArrayTypeStruct(struct_data=e):
            if e.sizes.constant_sized:
                size = e.sizes.maximum_size
            else:
                return None
        case model.ArrayTypePackedGUID():
            return None
        case model.ArrayTypeCstring():
            return None
        case model.ArrayTypeInteger(integer_type=integer_type):
            size = integer_type_to_size(integer_type)
        case model.ArrayTypeSpell():
            size = 4
        case v:
            raise Exception(f"{v}")

    match array.size:
        case model.ArraySizeFixed(size=array_size):
            return size * int(array_size)
        case model.ArraySizeVariable(size=array_size):
            return f"{size} * obj.{extra_indirection}{name}.size()" if is_cpp() else f"{size} * object->{extra_indirection}{array_size}"
        case model.ArraySizeEndless():
            return f"{size} * obj.{extra_indirection}{name}.size()" if is_cpp() else f"{size} * object->{extra_indirection}amount_of_{name}"

        case v2:
            raise Exception(f"{v2}")


def addable_size_value(
        data_type: model.DataType, extra_self: str, name: str, module_name: str, extra_indirection: str
) -> typing.Optional[typing.Tuple[int, typing.Optional[str]]]:
    variable_name = f"object->{extra_indirection}{name}"
    if is_cpp():
        variable_name = f"obj.{extra_indirection}{name}"
    namespace = "::wow_world_messages::util::" if is_cpp() else ""

    match data_type:
        case model.DataTypeStruct(struct_data=e):
            if not e.sizes.constant_sized:
                return 0, f"{e.name}_size({variable_name})" if is_cpp() else f"{module_name}_{e.name}_size(&{variable_name})"
            else:
                return e.sizes.maximum_size, None
        case model.DataTypeString() | model.DataTypeCstring():
            return 1, f"{variable_name}.size()" if is_cpp() else f"STRING_SIZE({variable_name})"
        case model.DataTypeSizedCstring():
            # null byte is not included in cpp count
            return 5, f"{variable_name}.size()" if is_cpp() else f"STRING_SIZE({variable_name})"
        case model.DataTypePackedGUID():
            return 0, f"{namespace}wwm_packed_guid_size({variable_name})"
        case model.DataTypeAchievementInProgressArray():
            return 0, f"achievement_in_progress_array_size({variable_name})" if is_cpp() else f"{module_name}_achievement_in_progress_array_size(&{variable_name})"
        case model.DataTypeAchievementDoneArray():
            return 0, f"achievement_done_array_size({variable_name})" if is_cpp() else f"{module_name}_achievement_done_array_size(&{variable_name})"

        case model.DataTypeAddonArray():
            return 0, f"{variable_name}.size() * 8" if is_cpp() else f"{module_name}_addon_array_size(&{variable_name})"
        case model.DataTypeVariableItemRandomProperty():
            return 0, f"{namespace}wwm_variable_item_random_property_size({variable_name})" if is_cpp() else f"wwm_variable_item_random_property_size(&{variable_name})"
        case model.DataTypeNamedGUID():
            return 0, f"{namespace}wwm_named_guid_size({variable_name})" if is_cpp() else f"wwm_named_guid_size(&{variable_name})"
        case model.DataTypeInspectTalentGearMask():
            return 0, f"inspect_talent_gear_mask_size({variable_name})" if is_cpp() else f"{module_name}_inspect_talent_gear_mask_size(&{variable_name})"
        case model.DataTypeUpdateMask():
            return 0, f"{module_name}::update_mask_size({variable_name})" if is_cpp() else f"{module_name}_update_mask_size(&{variable_name})"
        case model.DataTypeMonsterMoveSpline():
            address_of = "" if is_cpp() else "&"
            return 0, f"{namespace}wwm_monster_move_spline_size({address_of}{variable_name})"
        case model.DataTypeEnchantMask():
            return 0, f"enchant_mask_size({variable_name})" if is_cpp() else f"{module_name}_enchant_mask_size(&{variable_name})"
        case model.DataTypeCacheMask():
            return 0, f"cache_mask_size({variable_name})" if is_cpp() else f"{module_name}_cache_mask_size(&{variable_name})"
        case model.DataTypeAuraMask():
            return 0, f"aura_mask_size({variable_name})" if is_cpp() else f"{module_name}_aura_mask_size(&{variable_name})"
        case model.DataTypeArray(compressed=compressed):
            if compressed:
                return None
            size = array_size_inner_values(data_type, name, extra_indirection)
            if isinstance(size, str):
                return 0, size
            elif isinstance(size, int):
                return size, None
        case model.DataTypeInteger(integer_type=integer_type):
            return integer_type_to_size(integer_type), None
        case model.DataTypeBool(integer_type=integer_type):
            return integer_type_to_size(integer_type), None
        case model.DataTypeEnum(integer_type=integer_type):
            return integer_type_to_size(integer_type), None
        case model.DataTypeFlag(integer_type=integer_type):
            return integer_type_to_size(integer_type), None

        case model.DataTypeSpell() | model.DataTypeItem() | model.DataTypeDateTime() \
             | model.DataTypeGold() \
             | model.DataTypeSeconds() \
             | model.DataTypeMilliseconds() \
             | model.DataTypeIPAddress():
            return 4, None
        case model.DataTypePopulation():
            return 4, None
        case model.DataTypeGUID():
            return 8, None
        case model.DataTypeLevel():
            return 1, None
        case model.DataTypeLevel16() | model.DataTypeSpell16():
            return 2, None
        case model.DataTypeLevel32():
            return 4, None
        case model.DataTypeFloatingPoint():
            return 4, None

        case v:
            raise Exception(f"unhandled ty {v}")

    return None


def get_size_and_remaining_members(members: list[model.StructMember], module_name: str, extra_indirection: str) -> \
        typing.Tuple[
            int, list[str], list[model.StructMember]]:
    count = 0
    strings = []
    uncounted_members: typing.List[model.StructMember] = []

    for m in members:
        match m:
            case model.StructMemberDefinition(struct_member_content=d):
                addable = addable_size_value(d.data_type, "self.", d.name, module_name, extra_indirection)
                if addable is not None:
                    addable_count, addable_string = addable
                    count += addable_count
                    if addable_string is not None:
                        strings.append(addable_string)
                else:
                    uncounted_members.append(m)
            case model.StructMemberIfStatement():
                uncounted_members.append(m)
            case v:
                raise Exception(f"{v}")

    return count, strings, uncounted_members


def print_size_inner(s: Writer, m: model.StructMember, module_name: str, extra_indirection: str, size_of_other_fields: int):
    extra_self = "self."

    match m:
        case model.StructMemberDefinition(struct_member_content=d):
            match d.data_type:
                case model.DataTypeArray(compressed=compressed, inner_type=inner_type, size=size):
                    if compressed and not is_cpp():
                        s.open_curly("/* C89 scope for compressed size */")
                        s.wln(f"unsigned char {d.name}_uncompressed_data[{0xFFFF - 4 - size_of_other_fields}];")

                        s.wln(f"uint32_t compressed_i;")
                        s.wln(f"size_t compressed_size = 0;")
                        s.newline()
                        s.wln("WowWorldWriter stack_writer;")
                        s.wln("WowWorldWriter* writer = &stack_writer;")
                        s.newline()

                        s.open_curly(
                            f"for(compressed_i = 0; compressed_i < object->amount_of_{d.name}; ++compressed_i)")
                        s.wln(
                            f"compressed_size += {array_size_inner_action(inner_type, f'object->{d.name}', 'compressed_i', module_name)};")
                        s.closing_curly()  # for compressed i
                        s.newline()

                        s.open_curly("if (compressed_size)")

                        s.wln("int _return_value = 1;")
                        s.wln(f"stack_writer = wwm_create_writer({d.name}_uncompressed_data, compressed_size);")

                        print_array_inner_write(d, extra_indirection, inner_type, s, size, f"object->{d.name}",
                                                module_name)
                        s.newline()

                        s.wln_no_indent("cleanup:")
                        s.wln(
                            f"_size += wwm_compress_data_size({d.name}_uncompressed_data, compressed_size);")
                        s.newline()

                        s.closing_curly()  # if (compressed_size)

                        s.closing_curly()  # C89 scope for compressed size

                    elif compressed and is_cpp():
                        s.wln("auto writer = Writer(0);")
                        s.open_curly(f"for (const auto& v : obj.{d.name})")
                        print_array_inner_write(d, extra_indirection, inner_type, s, size, f"obj.{d.name}", module_name)
                        s.closing_curly()
                        s.newline()

                        s.open_curly("if (!writer.m_buf.empty())")
                        s.wln("_size += ::wow_world_messages::util::compress_data(writer.m_buf).size();")
                        s.closing_curly()  # if (writer.m_buf.size())
                    else:
                        print_size_for_array(s, d, extra_indirection, inner_type, size, module_name)
                case v:
                    raise Exception(f"{v}")

        case model.StructMemberIfStatement(struct_member_content=statement):
            print_size_if_statement(s, statement, False, module_name, extra_indirection)

        case v:
            raise Exception(f"{v}")

    s.newline()


def print_size_for_array(s: Writer, d: model.Definition, extra_indirection: str, inner_type: model.ArrayType,
                         size: model.ArraySize, module_name: str):
    variable_name = f"obj{extra_indirection}.{d.name}" if is_cpp() else f"object{extra_indirection}->{d.name}"
    match size:
        case model.ArraySizeFixed(size=ssize):
            loop_max = ssize
        case model.ArraySizeVariable(size=ssize):
            loop_max = f"(int)object->{extra_indirection}{ssize}"
        case model.ArraySizeEndless():
            loop_max = f"(int)object->amount_of_{d.name}"
        case _:
            raise Exception("invalid size")
    if is_cpp():
        s.open_curly(f"for(const auto& v : {variable_name})")
    else:
        s.open_curly("/* C89 scope to allow variable declarations */")
        s.wln("int i;")
        s.open_curly(f"for(i = 0; i < {loop_max}; ++i)")

    s.wln(f"_size += {array_size_inner_action(inner_type, variable_name, 'i', module_name)};")

    if not is_cpp():
        s.closing_curly()  # C89 scope
    s.closing_curly()  # array scope


def print_size_if_statement(s: Writer, statement: model.IfStatement, is_else_if: bool, module_name: str,
                            extra_indirection: str):
    extra_elseif = ""
    if is_else_if:
        extra_elseif = "else "

    print_if_statement_header(s, statement, extra_elseif, extra_indirection, module_name)

    print_size_until_inner_members(s, statement.members, None, False, False, module_name, extra_indirection)

    s.closing_curly()  # if

    for elseif in statement.else_if_statements:
        print_size_if_statement(s, elseif, True, module_name, extra_indirection)
