import typing

import model
from print_struct.struct_util import container_has_c_members, container_module_prefix
from util import first_version_as_module, \
    get_type_prefix, is_world, version_to_module_name, version_matches, get_export_define, container_needs_size_in_read, \
    is_cpp, library_type, world_version_is_wrath
from writer import Writer
import print_struct

INVALID_OPCODE = 0xFFFF


def side_matches(e: model.Container, side: str) -> int:
    match e.object_type:
        case model.ObjectTypeClogin(opcode=opcode) | model.ObjectTypeCmsg(opcode=opcode):
            if side == "client":
                return opcode
        case model.ObjectTypeSlogin(opcode=opcode) | model.ObjectTypeSmsg(opcode=opcode):
            if side == "server":
                return opcode
        case model.ObjectTypeMsg(opcode=opcode):
            return opcode

    return INVALID_OPCODE


def print_login_utils_side(s: Writer, h: Writer, messages: list[model.Container],
                           v: typing.Union[int | model.WorldVersion], side: str, opcodes: dict[str, int]):
    def matches(container: model.Container) -> bool:
        if not version_matches(container.tags, v) \
                or side_matches(container, side) == INVALID_OPCODE \
                or (first_version_as_module(container.tags) == "all" and version_to_module_name(v) != "all"):
            return False
        return True

    has_messages = False
    for e in messages:
        if not version_matches(e.tags, v) or side_matches(e, side) == INVALID_OPCODE:
            continue
        has_messages = True

    if not has_messages:
        return

    side_pascal = side.capitalize()

    module_name = version_to_module_name(v)
    module_name_pascal = module_name.capitalize()
    export_define = get_export_define(messages[0].tags)

    if is_cpp():
        h.open_curly(f"class {side.capitalize()}Opcode")
        h.open_curly("enum class Opcode")

        if is_world(messages[0].tags):
            h.wln("NONE = 0xFFFF,")
        else:
            h.wln("NONE = 0xFF,")
        for e in filter(matches, messages):
            opcode = opcodes[e.name.replace("_Client", "").replace("_Server", "")]

            h.wln(f"{e.name.replace('_Client', '').replace('_Server', '')} = {opcode},")

        h.closing_curly(" opcode;")

        h.newline()

        h.open_curly("union")

        first_opcode: typing.Union[None, str] = None
        first_type: typing.Union[None, str] = None
        for e in filter(matches, messages):
            ty = e.name.replace("_Client", "").replace("_Server", "")
            if first_opcode is None:
                first_opcode = ty
                first_type = e.name

            h.wln(f"{module_name}::{e.name} {ty};")

        h.closing_curly(";")  # union
        h.newline()

        h.wln_no_indent("public:")

        h.open_curly(f"{export_define} bool is_none() const noexcept")
        h.wln("return opcode == Opcode::NONE;")
        h.closing_curly()

        size_field_size = 2 if side == "server" else 4
        world = type(v) is model.WorldVersion

        extra = ", const std::function<void(unsigned char*, size_t)>& decrypt" if world else ""
        h.wln(f"{export_define} static {side.capitalize()}Opcode read(Reader& reader{extra});")
        h.newline()

        s.open_curly(
            f"{export_define} {side.capitalize()}Opcode {side.capitalize()}Opcode::read(Reader& reader{extra})")

        if type(v) is model.WorldVersion:
            if side == "server":
                opcode_type = "uint16_t"
                header_size = 4 if not world_version_is_wrath(v) else 5
            else:
                opcode_type = "uint32_t"
                header_size = 6

            s.write_block(f"""
                constexpr size_t HEADER_SIZE = {header_size};
                unsigned char header[HEADER_SIZE];
            """)

            if world_version_is_wrath(v) and side == "server":
                s.write_block(f"""
                    reader.read_bytes(header, HEADER_SIZE - 1);
                    decrypt(header, HEADER_SIZE - 1);

                    uint32_t _size;
                    uint16_t opcode;

                    if((header[0] &0x80) != 0) {{
                        reader.read_bytes(header + 4, 1);
                        decrypt(header + 4, 1);

                        _size = static_cast<uint32_t>(header[0]) << 16 | static_cast<uint32_t>(header[1]) << 8 | static_cast<uint32_t>(header[2]);
                        opcode = static_cast<uint16_t>(header[3]) | static_cast<uint16_t>(header[4]) << 8;
                    }} else {{
                        _size = static_cast<uint32_t>(header[0]) << 8 | static_cast<uint32_t>(header[1]);
                        opcode = static_cast<uint16_t>(header[2]) | static_cast<uint16_t>(header[3]) << 8;
                    }}
                """)
            else:
                s.write_block(f"""
                    reader.read_bytes(header, HEADER_SIZE);
                    decrypt(header, HEADER_SIZE);

                    const uint16_t _size = static_cast<uint16_t>(header[0]) << 8 | static_cast<uint16_t>(header[1]);
                """)

                if side == "server":
                    s.wln(
                        "const uint16_t opcode = static_cast<uint16_t>(header[2]) | static_cast<uint16_t>(header[3]) << 8;")
                else:
                    s.wln(
                        "const uint32_t opcode = static_cast<uint32_t>(header[2]) | static_cast<uint32_t>(header[3]) << 8 | static_cast<uint32_t>(header[4]) << 16 | static_cast<uint32_t>(header[5]) << 24 ;")
        else:
            opcode_type = "uint8_t"
            s.wln("const uint8_t opcode = reader.read_u8();")

        s.newline()
        s.wln(f"{side.capitalize()}Opcode op;")
        s.newline()

        for e in messages:
            if not version_matches(e.tags, v) \
                    or side_matches(e, side) == INVALID_OPCODE \
                    or (first_version_as_module(e.tags) == "all" and version_to_module_name(v) != "all"):
                continue
            name = e.name.replace('_Client', '').replace('_Server', '')
            s.open_curly(f"if (opcode == static_cast<{opcode_type}>({side.capitalize()}Opcode::Opcode::{name}))")
            module = library_type(e.tags)
            if not container_has_c_members(e):
                s.wln(f"return {side.capitalize()}Opcode(::wow_{module}_messages::{module_name}::{e.name}{{}});")
            else:
                body_size = f", _size - {size_field_size}" if container_needs_size_in_read(e) else ""

                module_prefix = container_module_prefix(e.tags, module_name)
                s.wln(
                    f"return {side.capitalize()}Opcode(::wow_{module}_messages::{module_prefix}::{e.name}_read(reader{body_size}));")

            s.closing_curly()  # if opcode == opcode

        s.newline()
        s.wln("return op;")

        s.closing_curly()  # function decl
        extra = "const std::function<void(unsigned char*, size_t)>& encrypt" if world else ""
        h.wln(f"{export_define} std::vector<unsigned char> write({extra}) const;")
        h.newline()

        s.open_curly(f"{export_define} std::vector<unsigned char> {side.capitalize()}Opcode::write({extra}) const")
        extra = "encrypt" if world else ""

        for e in messages:
            if not version_matches(e.tags, v) \
                    or side_matches(e, side) == INVALID_OPCODE \
                    or (first_version_as_module(e.tags) == "all" and version_to_module_name(v) != "all"):
                continue
            name = e.name.replace('_Client', '').replace('_Server', '')
            s.open_curly(f"if (this->opcode == {side.capitalize()}Opcode::Opcode::{name})")

            function_suffix: str = ""
            if type(e.object_type) is model.ObjectTypeMsg:
                first_char = side[0].lower()
                function_suffix = f"_{first_char}msg"
            s.wln(f"return this->{name}.write{function_suffix}({extra});;")
            s.closing_curly()  # if

        s.newline()
        s.wln("return {}; /* unreachable */")
        s.closing_curly()  # write_opcode

        h.newline()
        h.wln(f"{export_define} {side.capitalize()}Opcode() : opcode(Opcode::NONE), {first_opcode}() {{}}")
        h.newline()

        h.wln("/* 1 destructor */")
        h.wln(f"{export_define} ~{side.capitalize()}Opcode();")
        h.newline()

        s.wln("/* 1 destructor */")
        s.open_curly(f"{export_define} {side.capitalize()}Opcode::~{side.capitalize()}Opcode()")
        for e in filter(matches, messages):
            ty = e.name.replace("_Client", "").replace("_Server", "")

            s.open_curly(f"if (opcode == Opcode::{ty})")
            s.wln(f"this->{ty}.~{e.name}();")
            s.closing_curly()  # if (opcode == Opcode

        s.closing_curly()  # destructor
        s.newline()

        h.wln("/* 2 copy constructor */")
        h.wln(f"{export_define} {side.capitalize()}Opcode(const {side.capitalize()}Opcode& other);")

        s.wln("/* 2 copy constructor */")
        s.open_curly(
            f"{export_define} {side.capitalize()}Opcode::{side.capitalize()}Opcode(const {side.capitalize()}Opcode& other)")
        s.wln("this->opcode = other.opcode;")
        for e in filter(matches, messages):
            ty = e.name.replace("_Client", "").replace("_Server", "")

            s.open_curly(f"if (opcode == Opcode::{ty})")
            s.wln(f"new (&{ty}) auto(other.{ty});")
            s.closing_curly()  # if (opcode == Opcode
        s.closing_curly()
        s.newline()

        h.wln("/* 3 copy assignment */")
        h.wln(f"{export_define} {side.capitalize()}Opcode& operator=(const {side.capitalize()}Opcode& other);")

        s.wln("/* 3 copy assignment */")
        s.open_curly(
            f"{export_define} {side.capitalize()}Opcode& {side.capitalize()}Opcode::operator=(const {side.capitalize()}Opcode& other)")
        s.open_curly("if (this == &other)")
        s.wln("return *this;")
        s.closing_curly()
        s.newline()

        s.wln("this->opcode = other.opcode;")
        for e in filter(matches, messages):
            ty = e.name.replace("_Client", "").replace("_Server", "")

            s.open_curly(f"if (opcode == Opcode::{ty})")
            s.wln(f"{module_name}::{e.name} {ty}swap(other.{ty});")
            s.wln(f"std::swap(this->{ty}, {ty}swap);")
            s.closing_curly()  # if (opcode == Opcode

        s.wln("return *this;")
        s.closing_curly()
        s.newline()

        h.wln("/* 4 move constructor */")
        h.wln(f"{export_define} {side.capitalize()}Opcode({side.capitalize()}Opcode&& other) noexcept;")
        h.newline()

        s.wln("/* 4 move constructor */")
        s.open_curly(
            f"{export_define} {side.capitalize()}Opcode::{side.capitalize()}Opcode({side.capitalize()}Opcode&& other) noexcept")
        s.wln("this->opcode = other.opcode;")
        s.wln("other.opcode = Opcode::NONE;")
        for e in filter(matches, messages):
            ty = e.name.replace("_Client", "").replace("_Server", "")

            s.open_curly(f"if (opcode == Opcode::{ty})")
            s.wln(f"this->{ty} = std::move(other.{ty});")
            s.closing_curly()  # if (opcode == Opcode
        s.closing_curly()
        s.newline()

        h.wln("/* 5 move assignment */")
        h.wln(f"{export_define} {side.capitalize()}Opcode& operator=({side.capitalize()}Opcode&& other) noexcept;")
        h.newline()

        s.wln("/* 5 move assignment */")
        s.open_curly(
            f"{export_define} {side.capitalize()}Opcode& {side.capitalize()}Opcode::operator=({side.capitalize()}Opcode&& other) noexcept")
        s.wln("this->opcode = other.opcode;")
        s.wln("other.opcode = Opcode::NONE;")
        for e in filter(matches, messages):
            ty = e.name.replace("_Client", "").replace("_Server", "")

            s.open_curly(f"if (opcode == Opcode::{ty})")
            s.wln(f"{module_name}::{e.name} {ty}swap(std::move(other.{ty}));")
            s.wln(f"std::swap(this->{ty}, {ty}swap);")
            s.closing_curly()  # if (opcode == Opcode
        s.wln("return *this;")
        s.closing_curly()
        s.newline()

        for e in filter(matches, messages):
            ty = e.name.replace("_Client", "").replace("_Server", "")

            h.wln(f"{export_define} {side.capitalize()}Opcode({module_name}::{e.name}&& obj);")

            s.open_curly(
                f"{export_define} {side.capitalize()}Opcode::{side.capitalize()}Opcode({module_name}::{e.name}&& obj)")
            s.wln(f"opcode = Opcode::{ty};")
            s.wln(f"new (&this->{ty}) {module_name}::{e.name} (std::move(obj));")
            s.closing_curly()  # constructor
        h.newline()
        s.newline()

        h.wln("template<typename T>")
        h.wln("// NOLINTNEXTLINE")
        h.open_curly(f"T& get()")
        h.wln(f'static_assert(std::is_same<T, {module_name}::{first_type}>::value, "Invalid type. All possible types have been specialized below.");')
        h.closing_curly()
        h.newline()

        h.wln("template<typename T>")
        h.wln("// NOLINTNEXTLINE")
        h.open_curly(f"T* get_if()")
        h.wln(f'static_assert(std::is_same<T, {module_name}::{first_type}>::value, "Invalid type. All possible types have been specialized below.");')
        h.closing_curly()

        h.newline()
        h.wln(f"{export_define} const char* to_string() const;")
        s.open_curly(f"{export_define} const char* {side.capitalize()}Opcode::to_string() const")
        s.wln('if (opcode == Opcode::NONE) {{ return "NONE"; }}')
        for e in filter(matches, messages):
            ty = e.name.replace("_Client", "").replace("_Server", "")
            s.wln(f'if (opcode == Opcode::{ty}) {{ return "{e.name}"; }}')

        s.wln("return nullptr;")

        s.closing_curly()

        h.closing_curly(";")  # struct sideOpcode
        h.newline()

        for e in filter(matches, messages):
            ty = e.name.replace("_Client", "").replace("_Server", "")

            h.wln("template<>")
            h.wln(f"{export_define} {module_name}::{e.name}* {side.capitalize()}Opcode::get_if();")

            s.wln("template <>")
            s.open_curly(f"{export_define} {module_name}::{e.name}* {side.capitalize()}Opcode::get_if<{e.name}>()")
            s.open_curly(f"if (opcode == Opcode::{ty})")
            s.wln(f"return &{ty};")
            s.closing_curly()  # if opcode ==
            s.wln("return nullptr;")
            s.closing_curly()  # get_if()

            h.wln("template<>")
            h.wln(f"{export_define} {module_name}::{e.name}& {side.capitalize()}Opcode::get();")
            s.wln("template <>")
            s.open_curly(f"{export_define} {module_name}::{e.name}& {side.capitalize()}Opcode::get<{e.name}>()")
            s.wln(f"auto p = {side.capitalize()}Opcode::get_if<{module_name}::{e.name}>();")
            s.open_curly("if (p)")
            s.wln("return *p;")
            s.closing_curly()  # if p
            s.wln("throw bad_opcode_access{};")

            s.closing_curly()  # get()

            s.newline()
        s.newline()
        h.newline()


    else:
        h.open_curly("typedef struct")
        if type(v) is int:
            h.wln(f"WowLoginOpcode opcode;")
        else:
            h.wln(f"Wow{module_name_pascal}WorldOpcode opcode;")
        h.newline()

        h.open_curly("union")

        for e in messages:
            if not version_matches(e.tags, v) \
                    or side_matches(e, side) == INVALID_OPCODE \
                    or (first_version_as_module(e.tags) == "all" and version_to_module_name(v) != "all"):
                continue
            if not container_has_c_members(e):
                continue
            version = container_module_prefix(e.tags, module_name)
            h.wln(f"{version}_{e.name} {e.name};")

        h.closing_curly(" body;")  # union

        h.closing_curly(f" {module_name_pascal}{side_pascal}OpcodeContainer;")  # struct
        h.newline()

    write_opcode_write(s, h, messages, v, side, module_name, module_name_pascal)

    write_opcode_read(s, h, messages, v, side, module_name, module_name_pascal, side_pascal)

    if not is_cpp():
        write_opcode_free(s, h, messages, v, side, module_name, module_name_pascal, side_pascal)

        write_opcode_to_string(s, h, messages, v, side, module_name, module_name_pascal, side_pascal)


