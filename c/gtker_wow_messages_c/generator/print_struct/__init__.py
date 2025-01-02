from print_struct.print_free import print_free_for_container
from print_struct.struct_util import print_if_statement_header, container_has_c_members, all_members_from_container, \
    container_module_prefix
from util import get_export_define, is_cpp, snake_case_to_pascal_case, is_world

import model
from print_struct.print_members import print_members_definitions, print_member_definition
from print_struct.print_read import print_read
from print_struct.print_size import print_size
from print_struct.print_write import print_write
from writer import Writer


def print_struct(s: Writer, h: Writer, container: model.Container, module_name: str):
    if container_has_c_members(container) or is_cpp():
        if is_cpp():
            if container.tags.comment is not None:
                h.wln(f"/* {container.tags.comment} */")

            h.open_curly(f"struct {container.name}")
            if container.optional is not None:
                h.open_curly(f"struct {snake_case_to_pascal_case(container.optional.name)}")
                for member in container.optional.members:
                    print_member_definition(h, member, module_name)
                h.closing_curly(f";")
                h.newline()

            for member in container.members:
                print_member_definition(h, member, module_name)

            if container.optional is not None:
                h.wln(
                    f"std::shared_ptr<{snake_case_to_pascal_case(container.optional.name)}> {container.optional.name};")

            if type(container.object_type) is not model.ObjectTypeStruct:
                h.newline()
                extra = "const std::function<void(unsigned char*, size_t)>& encrypt" if is_world(container.tags) else ""
                export = get_export_define(container.tags)
                if type(container.object_type) is model.ObjectTypeMsg:
                    h.wln(f"{export} std::vector<unsigned char> write_smsg({extra}) const;")
                    h.wln(f"{export} std::vector<unsigned char> write_cmsg({extra}) const;")
                else:
                    h.wln(f"{export} std::vector<unsigned char> write({extra}) const;")

            h.closing_curly(";")  # struct
        else:
            if container.optional is not None:
                h.open_curly("typedef struct")
                for member in container.optional.members:
                    print_member_definition(h, member, module_name)
                h.closing_curly(f" {module_name}_{container.name}_{container.optional.name};")
                h.newline()

            if container.tags.comment is not None:
                h.wln(f"/* {container.tags.comment} */")

            h.open_curly(f"typedef struct")
            print_members_definitions(h, container, module_name)
            if container.optional is not None:
                h.wln(f"{module_name}_{container.name}_{container.optional.name}* {container.optional.name};")
            h.closing_curly(f" {module_name}_{container.name};")

    if not is_cpp():
        print_free_for_container(s, h, container, module_name)

    print_size(s, container, module_name)

    print_read(s, container, module_name)

    match container.object_type:
        case model.ObjectTypeMsg(opcode=opcode):
            print_write(s, h, container, model.ObjectTypeCmsg("CMsg", opcode), module_name, "")
            print_write(s, h, container, model.ObjectTypeSmsg("SMsg", opcode), module_name, "")
        case _:
            print_write(s, h, container, container.object_type, module_name, "")

    h.newline()


