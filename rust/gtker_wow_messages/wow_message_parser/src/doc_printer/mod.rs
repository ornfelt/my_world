pub mod container;
pub mod definer;
mod implementation_types;
mod update_mask;

use crate::doc_printer::container::print_docs_for_container;
use crate::doc_printer::definer::{print_docs_for_enum, print_docs_for_flag};
use crate::doc_printer::update_mask::print_update_mask_docs;
use crate::file_utils::create_and_overwrite_if_not_same_contents;
use crate::parser::types::container::Container;
use crate::parser::types::definer::Definer;
use crate::parser::types::objects::Objects;
use crate::parser::types::tags::ObjectTags;
use crate::parser::types::version::AllVersions;
use crate::path_utils::{doc_summary_path, docs_directory};
use crate::rust_printer::writer::Writer;
use crate::rust_printer::DefinerType;
use crate::should_not_write_object_docs;
use hashbrown::HashMap;
use std::collections::{BTreeMap, BTreeSet};
use std::fs::read_to_string;
use std::path::PathBuf;

pub(crate) fn print_docs(o: &Objects) {
    print_update_mask_docs();

    print_docs_summary_and_objects(o);

    implementation_types::print_implementation_types(o);
}

fn common(s: &mut Writer, tags: &ObjectTags, name: &str, print_header: bool) {
    if print_header {
        s.wln(format!("# {name}"));
        s.newline();
    }

    print_versions(s, tags);

    print_metadata(s, tags);
}

fn print_metadata(s: &mut Writer, tags: &ObjectTags) {
    if let Some(comment) = tags.comment() {
        for l in comment.as_doc_lines() {
            s.wln(l);
            s.newline();
        }
    }
}

fn print_versions(s: &mut Writer, tags: &ObjectTags) {
    s.w("## ");

    match tags.all_versions() {
        AllVersions::Login(l) => {
            for (i, l) in l.iter().enumerate() {
                if i != 0 {
                    s.w(", ");
                }
                s.w(format!("Protocol Version {l}"));
            }
        }
        AllVersions::World(l) => {
            for (i, l) in l.iter().enumerate() {
                if i != 0 {
                    s.w(", ");
                }
                s.w(format!("Client Version {l}"));
            }
        }
    }

    s.newline();
    s.newline();
}

const AUTOGENERATED_LINE_COMMENT: &str = "[comment]: # (AUTOGENERATED_FROM_HERE_UNTIL_END_OF_FILE)";
const LOGIN_DEFINER_HEADER: &str = "# Login Definers";
const LOGIN_CONTAINER_HEADER: &str = "# Login Containers";
const WORLD_DEFINER_HEADER: &str = "# World Definers";
const WORLD_CONTAINER_HEADER: &str = "# World Containers";

pub(crate) fn print_docs_summary_and_objects(o: &Objects) {
    let s = read_to_string(doc_summary_path()).unwrap();
    let (s, _) = s.split_once(AUTOGENERATED_LINE_COMMENT).unwrap();
    let mut s = Writer::start_with(s.to_string());

    s.wln(AUTOGENERATED_LINE_COMMENT);
    s.newline();

    let mut files = HashMap::new();

    print_definers(
        &mut s,
        o.all_definers()
            .filter(|a| !should_not_write_object_docs(a.tags())),
        &mut files,
    );
    print_containers(
        &mut s,
        o.all_containers()
            .filter(|a| !should_not_write_object_docs(a.tags())),
        &mut files,
        o,
    );

    create_and_overwrite_if_not_same_contents(s.inner(), &doc_summary_path());

    for (path, s) in &files {
        create_and_overwrite_if_not_same_contents(s, path);
    }
}

fn print_definers<'a>(
    s: &mut Writer,
    definers: impl Iterator<Item = &'a Definer>,
    files: &mut HashMap<PathBuf, String>,
) {
    let mut login_definers = BTreeSet::new();
    let mut world_definers = BTreeSet::new();

    for definer in definers {
        let path = format!(
            "{lower_name}.md",
            lower_name = definer.name().to_lowercase()
        );

        let full_path = docs_directory().join(&path);
        let print_header = files.get(&full_path).is_none();
        let definer_inner = match definer.definer_ty() {
            DefinerType::Enum => print_docs_for_enum(definer, print_header).into_inner(),
            DefinerType::Flag => print_docs_for_flag(definer, print_header).into_inner(),
        };

        create_or_append_hashmap(&definer_inner, full_path, files);

        let is_used_in_object = !definer.objects_used_in().is_empty();
        if is_used_in_object {
            continue;
        }

        let bullet_point = format!(
            "- [{name}](docs/{path})",
            name = definer.name(),
            path = path,
        );
        if definer.tags().has_login_version() {
            login_definers.insert(bullet_point);
        } else {
            world_definers.insert(bullet_point);
        }
    }

    if !login_definers.is_empty() {
        s.wln(LOGIN_DEFINER_HEADER);
        for i in login_definers {
            s.wln(&i);
        }
        s.newline();
    }

    if !world_definers.is_empty() {
        s.wln(WORLD_DEFINER_HEADER);
        for i in world_definers {
            s.wln(&i);
        }
        s.newline();
    }
}

fn print_containers<'a>(
    s: &mut Writer,
    containers: impl Iterator<Item = &'a Container>,
    files: &mut HashMap<PathBuf, String>,
    o: &Objects,
) {
    let mut login_containers = BTreeMap::new();
    let mut world_containers = BTreeMap::new();

    for container in containers {
        let path = format!(
            "{lower_name}.md",
            lower_name = container.name().to_lowercase()
        );

        let full_path = docs_directory().join(&path);
        let print_header = files.get(&full_path).is_none();
        let container_inner = print_docs_for_container(container, o, print_header).into_inner();
        create_or_append_hashmap(&container_inner, full_path, files);

        let bullet_point = format!(
            "- [{name}](docs/{path})",
            name = container.name(),
            path = path,
        );

        let mut definers: Vec<String> = container
            .all_definers()
            .iter()
            .map(|a| {
                let name = a.name();
                let path = format!("{}.md", name.to_lowercase());

                format!("    - [{name}](docs/{path})")
            })
            .collect();
        definers.sort();
        definers.dedup();

        if container.tags().has_login_version() {
            login_containers.insert(bullet_point, definers);
        } else {
            world_containers.insert(bullet_point, definers);
        }
    }

    if !login_containers.is_empty() {
        s.wln(LOGIN_CONTAINER_HEADER);
        for (i, definers) in login_containers {
            s.wln(&i);

            for definer in definers {
                s.wln(definer);
            }
        }
        s.newline();
    }

    if !world_containers.is_empty() {
        s.wln(WORLD_CONTAINER_HEADER);
        for (i, definers) in world_containers {
            s.wln(&i);

            for definer in definers {
                s.wln(definer);
            }
        }
        s.newline();
    }
}

fn create_or_append_hashmap(s: &str, path: PathBuf, files: &mut HashMap<PathBuf, String>) {
    if let Some(c) = files.get_mut(&path) {
        c.push_str(s);
    } else {
        files.insert(path, s.to_string());
    }
}