def write_opcode_to_string(s: Writer, h: Writer, messages: list[model.Container],
                           v: typing.Union[int | model.WorldVersion],
                           side: str, module_name: str,
                           module_name_pascal: str, side_pascal: str):
    if not is_cpp():
        function_declaration = f"{get_export_define(messages[0].tags)} char* {module_name}_{side}_opcode_to_str({module_name_pascal}{side_pascal}OpcodeContainer* opcodes)"
        h.wln(f"{function_declaration};")
        s.open_curly(function_declaration)
        prefix = f"{module_name[0].upper()}_" if is_world(messages[0].tags) else ""

        s.open_curly("switch (opcodes->opcode)")
        for e in messages:
            if not version_matches(e.tags, v) \
                    or side_matches(e, side) == INVALID_OPCODE \
                    or (first_version_as_module(e.tags) == "all" and version_to_module_name(v) != "all"):
                continue
            if not container_has_c_members(e):
                continue

            s.wln(f"case {prefix}{e.name.replace('_Client', '').replace('_Server', '')}: return \"{e.name}\";")

        s.wln("default:")
        s.inc_indent()
        s.wln("break;")
        s.dec_indent()

        s.closing_curly()  # switch
        s.newline()
        s.wln(f"return NULL;")

        s.closing_curly()
    s.newline()


