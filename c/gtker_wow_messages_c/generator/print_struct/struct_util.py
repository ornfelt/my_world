import typing

import model
from util import pascal_case_to_snake_case, first_version_as_module, world_version_to_module_name, \
    version_to_module_name, is_cpp, is_world
from writer import Writer

SKIPS: typing.Dict[str, int] = {}


def print_skip(container: model.Container, reason: str):
    if reason in SKIPS:
        SKIPS[reason] += 1
    else:
        SKIPS[reason] = 1
    print(f"Skipping {'cpp' if is_cpp() else 'c'} {container.name} because it {reason}")


def container_uses_compression(container: model.Container) -> bool:
    if container.tags.compressed:
        return True

    if container_has_compressed_array(container):
        return True

    return False

def container_has_compressed_array(container: model.Container) -> bool:
    for d, _ in all_members_from_container(container):
        match d.data_type:
            case model.DataTypeArray(compressed=compressed):
                if compressed:
                    return True

    return False


def container_should_print(container: model.Container) -> bool:
    return True


def container_should_have_size_function(container: model.Container):
    match container.object_type:
        case model.ObjectTypeCmsg() | model.ObjectTypeSmsg() | model.ObjectTypeMsg() | model.ObjectTypeStruct():
            if container.sizes.constant_sized:
                return False
        case _:
            if container.sizes.constant_sized:
                return False
            if container.manual_size_subtraction is None and not is_cpp():
                return False

    return True


def integer_type_to_short(ty: model.IntegerType) -> str:
    prefix = "i" if integer_type_is_signed(ty) else "u"
    return f"{prefix}{integer_type_to_size(ty) * 8}"


def integer_type_is_signed(ty: model.IntegerType) -> bool:
    match ty:
        case model.IntegerType.I16:
            return True
        case model.IntegerType.I32:
            return True
        case model.IntegerType.I64:
            return True
        case model.IntegerType.I8:
            return True
        case model.IntegerType.U16:
            return False
        case model.IntegerType.U32:
            return False
        case model.IntegerType.U48:
            return False
        case model.IntegerType.U64:
            return False
        case model.IntegerType.U8:
            return False


def integer_type_to_size(ty: model.IntegerType) -> int:
    match ty:
        case model.IntegerType.I16:
            return 2
        case model.IntegerType.I32:
            return 4
        case model.IntegerType.I64:
            return 8
        case model.IntegerType.I8:
            return 1
        case model.IntegerType.U16:
            return 2
        case model.IntegerType.U32:
            return 4
        case model.IntegerType.U48:
            return 6
        case model.IntegerType.U64:
            return 8
        case model.IntegerType.U8:
            return 1


def container_has_c_members(e: model.Container) -> bool:
    for m, _ in all_members_from_container(e):
        if m.constant_value is None and m.used_as_size_in is None:
            return True

    return False


def integer_type_to_c_str(integer_type: model.IntegerType) -> str:
    prefix = "u"
    if integer_type_is_signed(integer_type):
        prefix = ""
    size = integer_type_to_size(integer_type) * 8
    if size == 6 * 8:
        size = 8 * 8
    return f"{prefix}int{size}_t"

def container_module_prefix(tags: model.ObjectTags, module_name: str) -> str:
    first_module = first_version_as_module(tags)
    version = module_name if (is_world(tags) and first_module != "all") else first_module
    return version


def type_to_c_str(ty: model.DataType, module_name: str) -> str:
    match ty:
        case model.DataTypeInteger(integer_type=integer_type):
            return integer_type_to_c_str(integer_type)
        case model.DataTypeBool():
            return "bool"

        case model.DataTypeString() | model.DataTypeCstring() | model.DataTypeSizedCstring():
            if is_cpp():
                return "std::string"
            else:
                return "char*"
        case model.DataTypeFloatingPoint():
            return "float"
        case model.DataTypeStruct(struct_data=e):
            version = container_module_prefix(e.tags, module_name)
            separator = "::" if is_cpp() else "_"
            return f"{version}{separator}{e.name}"
        case model.DataTypeEnum(type_name=type_name):
            return type_name if is_cpp() else f"{module_name}_{type_name}"
        case model.DataTypeFlag(type_name=type_name):
            return type_name if is_cpp() else f"{module_name}_{type_name}"

        case model.DataTypeArray(size=size, inner_type=inner_type):
            match size:
                case model.ArraySizeFixed():
                    return f"{array_type_to_c_str(inner_type, module_name)}"

            return f"std::vector<{array_type_to_c_str(inner_type, module_name)}>" if is_cpp() else f"{array_type_to_c_str(inner_type, module_name)}*"

        case model.DataTypeLevel():
            return "uint8_t"

        case model.DataTypeLevel16():
            return "uint16_t"
        case model.DataTypeSpell16():
            return "uint16_t"

        case model.DataTypeGold() | model.DataTypeIPAddress() | model.DataTypeLevel32() \
             | model.DataTypeSpell() | model.DataTypeItem() | model.DataTypeMilliseconds() \
             | model.DataTypeSeconds() | model.DataTypeDateTime():
            return "uint32_t"
        case model.DataTypeGUID() | model.DataTypePackedGUID():
            return "uint64_t"

        case model.DataTypePopulation():
            return "float"

        case model.DataTypeUpdateMask():
            return "UpdateMask" if is_cpp() else f"{module_name}_UpdateMask"

        case model.DataTypeAchievementDoneArray():
            return "std::vector<AchievementDone>" if is_cpp() else f"{module_name}_AchievementDoneArray"
        case model.DataTypeAchievementInProgressArray():
            return "std::vector<AchievementInProgress>" if is_cpp() else f"{module_name}_AchievementInProgressArray"
        case model.DataTypeAddonArray():
            return "std::vector<Addon>" if is_cpp() else f"{module_name}_AddonArray"
        case model.DataTypeAuraMask():
            return "AuraMask" if is_cpp() else f"{module_name}_AuraMask"

        case model.DataTypeEnchantMask():
            return "EnchantMask" if is_cpp() else f"{module_name}_EnchantMask"
        case model.DataTypeInspectTalentGearMask():
            return "InspectTalentGearMask" if is_cpp() else f"{module_name}_InspectTalentGearMask"
        case model.DataTypeMonsterMoveSpline():
            return "std::vector<::wow_world_messages::all::Vector3d>" if is_cpp() else "MonsterMoveSpline"
        case model.DataTypeNamedGUID():
            return "NamedGuid"
        case model.DataTypeVariableItemRandomProperty():
            return "VariableItemRandomProperty"
        case model.DataTypeCacheMask():
            return "CacheMask" if is_cpp() else f"{module_name}_CacheMask"
        case v:
            raise Exception(f"{v}")


