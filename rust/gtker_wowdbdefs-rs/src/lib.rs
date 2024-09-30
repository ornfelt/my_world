//! # `WoWDBDefs-rs`
//!
//! Crate for reading the `.dbd` format from the [`WoWDBDefs`](https://github.com/wowdev/WoWDBDefs) repository.
//!
//! # Example
//!
//! ```rust
//! # use std::path::Path;
//! # use wowdbdefs_rs::error::ParseError;
//! # use wowdbdefs_rs::{load_file, load_file_from_string};
//! # fn t(contents: &str, path: &Path) -> Result<(), Box<dyn std::error::Error>> {
//! // From &str
//! // Ensure that the .dbd name is correct
//! let file = load_file_from_string(contents, "Map.dbd")?.into_proper()?;
//!
//! // Or from a path
//! // `load_file` has two levels of error, one is io::Error the other is ParseError
//! let file = load_file(path)??.into_proper()?;
//!
//! // Then either use the parsed types with into_proper which is a more ergonomic API
//! for definition in &file.definitions {
//!     for entry in &definition.entries {
//!         println!("{}", entry.name);
//!         println!("{:#?}", entry.ty);
//!     }
//! }
//!
//! // Or the raw types which are a direct representation of the format
//! let file = load_file(path)??;
//!
//! for definition in &file.definitions {
//!     for entry in &definition.entries {
//!         println!("{}", entry.name);
//!         let column = file.columns.get(&entry.name).ok_or("unable to find column")?;
//!         println!("{:#?}", column.ty);
//!     }
//! }
//!
//! # Ok(())
//! # }
//! ```
//!
//! # Usage
//!
//! Add the following to your `Cargo.toml`:
//!
//! ```toml
//! [dependencies]
//! wowdbdefs-rs = "0.1.0"
//! ```
//!
//! # MSRV
//!
//! The MSRV for this crate is `1.58.1`.
//!
#![forbid(unsafe_code)]
#![warn(
    clippy::perf,
    clippy::correctness,
    clippy::style,
    clippy::missing_const_for_fn,
    clippy::doc_markdown,
    clippy::unseparated_literal_suffix,
    missing_docs,
    clippy::missing_errors_doc,
    clippy::missing_panics_doc
)]

use crate::error::ParseError;
use crate::parser::parse_file;
use std::fs::read_to_string;
use std::path::Path;
pub use types::*;

pub mod error;
mod parser;
mod types;
mod write_to_file;

pub use write_to_file::*;

mod writer;

/// Placeholder name used in [`load_file`] in case the filename is invalid.
pub const PLACEHOLDER_NAME: &str = "PLACEHOLDER";

/// Wrapper over [`load_file_from_string`].
///
/// If the filename of the path is not a valid Rust string the [`PLACEHOLDER_NAME`] will be used.
///
/// # Errors
///
/// The function has two error types:
///
/// * [`std::io::Error`], for errors in reading the file.
/// * [`ParseError`], for errors in parsing the `.dbd` file.
///
pub fn load_file(path: &Path) -> std::io::Result<Result<RawDbdFile, ParseError>> {
    let contents = read_to_string(path)?;

    let filename = if let Some(filename) = path.file_name() {
        filename.to_string_lossy().to_string()
    } else {
        PLACEHOLDER_NAME.to_string()
    };

    Ok(load_file_from_string(&contents, filename))
}

/// Load DBD file from string.
///
/// `name` must be the name of the file including `.dbd`.
/// For example `Map.dbd`.
///
/// # Errors
///
/// Returns a [`ParseError`] in case parsing fails.
pub fn load_file_from_string(
    contents: &str,
    name: impl Into<String>,
) -> Result<RawDbdFile, ParseError> {
    parse_file(contents, name.into())
}

#[cfg(test)]
mod tests {
    use crate::{load_file, load_file_from_string, write_to_file, RawDbdFile, Version};

    const MAP_CONTENTS: &str = include_str!(concat!(
        env!("CARGO_MANIFEST_DIR"),
        "/WoWDBDefs/definitions/Map.dbd"
    ));

    #[test]
    fn test_write() {
        let f = load_file_from_string(MAP_CONTENTS, "Map.dbd").unwrap();
        println!("{}", write_to_file(&f));
    }

    #[test]
    fn find_version() {
        let f = load_file_from_string(MAP_CONTENTS, "Map.dbd").unwrap();
        let wrath = f.specific_version(&Version::new(3, 3, 5, 12340));
        assert!(wrath.is_some());

        let tbc = f.specific_version(&Version::new(2, 4, 3, 8606));
        assert!(tbc.is_some());
    }

    #[test]
    fn line_and_column_to_string() {
        const CONTENTS: &str = "COLUMNS
int ID
string Directory // reference to World\\Map\\ [...]
locstring MapName_lang
int InstanceType // Integer 0: none, 1: party, 2: raid, 3: pvp, 4: arena, >=5: none (official from \"IsInInstance()\")
int Unk0?
int<Map::ID> ParentMapID

BUILD 0.6.0.3592
BUILD 0.5.3.3368, 0.5.3.3494
BUILD 0.5.s.3368-0.5.0.3592
$id$ID<32>
Directory
PVP<32>
IsInMap<32>
MapName_lang
";

        let contents = load_file_from_string(CONTENTS, "Contents.dbd");
        match contents {
            Ok(_) => panic!(),
            Err(e) => {
                dbg!(e.start_str_at(CONTENTS).unwrap());
            }
        }
    }

    #[test]
    fn parse_one() {
        load_file_from_string(MAP_CONTENTS, "Contents.dbd").unwrap();
    }

    fn get_all_files() -> Vec<RawDbdFile> {
        let mut v = Vec::with_capacity(1024);

        let paths = std::fs::read_dir("./WoWDBDefs/definitions/").unwrap();
        for entry in paths {
            let entry = entry.unwrap();

            if !entry.file_type().unwrap().is_file() {
                continue;
            }

            let f = load_file(&entry.path()).unwrap().unwrap();
            v.push(f);
        }

        v
    }

    #[test]
    fn assert_no_unexpected_integer_sizes() {
        let files = get_all_files();

        for file in files {
            for f in &file.definitions {
                for e in &f.entries {
                    file.find_column(e).unwrap();
                    if let Some(v) = e.integer_width {
                        assert!(v == 8 || v == 16 || v == 32 || v == 64);
                    }
                }
                f.to_definition(&file.columns).unwrap();
            }

            file.into_proper().unwrap();
        }
    }
}