def write_opcode_write(s: Writer, h: Writer, messages: list[model.Container], v: typing.Union[int | model.WorldVersion],
                       side: str, module_name: str, module_name_pascal: str):
    if is_cpp():
        return

    export = get_export_define(messages[0].tags)
    result_type = get_type_prefix(messages[0].tags)
    world = is_world(messages[0].tags)
    function_declaration = f"{export} {result_type}Result {module_name}_{side}_opcode_write({result_type}Writer* writer, const {module_name_pascal}{side.capitalize()}OpcodeContainer* opcodes)"
    wlm_prefix = "WWM" if world else "WLM"

    h.wln(f"{function_declaration};")
    h.newline()

    s.open_curly(function_declaration)
    s.wln("int _return_value = 1;")

    s.write_block(f"""
        if (writer->index > writer->length) {{
            return {wlm_prefix}_RESULT_INVALID_PARAMETERS;
        }}
    """)
    s.newline()

    s.open_curly("switch (opcodes->opcode)")
    prefix = f"{module_name[0].upper()}_" if world else ""

    for e in messages:
        if not version_matches(e.tags, v) \
                or side_matches(e, side) == INVALID_OPCODE \
                or (first_version_as_module(e.tags) == "all" and version_to_module_name(v) != "all"):
            continue
        if not container_has_c_members(e):
            continue

        s.wln(f"case {prefix}{e.name.replace('_Client', '').replace('_Server', '')}:")
        s.inc_indent()

        version = container_module_prefix(e.tags, module_name)
        extra_msg = "" if not type(e.object_type) is model.ObjectTypeMsg else f"_{side[0]}msg"
        s.wln(
            f"{wlm_prefix}_CHECK_RETURN_CODE({version}_{e.name}{extra_msg}_write(writer, &opcodes->body.{e.name}));")

        s.wln("break;")
        s.dec_indent()

    s.wln("default:")
    s.inc_indent()
    s.wln("break;")
    s.dec_indent()

    s.closing_curly()  # switch
    s.newline()
    s.wln(f"return {wlm_prefix}_RESULT_SUCCESS;")

    s.wln_no_indent("cleanup: return _return_value;")

    s.closing_curly()  # function decl
    s.newline()