def array_type_to_c_str(ty: model.ArrayType, module_name: str):
    match ty:
        case model.ArrayTypeCstring():
            return "std::string" if is_cpp() else "char*"
        case model.ArrayTypeGUID():
            return "uint64_t"
        case model.ArrayTypeInteger(integer_type=integer_type):
            return integer_type_to_c_str(integer_type)
        case model.ArrayTypeSpell():
            return "uint32_t"
        case model.ArrayTypePackedGUID():
            return "uint64_t"
        case model.ArrayTypeStruct(struct_data=struct_data):
            version = container_module_prefix(struct_data.tags, module_name)
            separator = "::" if is_cpp() else "_"
            return f"{version}{separator}{struct_data.name}"
        case v:
            raise Exception(f"{v}")


def all_members_from_container(
        container: model.Container,
) -> typing.List[typing.Tuple[model.Definition, str]]:
    out_members: typing.List[typing.Tuple[model.Definition, str]] = []

    def inner(m: model.StructMember, out_members: typing.List[typing.Tuple[model.Definition, str]], extra: str):
        def inner_if(
                statement: model.IfStatement, out_members: typing.List[typing.Tuple[model.Definition, str]]
        ):
            for member in statement.members:
                inner(member, out_members, extra)

            for elseif in statement.else_if_statements:
                inner_if(elseif, out_members)

        match m:
            case model.StructMemberDefinition(_tag, definition):
                out_members.append((definition, extra))

            case model.StructMemberIfStatement(_tag, struct_member_content=statement):
                inner_if(statement, out_members)

            case v:
                raise Exception(f"invalid struct member {v}")

    for m in container.members:
        inner(m, out_members, "")

    if container.optional is not None:
        for m in container.optional.members:
            inner(m, out_members, f"{container.optional.name}->")

    return out_members


def print_if_statement_header(
        s: Writer,
        statement: model.IfStatement,
        extra_elseif: str,
        extra_indirection: str,
        module_name: str
):
    original_type = type_to_c_str(statement.original_type, module_name).replace(f"{module_name}_", '')
    original_type_pascal = pascal_case_to_snake_case(original_type).upper()
    variable_name = f"obj.{extra_indirection}{statement.variable_name}" if is_cpp() else f"object->{extra_indirection}{statement.variable_name}"

    first_version = container_module_prefix(statement.original_type.tags, module_name).upper() # type: ignore[attr-defined]

    match statement.definer_type:
        case model.DefinerType.ENUM:
            if len(statement.values) == 1:
                if is_cpp():
                    s.open_curly(
                        f"{extra_elseif}if ({variable_name} == {original_type}::{statement.values[0]})"
                    )
                else:
                    s.open_curly(
                        f"{extra_elseif}if ({variable_name} == {first_version}_{original_type_pascal}_{statement.values[0]})"
                    )
            else:
                s.w(f"{extra_elseif}if (")
                for i, val in enumerate(statement.values):
                    if i != 0:
                        s.w_no_indent("|| ")
                    if is_cpp():
                        s.w_no_indent(
                            f"{variable_name} == {original_type}::{val}")
                    else:
                        s.w_no_indent(
                            f"{variable_name} == {first_version}_{original_type_pascal}_{val}")
                s.wln_no_indent(") {")
                s.inc_indent()

        case model.DefinerType.FLAG:
            s.w(f"{extra_elseif}if (")
            for i, val in enumerate(statement.values):
                if i != 0:
                    s.w_no_indent("|| ")
                if is_cpp():
                    s.w_no_indent(
                        f"({variable_name} & {original_type_pascal}_{val}) != 0")
                else:
                    s.w_no_indent(
                        f"({variable_name} & {first_version}_{original_type_pascal}_{val}) != 0")
            s.wln_no_indent(") {")
            s.inc_indent()
        case _:
            raise Exception()
