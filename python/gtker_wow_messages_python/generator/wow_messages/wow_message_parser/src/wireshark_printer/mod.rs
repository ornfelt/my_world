mod printer;
mod types;

use crate::file_utils::overwrite_if_not_same_contents;
use crate::parser::types::array::ArrayType;
use crate::parser::types::ty::Type;
use crate::parser::types::IntegerType;
use crate::path_utils::wireshark_directory;
use crate::rust_printer::writer::Writer;
use crate::wireshark_printer::printer::{
    print_enums, print_int_declarations, print_parser, print_register_info,
};
use crate::{file_utils, Objects};
use heck::{ToShoutySnakeCase, ToSnakeCase, ToTitleCase};
use std::fs::read_to_string;
use std::path::{Path, PathBuf};

pub(crate) fn print_wireshark(o: &Objects) {
    let world_objects = types::get_wireshark_object(o.world_wireshark_containers());
    let mut world_imports = print_int_declarations(&world_objects);
    let mut world_enums = print_enums(&world_objects);
    let mut world_register = print_register_info(&world_objects, false);
    let (mut world_parser, mut world_variables) = print_parser(
        &o.world_wireshark_messages(),
        &o.world_wireshark_messages(),
        &world_objects,
    );

    let login_objects = types::get_wireshark_object(o.login_wireshark_containers());
    let login_imports = print_int_declarations(&login_objects);
    let login_enums = print_enums(&login_objects);
    let login_register = print_register_info(&login_objects, true);

    let (login_parser, login_variables) = print_parser(
        &o.login_wireshark_messages(),
        &o.all_login_wireshark_messages(),
        &login_objects,
    );

    if let Ok(path) = std::env::var("WOWM_WIRESHARK") {
        let path = PathBuf::from(&path);

        let world_path = path.join("packet-woww.c");
        apply_to_wireshark_file(
            &world_imports,
            &world_enums,
            &world_register,
            &world_parser,
            &world_path,
            &world_variables,
        );

        let login_path = path.join("packet-wow.c");
        apply_to_wireshark_file(
            &login_imports,
            &login_enums,
            &login_register,
            &login_parser,
            &login_path,
            &login_variables,
        );
    }

    world_imports.append(login_imports);
    world_enums.append(login_enums);
    world_register.append(login_register);
    world_parser.append(login_parser);
    world_variables.append(login_variables);

    write_to_repo_files(
        &world_imports,
        &world_enums,
        &world_register,
        &world_parser,
        &world_variables,
    );
}

fn write_to_repo_files(
    imports: &Writer,
    enums: &Writer,
    register: &Writer,
    parser: &Writer,
    variables: &Writer,
) {
    overwrite_if_not_same_contents(imports.inner(), &wireshark_directory().join("imports.txt"));
    overwrite_if_not_same_contents(enums.inner(), &wireshark_directory().join("enums.txt"));
    overwrite_if_not_same_contents(
        register.inner(),
        &wireshark_directory().join("register.txt"),
    );
    overwrite_if_not_same_contents(parser.inner(), &wireshark_directory().join("parser.txt"));
    overwrite_if_not_same_contents(
        variables.inner(),
        &wireshark_directory().join("variables.txt"),
    );
}