def write_opcode_read(s: Writer, h: Writer, messages: list[model.Container], v: typing.Union[int | model.WorldVersion],
                      side: str, module_name: str, module_name_pascal: str, side_pascal: str):
    export = get_export_define(messages[0].tags)
    world = is_world(messages[0].tags)
    result_type = get_type_prefix(messages[0].tags)
    size_field_size = 2 if side == "server" else 4

    if not is_cpp():
        wlm_prefix = "WWM" if world else "WLM"

        function_declaration = f"{export} {result_type}Result {module_name}_{side}_opcode_read({result_type}Reader* reader, {module_name_pascal}{side_pascal}OpcodeContainer* opcodes)"

        h.wln(f"{function_declaration};")
        h.newline()

        s.open_curly(function_declaration)
        s.wln("int _return_value = 1;")

        if world:
            s.wln("uint16_t _size;")

        s.write_block(f"""
            if (reader->index > reader->length) {{
                return {wlm_prefix}_RESULT_INVALID_PARAMETERS;
            }}
        """)
        s.newline()

        if world:
            if side == "server":
                s.wln("READ_U16_BE(_size);")
                s.newline()

                s.wln("opcodes->opcode = 0;")
                s.wln("READ_U16(opcodes->opcode);")
            else:
                s.wln("READ_U16_BE(_size);")
                s.newline()

                s.wln("opcodes->opcode = 0;")
                s.wln("READ_U32(opcodes->opcode);")
        else:
            s.wln("opcodes->opcode = 0;")
            s.wln("READ_U8(opcodes->opcode);")

        s.newline()

        s.open_curly("switch (opcodes->opcode)")

        wlm_prefix = "WWM" if is_world(messages[0].tags) else "WLM"
        prefix = f"{module_name[0].upper()}_" if is_world(messages[0].tags) else ""

        for e in messages:
            if not version_matches(e.tags, v) \
                    or side_matches(e, side) == INVALID_OPCODE \
                    or (first_version_as_module(e.tags) == "all" and version_to_module_name(v) != "all"):
                continue
            if not container_has_c_members(e):
                continue

            s.wln(f"case {prefix}{e.name.replace('_Client', '').replace('_Server', '')}:")
            s.inc_indent()

            body_size = f", _size - {size_field_size}" if container_needs_size_in_read(e) else ""

            version = container_module_prefix(e.tags, module_name)
            s.wln(
                f"{wlm_prefix}_CHECK_RETURN_CODE({version}_{e.name}_read(reader, &opcodes->body.{e.name}{body_size}));")

            s.wln("break;")
            s.dec_indent()

        s.wln("default:")
        s.inc_indent()
        s.wln("break;")
        s.dec_indent()

        s.closing_curly()  # switch
        s.newline()

        if is_world(messages[0].tags):
            s.wln("return WWM_RESULT_SUCCESS;")
        else:
            s.wln("return WLM_RESULT_SUCCESS;")

        s.wln_no_indent("cleanup: return _return_value;")

        s.closing_curly()  # function_declaration
        s.newline()


