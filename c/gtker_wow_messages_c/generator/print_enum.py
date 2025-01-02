import sys

from print_struct.struct_util import integer_type_to_size, integer_type_to_c_str, container_module_prefix

import model
from util import pascal_case_to_snake_case, is_cpp
from model import Definer
from util import first_version_as_module
from writer import Writer


def value_to_integer(val: str, integer_type: model.IntegerType) -> str:
    value: int = int(val)
    if integer_type_to_size(integer_type) <= 4 and int(val) > 2147483647:
        value_bytes: bytes = value.to_bytes(integer_type_to_size(integer_type), byteorder=sys.byteorder, signed=False)
        val = str(int.from_bytes(value_bytes, byteorder=sys.byteorder, signed=True))
        if val == "-2147483648":
            return "-2147483647 - 1"  # Work around MSVC parsing it as unary negate operator, then as unsigned integer

    return val

def print_enum(h: Writer, enum: Definer, module_name: str):
    version_prefix = container_module_prefix(enum.tags, module_name)
    version_prefix_upper = version_prefix.upper()
    enum_name_upper = pascal_case_to_snake_case(enum.name).upper()

    if is_cpp():
        enum_type = "enum class" if enum.definer_type is model.DefinerType.ENUM else "enum"
        h.open_curly(f"{enum_type} {enum.name} : {integer_type_to_c_str(enum.integer_type)}")

        for i, enumerator in enumerate(enum.enumerators):
            flag_prefix = f"{enum_name_upper}_" if enum.definer_type is model.DefinerType.FLAG else ""
            h.wln(f"{flag_prefix}{enumerator.name.upper()} = {enumerator.value.value},")


        h.closing_curly(";") # enum class
    else:
        if integer_type_to_size(enum.integer_type) > 4:
            h.wln("/* standard C only allows enums to be ints. We need larger than that */")
            h.wln(f"typedef uint64_t {version_prefix}_{enum.name};")

            for enumerator in enum.enumerators:
                h.wln(f"#define {version_prefix_upper}_{enum_name_upper}_{enumerator.name.upper()} {enumerator.value.value}")

        else:
            h.open_curly("typedef enum")

            for i, enumerator in enumerate(enum.enumerators):
                value = value_to_integer(enumerator.value.value, enum.integer_type)
                comma = "," if i < (len(enum.enumerators) - 1) else ""
                h.wln(f"{version_prefix_upper}_{enum_name_upper}_{enumerator.name.upper()} = {value}{comma}")

            h.closing_curly(f" {version_prefix}_{enum.name};")

    h.newline()