fn apply_to_wireshark_file(
    imports: &Writer,
    enums: &Writer,
    register: &Writer,
    parser: &Writer,
    path: &Path,
    variables: &Writer,
) {
    let s = read_to_string(path).unwrap();

    const HF_START: &str = "/* AUTOGENERATED_START_HF */";
    const HF_END: &str = "/* AUTOGENERATED_END_HF */";
    let s = file_utils::insert_between(&s, HF_START, HF_END, imports.inner());

    const ENUM_START: &str = "/* AUTOGENERATED_START_ENUM */";
    const ENUM_END: &str = "/* AUTOGENERATED_END_ENUM */";
    let s = file_utils::insert_between(&s, ENUM_START, ENUM_END, enums.inner());

    const REGISTER_START: &str = "/* AUTOGENERATED_START_REGISTER */";
    const REGISTER_END: &str = "/* AUTOGENERATED_END_REGISTER */";
    let s = file_utils::insert_between(&s, REGISTER_START, REGISTER_END, register.inner());

    const VARIABLES_START: &str = "/* AUTOGENERATED_START_VARIABLES */";
    const VARIABLES_END: &str = "/* AUTOGENERATED_END_VARIABLES */";
    let s = file_utils::insert_between(&s, VARIABLES_START, VARIABLES_END, variables.inner());

    const PARSER_START: &str = "/* AUTOGENERATED_START_PARSER */";
    const PARSER_END: &str = "/* AUTOGENERATED_END_PARSER */";
    let s = file_utils::insert_between(&s, PARSER_START, PARSER_END, parser.inner());

    overwrite_if_not_same_contents(&s, path);
}

fn is_client_name(name: &str) -> bool {
    name.contains("_Client")
}

fn is_server_name(name: &str) -> bool {
    name.contains("_Server")
}

fn server_to_client_name(name: &str) -> String {
    name.replace("_Server", "_Client")
}

fn clean_opcode_name(name: &str) -> String {
    name.replace("_Server", "").replace("_Client", "")
}

fn ui_name(name: &str) -> String {
    name.replace('_', ".")
}

fn enum_name(name: &str) -> String {
    format!("e_{}", name.to_snake_case())
}

fn enumerator_name(enum_name: &str, name: &str) -> String {
    format!(
        "{}_{}",
        enum_name.to_shouty_snake_case(),
        name.to_shouty_snake_case()
    )
}

fn enum_strings(name: &str) -> String {
    format!("{}_strings", enum_name(name))
}

fn pretty_name(name: &str) -> String {
    name.to_title_case()
        .replace("Doesnt", "Doesn't")
        .replace("Didnt", "Didn't")
}

fn name_to_hf(name: &str, ty: &Type, is_login_type: bool) -> String {
    let mut name = match ty {
        Type::Flag { e, .. } | Type::Enum { e, .. } => e.name().to_snake_case(),
        Type::Struct { e } => e.name().to_string(),
        _ => name.to_string(),
    };

    let pos = name.chars().enumerate().find(|(_, a)| a.is_numeric());

    if let Some((i, _)) = pos {
        name = name[..i].to_string();
    }

    if name == "character" {
        match ty {
            Type::PackedGuid | Type::Guid => {
                name += "_guid";
            }
            Type::CString | Type::SizedCString | Type::String { .. } => {
                name += "_name";
            }
            _ => unreachable!("Types with the same name but different types in wireshark printer"),
        }
    } else if name == "unknown" {
        match ty {
            Type::Integer(_) => {
                name += "_int";
            }
            Type::FloatingPoint => name += "_float",
            Type::Array(array) => match array.ty() {
                ArrayType::Integer(_) => {
                    name += "_bytes";
                }
                _ => unreachable!(
                    "Types with the same name but different types in wireshark printer"
                ),
            },
            Type::PackedGuid | Type::Guid => {
                name += "_guid";
            }
            _ => panic!("unknown got type '{ty:#?}'"),
        }
    } else if name == "emote" {
        match ty {
            Type::Integer(_) => {
                name += "_int";
            }
            Type::Enum { .. } => name += "_enum",
            _ => panic!("Types with the same name but different types in wireshark printer"),
        }
    } else if name == "size" {
        if !is_login_type {
            name += "_struct";
        }
    } else if name.starts_with("position") && matches!(ty, Type::Integer(IntegerType::U16)) {
        name += "_int";
    } else if name == "item_slot" || name == "protocol_version" {
        if let Type::Integer(_) = ty {
            name += "_int";
        }
    }

    if is_login_type {
        format!("hf_wow_{name}")
    } else {
        format!("hf_woww_{name}")
    }
}