def write_opcode_free(s: Writer, h: Writer, messages: list[model.Container], v: typing.Union[int | model.WorldVersion],
                      side: str, module_name: str,
                      module_name_pascal: str, side_pascal: str):
    export = get_export_define(messages[0].tags)
    function_declaration = f"{export} void {module_name}_{side}_opcode_free({module_name_pascal}{side_pascal}OpcodeContainer* opcodes)"
    h.wln(f"{function_declaration};")
    h.newline()

    s.open_curly(function_declaration)

    prefix = f"{module_name[0].upper()}_" if is_world(messages[0].tags) else ""
    s.open_curly("switch (opcodes->opcode)")
    for e in messages:
        if not version_matches(e.tags, v) \
                or side_matches(e, side) == INVALID_OPCODE \
                or (first_version_as_module(e.tags) == "all" and version_to_module_name(v) != "all"):
            continue

        if not print_struct.print_free.container_has_free(e, module_name):
            continue

        s.wln(f"case {prefix}{e.name.replace('_Client', '').replace('_Server', '')}:")
        s.inc_indent()

        version = container_module_prefix(e.tags, module_name)
        s.wln(f"{version}_{e.name}_free(&opcodes->body.{e.name});")

        s.wln("break;")
        s.dec_indent()

    s.wln("default:")
    s.inc_indent()
    s.wln("break;")
    s.dec_indent()

    s.closing_curly()  # switch

    s.closing_curly()  # function_declaration
    s.newline()


