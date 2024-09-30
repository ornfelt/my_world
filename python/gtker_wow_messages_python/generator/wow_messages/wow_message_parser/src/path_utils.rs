use crate::file_utils;
use crate::parser::types::version::LoginVersion;
use crate::parser::types::version::MajorWorldVersion;
use std::path::{Path, PathBuf};

pub(crate) fn workspace_directory() -> PathBuf {
    let mut p = PathBuf::from(env!("CARGO_MANIFEST_DIR"));
    assert!(p.pop());
    p
}

pub(crate) fn parser_test_directory() -> PathBuf {
    workspace_directory()
        .join("wow_message_parser")
        .join("tests")
}

pub(crate) fn doc_impl_world_path() -> PathBuf {
    workspace_directory()
        .join("wowm_language")
        .join("src")
        .join("ir")
        .join("implementing_world.md")
}

pub(crate) fn doc_impl_login_path() -> PathBuf {
    workspace_directory()
        .join("wowm_language")
        .join("src")
        .join("ir")
        .join("implementing_login.md")
}

pub(crate) fn doc_lang_spec_path() -> PathBuf {
    workspace_directory()
        .join("wowm_language")
        .join("src")
        .join("spec")
        .join("lang-spec.md")
}

pub(crate) fn doc_summary_path() -> PathBuf {
    workspace_directory()
        .join("wowm_language")
        .join("src")
        .join("SUMMARY.md")
}

pub(crate) fn docs_directory() -> PathBuf {
    workspace_directory()
        .join("wowm_language")
        .join("src")
        .join("docs")
}

pub(crate) fn update_mask_doc_file() -> PathBuf {
    workspace_directory()
        .join("wowm_language")
        .join("src")
        .join("types")
        .join("update-mask.md")
}

pub(crate) fn intermediate_representation() -> PathBuf {
    workspace_directory().join("intermediate_representation.json")
}

pub(crate) fn opcodes_file(version: MajorWorldVersion) -> PathBuf {
    workspace_directory()
        .join("wow_message_parser")
        .join("src")
        .join("parser")
        .join("stats")
        .join(format!("{}_messages.rs", version.module_name()))
}

pub(crate) fn expected_file(version: MajorWorldVersion) -> PathBuf {
    workspace_directory()
        .join("wow_world_messages")
        .join("src")
        .join("helper")
        .join(version.module_name())
        .join("expected.rs")
}

pub(crate) fn utils_shared_file() -> PathBuf {
    workspace_directory()
        .join("wow_world_messages")
        .join("src")
        .join("util")
        .join("functions")
        .join("shared.rs")
}

pub(crate) fn wireshark_directory() -> PathBuf {
    parser_test_directory().join("wireshark")
}

pub(crate) fn wowm_directory(directory: &str) -> PathBuf {
    workspace_directory()
        .join("wow_message_parser")
        .join("wowm")
        .join(directory)
}

pub(crate) fn login_directory() -> PathBuf {
    workspace_directory()
        .join("wow_login_messages")
        .join("src")
        .join("logon")
}

pub(crate) fn world_directory() -> PathBuf {
    workspace_directory()
        .join("wow_world_messages")
        .join("src")
        .join("world")
}

pub(crate) fn base_directory() -> PathBuf {
    workspace_directory()
        .join("wow_world_base")
        .join("src")
        .join("inner")
}

pub(crate) fn opcode_to_name_location(version: MajorWorldVersion) -> PathBuf {
    workspace_directory()
        .join("wow_world_messages")
        .join("src")
        .join("helper")
        .join(version.module_name())
        .join("opcode_to_name.rs")
}

pub(crate) fn update_mask_location(version: MajorWorldVersion) -> PathBuf {
    workspace_directory()
        .join("wow_world_messages")
        .join("src")
        .join("helper")
        .join(version.module_name())
        .join("update_mask")
        .join("impls.rs")
}

pub(crate) fn update_mask_index_location(version: MajorWorldVersion) -> PathBuf {
    workspace_directory()
        .join("wow_world_messages")
        .join("src")
        .join("helper")
        .join(version.module_name())
        .join("update_mask")
        .join("indices.rs")
}

pub(crate) fn path_to_fileinfo(path: &Path) -> String {
    let ws = workspace_directory().canonicalize().unwrap();
    let path = path.canonicalize().unwrap();

    path.strip_prefix(ws)
        .unwrap()
        .to_str()
        .unwrap()
        .replace('\\', "/")
}

pub(crate) fn get_world_version_file_path(version: &MajorWorldVersion) -> PathBuf {
    world_directory().join(file_utils::major_version_to_string(version))
}

pub(crate) fn get_login_version_file_path(version: &LoginVersion) -> PathBuf {
    login_directory().join(version.as_module_case())
}

pub(crate) fn get_base_filepath(object_name: &str, version: &MajorWorldVersion) -> PathBuf {
    base_directory()
        .join(file_utils::major_version_to_string(version))
        .join(format!("{}.rs", file_utils::get_module_name(object_name)))
}

pub(crate) fn get_base_shared_filepath(shared_module_name: &str) -> PathBuf {
    base_directory()
        .join("shared")
        .join(format!("{shared_module_name}.rs",))
}

pub(crate) fn get_world_shared_filepath(shared_module_name: &str) -> PathBuf {
    world_directory()
        .join("shared")
        .join(format!("{shared_module_name}.rs",))
}

pub(crate) fn get_world_filepath(object_name: &str, version: &MajorWorldVersion) -> PathBuf {
    get_world_version_file_path(version)
        .join(format!("{}.rs", file_utils::get_module_name(object_name)))
}

pub(crate) fn get_login_filepath(object_name: &str, version: &LoginVersion) -> PathBuf {
    get_login_version_file_path(version)
        .join(format!("{}.rs", file_utils::get_module_name(object_name)))
}
