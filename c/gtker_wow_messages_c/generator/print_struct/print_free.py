from print_struct.struct_util import print_if_statement_header, all_members_from_container, \
    container_module_prefix
from util import get_export_define

import model
from writer import Writer


def container_has_free(e: model.Container, module_name: str) -> bool:
    writer = Writer()

    if e.optional is not None:
        return True


    for d, _ in all_members_from_container(e):
        if print_free_struct_member(writer, d, module_name, ""):
            return True

    return False


def print_free_for_container(s: Writer, h: Writer, e: model.Container, module_name):
    if not container_has_free(e, module_name):
        return

    export = get_export_define(e.tags)
    function_declaration = f"{export} void {module_name}_{e.name}_free({module_name}_{e.name}* object)"

    match e.object_type:
        case model.ObjectTypeStruct():
            pass
        case _:
            h.wln(f"{function_declaration};")

    s.open_curly(function_declaration)

    needs_loop = False
    for d, _ in all_members_from_container(e):
        match d.data_type:
            case model.DataTypeArray(inner_type=inner_type):
                match inner_type:
                    case model.ArrayTypeStruct(struct_data=struct_data):
                        if container_has_free(struct_data, module_name):
                            needs_loop = True
                    case model.ArrayTypeCstring():
                        needs_loop = True

    if needs_loop:
        s.wln("size_t i;")
        s.newline()

    print_free_function_body(s, e, module_name)

    s.closing_curly()  # function_declaration
    s.newline()


def print_free_function_body(s: Writer, e: model.Container, module_name: str):
    for m in e.members:
        print_free_member(s, m, module_name, "")
    if e.optional is not None:
        s.open_curly(f"if (object->{e.optional.name} != NULL)")
        for m in e.optional.members:
            print_free_member(s, m, module_name, f"{e.optional.name}->")

        s.wln(f"free(object->{e.optional.name});")
        s.wln(f"object->{e.optional.name} = NULL;")
        s.closing_curly()


def print_free_member(s: Writer, m: model.StructMember, module_name: str, extra_indirection: str):
    match m:
        case model.StructMemberDefinition(_tag, definition):
            print_free_struct_member(s, definition, module_name, extra_indirection)

        case model.StructMemberIfStatement(_tag, statement):
            print_free_if_statement(s, statement, False, module_name, extra_indirection)

        case _:
            raise Exception("invalid struct member")


def print_free_if_statement(s: Writer, statement: model.IfStatement, is_else_if: bool, module_name: str,
                            extra_indirection: str):
    extra_elseif = ""
    if is_else_if:
        extra_elseif = "else "

    print_if_statement_header(s, statement, extra_elseif, extra_indirection, module_name)

    for m in statement.members:
        print_free_member(s, m, module_name, extra_indirection)

    s.closing_curly()

    for elseif in statement.else_if_statements:
        print_free_if_statement(s, elseif, True, module_name, extra_indirection)


def print_free_struct_member(s: Writer, d: model.Definition, module_name: str, extra_indirection: str) -> bool:
    variable_name = f"object->{extra_indirection}{d.name}"
    match d.data_type:
        case model.DataTypeInteger(integer_type=integer_type):
            pass

        case model.DataTypeBool(integer_type=integer_type):
            pass

        case model.DataTypeFlag(
            type_name=type_name, integer_type=integer_type
        ):
            pass

        case model.DataTypeEnum(integer_type=integer_type, type_name=type_name):
            pass

        case model.DataTypeString():
            s.wln(f"FREE_STRING({variable_name});")
            s.newline()
            return True

        case model.DataTypeCstring():
            s.wln(f"FREE_STRING({variable_name});")
            s.newline()
            return True

        case model.DataTypeSizedCstring():
            s.wln(f"FREE_STRING({variable_name});")
            s.newline()
            return True

        case model.DataTypeLevel32() | model.DataTypeSpell() | model.DataTypeItem() \
             | model.DataTypeDateTime() | model.DataTypeGold() | model.DataTypeSeconds() \
             | model.DataTypeMilliseconds() | model.DataTypeIPAddress() | model.DataTypePopulation():
            pass

        case model.DataTypeGUID():
            pass

        case model.DataTypeLevel():
            pass

        case model.DataTypeLevel16() | model.DataTypeSpell16():
            pass

        case model.DataTypePackedGUID():
            pass

        case model.DataTypeFloatingPoint():
            pass

        case model.DataTypeStruct(struct_data=e):
            if container_has_free(e, module_name):
                version = container_module_prefix(e.tags, module_name)

                s.wln(f"{version}_{e.name}_free(&{variable_name});")
                return True

        case model.DataTypeUpdateMask():
            pass

        case model.DataTypeAuraMask():
            pass

        case model.DataTypeVariableItemRandomProperty():
            pass

        case model.DataTypeMonsterMoveSpline():
            s.wln(f"wwm_monster_move_spline_free(&{variable_name});")
            s.newline()
            return True

        case model.DataTypeNamedGUID():
            s.wln(f"wwm_named_guid_free(&{variable_name});")
            s.newline()
            return True

        case model.DataTypeEnchantMask():
            pass

        case model.DataTypeInspectTalentGearMask():
            pass

        case model.DataTypeCacheMask():
            pass

        case model.DataTypeAddonArray():
            s.wln(f"free({variable_name}.addons);")
            return True

        case model.DataTypeAchievementDoneArray():
            s.wln(f"{module_name}_achievement_done_array_free(&{variable_name});")
            return True

        case model.DataTypeAchievementInProgressArray():
            s.wln(f"{module_name}_achievement_in_progress_array_free(&{variable_name});")
            return True

        case model.DataTypeArray(inner_type=inner_type, size=size):
            match size:
                case model.ArraySizeFixed(size=loop_size):
                    loop_variable = loop_size
                    can_be_null = False
                case model.ArraySizeVariable(size=variable_size):
                    loop_variable = f"object->{variable_size}"
                    can_be_null = True
                case model.ArraySizeEndless():
                    loop_variable = f"object->amount_of_{d.name}"
                    can_be_null = True
                case _:
                    raise Exception("invalid size")

            if can_be_null:
                s.open_curly(f"if ({variable_name} != NULL)")

            has_free = False
            match inner_type:
                case model.ArrayTypeStruct(struct_data=struct_data):
                    if container_has_free(struct_data, module_name):
                        s.open_curly(f"for (i = 0; i < {loop_variable}; ++i)")
                        s.wln(
                            f"{container_module_prefix(struct_data.tags, module_name)}_{struct_data.name}_free(&(({variable_name})[i]));")
                        s.closing_curly()  # for int i
                        has_free = True
                case model.ArrayTypeCstring():
                    s.open_curly(f"for (i = 0; i < {loop_variable}; ++i)")
                    s.wln(f"FREE_STRING((({variable_name})[i]));")
                    s.closing_curly()  # for int i
                    has_free = True

            if type(size) is not model.ArraySizeFixed:
                s.wln(f"free({variable_name});")
                s.wln(f"{variable_name} = NULL;")
                has_free = True

            if can_be_null:
                s.closing_curly()

            return has_free

    return False