def print_login_utils(s: Writer, h: Writer, messages: list[model.Container], v: typing.Union[int | model.WorldVersion]):
    module_name = version_to_module_name(v)
    module_name_pascal = module_name.capitalize()

    opcodes: dict[str, int] = {}
    for e in messages:
        if not version_matches(e.tags, v):
            continue
        name = e.name.replace('_Client', '').replace('_Server', '')
        op = 0xFFFF
        match e.object_type:
            case model.ObjectTypeClogin(opcode=opcode):
                op = opcode
            case model.ObjectTypeSlogin(opcode=opcode):
                op = opcode
            case model.ObjectTypeSmsg(opcode=opcode):
                op = opcode
            case model.ObjectTypeCmsg(opcode=opcode):
                op = opcode
            case model.ObjectTypeMsg(opcode=opcode):
                op = opcode
            case _:
                continue

        opcodes[name] = op

    if not is_cpp() and type(v) is model.WorldVersion and module_name != "all":
        h.open_curly("typedef enum")
        prefix = module_name[0].upper()

        for i, (name, opcode) in enumerate(opcodes.items()):
            comma = "," if i < len(opcodes.items()) - 1 else ""
            h.wln(f"{prefix}_{name} = {opcode}{comma}")

        h.closing_curly(f" Wow{module_name_pascal}WorldOpcode;")
        h.newline()

    print_login_utils_side(s, h, messages, v, "client", opcodes)
    print_login_utils_side(s, h, messages, v, "server", opcodes)
