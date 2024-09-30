use crate::error::ConversionError;
use std::cmp::Ordering;
use std::collections::{BTreeSet, HashMap};
use std::fmt::{Debug, Display, Formatter};

/// An entire `.dbd` file with all subtypes.
///
/// Use [`RawDbdFile::into_proper`] to get a more ergonomic API.
///
/// use [`RawDbdFile::specific_version`] to find the definition that is valid for that specific version.
#[derive(Debug, Clone, Eq, PartialEq)]
pub struct RawDbdFile {
    /// Name of the `dbd` file, including `.dbd`.
    ///
    /// Can not be assumed to always be correct since [`load_file_from_string`](crate::load_file_from_string) can provide an invalid name.
    pub name: String,
    /// Column definitions found under `COLUMNS`.
    pub columns: HashMap<String, RawColumn>,
    /// Individual definitions, including versioning and layouts.
    pub definitions: Vec<RawDefinition>,
}

fn compare_versions(
    version: &Version,
    version_ranges: &[VersionRange],
    versions: &BTreeSet<Version>,
) -> bool {
    for b in version_ranges {
        if b.within_range(version) {
            return true;
        }
    }

    for b in versions {
        if b == version {
            return true;
        }
    }

    false
}

impl RawDbdFile {
    /// Finds the definition for a specific version, if it exists.
    pub fn specific_version(&self, version: &Version) -> Option<&RawDefinition> {
        self.definitions
            .iter()
            .find(|a| compare_versions(version, &a.version_ranges, &a.versions))
    }

    /// Converts the raw file into a more ergonomic Rust API.
    ///
    /// Can fail if the `dbd` file does invalid things.
    ///
    /// # Errors
    ///
    /// Errors if the file does not uphold the invariants described in [`ConversionError`].
    pub fn into_proper(self) -> Result<DbdFile, ConversionError> {
        let mut definitions = Vec::with_capacity(self.definitions.len());

        for def in self.definitions {
            definitions.push(def.to_definition(&self.columns)?)
        }

        Ok(DbdFile {
            name: self.name,
            definitions,
        })
    }

    /// Find the corresponding column for an entry.
    pub fn find_column(&self, entry: &RawEntry) -> Option<&RawColumn> {
        self.columns.get(&entry.name)
    }

    pub(crate) fn empty(name: String) -> Self {
        Self {
            name,
            columns: HashMap::new(),
            definitions: vec![],
        }
    }
    pub(crate) fn add_column(&mut self, column: RawColumn) {
        self.columns.insert(column.name.clone(), column);
    }

    pub(crate) fn add_database(&mut self, definition: RawDefinition) {
        self.definitions.push(definition);
    }
}

/// Partial representation of the type.
///
/// This is parsed from the `COLUMNS` so it can not contain information about integer sizes or array status.
/// Use [`Type`] from [`DbdFile`] instead.
#[derive(Debug, Clone, Copy, Ord, PartialOrd, Eq, PartialEq, Hash)]
pub enum RawType {
    /// Integer.
    ///
    /// Can be both signed and unsigned.
    Int,
    /// Floating point value.
    Float,
    /// Localized String.
    ///
    /// Depends on the exact version but is an array of indices into the string block.
    LocString,
    /// Index into the string block.
    String,
}

impl Display for RawType {
    fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        f.write_str(match self {
            RawType::Int => "int",
            RawType::Float => "float",
            RawType::LocString => "locstring",
            RawType::String => "string",
        })
    }
}

/// Foreign key.
///
/// Is not guaranteed to point to a valid table or column since the DBC files themselves do not guarantee this.
#[derive(Debug, Clone, Ord, PartialOrd, Eq, PartialEq, Hash)]
pub struct ForeignKey {
    /// Name of the table this foreign key belongs to.
    pub database: String,
    /// Name of the column in the table.
    pub column: String,
}

impl Display for ForeignKey {
    fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        write!(f, "<{}::{}>", self.database, self.column)
    }
}

impl ForeignKey {
    /// Constructor for foreign key.
    pub const fn new(database: String, column: String) -> Self {
        Self { database, column }
    }
}

/// Column definition found under `COLUMNS`.
///
/// This can not know specifics like integer sizes and array status.
/// Use [`Entry`] from [`DbdFile`] instead.
#[derive(Debug, Clone, Ord, PartialOrd, Eq, PartialEq, Hash)]
pub struct RawColumn {
    /// Name of the column.
    pub name: String,
    /// Partial type of the column.
    pub ty: RawType,
    /// Foreign key status of the column.
    pub foreign_key: Option<ForeignKey>,
    /// If the column has been verified to be valid.
    pub verified: bool,
    /// Comment placed after the column definition with `//`.
    pub comment: Option<String>,
}

impl RawColumn {
    /// Constructor for the column.
    pub const fn new(
        name: String,
        ty: RawType,
        foreign_key: Option<ForeignKey>,
        verified: bool,
        comment: Option<String>,
    ) -> Self {
        Self {
            name,
            ty,
            foreign_key,
            verified,
            comment,
        }
    }
}

/// `WoW` client version representation.
///
/// Will sort correctly with respect to expansions and patches.
#[derive(Debug, Copy, Clone, Hash, Default, PartialEq, Eq)]
pub struct Version {
    /// Expansion version. 0 for alpha/beta, 1 for vanilla, 2 for TBC, etc.
    pub major: u8,
    /// Minor version.
    pub minor: u8,
    /// Patch version.
    pub patch: u8,
    /// Build version. 5875 for 1.12.1 and 12340 for 3.3.5 for example.
    pub build: u16,
}

impl Ord for Version {
    fn cmp(&self, other: &Self) -> Ordering {
        match self.major.cmp(&other.major) {
            Ordering::Less => Ordering::Less,
            Ordering::Greater => Ordering::Greater,
            Ordering::Equal => match self.minor.cmp(&other.minor) {
                Ordering::Less => Ordering::Less,
                Ordering::Greater => Ordering::Greater,
                Ordering::Equal => match self.patch.cmp(&other.patch) {
                    Ordering::Less => Ordering::Less,
                    Ordering::Greater => Ordering::Greater,
                    Ordering::Equal => self.build.cmp(&other.build),
                },
            },
        }
    }
}

impl PartialOrd for Version {
    fn partial_cmp(&self, other: &Self) -> Option<Ordering> {
        Some(self.cmp(other))
    }
}

impl Display for Version {
    fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        f.write_fmt(format_args!(
            "{}.{}.{}.{}",
            self.major, self.minor, self.patch, self.build
        ))
    }
}

impl Version {
    /// Constructor for version.
    pub const fn new(major: u8, minor: u8, patch: u8, build: u16) -> Self {
        Self {
            major,
            minor,
            patch,
            build,
        }
    }
}

/// Representation of version range.
#[derive(Debug, Copy, Clone, Eq, PartialEq, Hash, Default, Ord, PartialOrd)]
pub struct VersionRange {
    /// First valid version.
    pub from: Version,
    /// Last valid version.
    pub to: Version,
}

impl VersionRange {
    /// Constructor for version range.
    pub const fn new(from: Version, to: Version) -> Self {
        Self { from, to }
    }

    /// Returns true if the [`Version`] is within the range.
    pub fn within_range(&self, version: &Version) -> bool {
        self.from <= *version && self.to >= *version
    }
}

/// Representation of the layout.
#[derive(Debug, Clone, Ord, PartialOrd, Eq, PartialEq, Hash, Default)]
pub struct Layout {
    /// Integer version of the layout.
    pub inner: u32,
}

impl Display for Layout {
    fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        std::fmt::Display::fmt(&self.inner, f)
    }
}

impl Layout {
    /// Constructor for the layout.
    pub const fn new(value: u32) -> Self {
        Self { inner: value }
    }
}

/// Entry for specific column in a [`RawDefinition`].
///
/// Does not have information contained under `COLUMNS`.
/// Use [`Entry`] for that instead.
#[derive(Debug, Clone, Ord, PartialOrd, Eq, PartialEq, Hash)]
pub struct RawEntry {
    /// Name of the column.
    pub name: String,
    /// Comment placed after the column definition with `//`.
    pub comment: Option<String>,
    /// Integer width of the type.
    pub integer_width: Option<u8>,
    /// If the type is an array, this is the size.
    pub array_size: Option<usize>,
    /// Signedness of integer types.
    pub unsigned: bool,
    /// True if the column is a primary key in the current table.
    pub primary_key: bool,
    /// Data is stored inline with the row and not inside [DB2](https://wowdev.wiki/DB2#Structure_8) files.
    pub inline: bool,
    /// True if the column is a foreign key.
    pub relation: bool,
}

impl RawEntry {
    /// Constructor for raw entry.
    #[allow(clippy::too_many_arguments)]
    pub const fn new(
        name: String,
        comment: Option<String>,
        integer_width: Option<u8>,
        array_size: Option<usize>,
        unsigned: bool,
        primary_key: bool,
        inline: bool,
        relation: bool,
    ) -> Self {
        Self {
            name,
            comment,
            integer_width,
            unsigned,
            array_size,
            primary_key,
            inline,
            relation,
        }
    }

    pub(crate) const fn has_any_tag(&self) -> bool {
        self.primary_key || !self.inline || self.relation
    }
}

/// Definition for specific set of versions.
///
/// Use [`Definition`] for a more cohesive API that removes some tedium.
#[derive(Debug, Clone, Ord, PartialOrd, Eq, PartialEq, Hash, Default)]
pub struct RawDefinition {
    /// Specific valid versions.
    pub versions: BTreeSet<Version>,
    /// Valid version ranges.
    pub version_ranges: Vec<VersionRange>,
    /// Valid layouts.
    pub layouts: BTreeSet<Layout>,
    /// Entries in the definition.
    pub entries: Vec<RawEntry>,
}

impl RawDefinition {
    /// Constructor for definition.
    pub fn new(
        versions: BTreeSet<Version>,
        version_ranges: Vec<VersionRange>,
        layouts: BTreeSet<Layout>,
        entries: Vec<RawEntry>,
    ) -> Self {
        Self {
            versions,
            version_ranges,
            entries,
            layouts,
        }
    }

    /// Convert to a [`Definition`].
    ///
    /// This is far more ergonomic API for using the definitions.
    ///
    /// # Errors
    ///
    /// Errors if the file does not uphold the invariants described in [`ConversionError`].
    pub fn to_definition(
        &self,
        columns: &HashMap<String, RawColumn>,
    ) -> Result<Definition, ConversionError> {
        let mut entries = Vec::with_capacity(self.entries.len());

        for entry in &self.entries {
            let column = if let Some(c) = columns.get(&entry.name) {
                c
            } else {
                return Err(ConversionError::ColumnNotFound(entry.name.clone()));
            };

            let mut ty = match column.ty {
                RawType::Int => match entry.integer_width {
                    None => return Err(ConversionError::NoIntegerWidth),
                    Some(v) => match entry.unsigned {
                        true => match v {
                            8 => Type::UInt8,
                            16 => Type::UInt16,
                            32 => Type::UInt32,
                            64 => Type::UInt64,
                            v => return Err(ConversionError::InvalidIntegerWidth(v.into())),
                        },
                        false => match v {
                            8 => Type::Int8,
                            16 => Type::Int16,
                            32 => Type::Int32,
                            64 => Type::Int64,
                            v => return Err(ConversionError::InvalidIntegerWidth(v.into())),
                        },
                    },
                },
                RawType::Float => Type::Float,
                RawType::LocString => Type::LocString,
                RawType::String => Type::String,
            };

            if let Some(foreign_key) = &column.foreign_key {
                match ty {
                    Type::Array { .. }
                    | Type::Int8
                    | Type::Int16
                    | Type::Int32
                    | Type::Int64
                    | Type::UInt8
                    | Type::UInt16
                    | Type::UInt32
                    | Type::UInt64 => {
                        ty = Type::ForeignKey {
                            ty: Box::new(ty),
                            key: foreign_key.clone(),
                        };
                    }
                    Type::Float => return Err(ConversionError::FloatAsForeignKey),
                    Type::LocString => return Err(ConversionError::LocStringAsForeignKey),
                    Type::String => return Err(ConversionError::StringAsForeignKey),
                    Type::ForeignKey { .. } => {
                        unreachable!("ty has not been set to foreign key yet")
                    }
                }
            }

            if let Some(width) = entry.array_size {
                ty = Type::Array {
                    ty: Box::new(ty.clone()),
                    width,
                };
            }

            entries.push(Entry {
                name: entry.name.clone(),
                ty,
                comment: entry.comment.clone(),
                column_comment: column.comment.clone(),
                verified: column.verified,
                primary_key: entry.primary_key,
                inline: entry.inline,
                relation: entry.relation,
            });
        }

        Ok(Definition {
            versions: self.versions.clone(),
            version_ranges: self.version_ranges.clone(),
            layouts: self.layouts.clone(),
            entries,
        })
    }
}

/// Parsed and validated definition.
///
/// Created from [`RawDefinition::to_definition`] and [`RawDbdFile::into_proper`].
#[derive(Debug, Clone, Ord, PartialOrd, Eq, PartialEq, Hash, Default)]
pub struct Definition {
    /// Specific valid versions.
    pub versions: BTreeSet<Version>,
    /// Valid version ranges.
    pub version_ranges: Vec<VersionRange>,
    /// Specific valid layouts.
    pub layouts: BTreeSet<Layout>,
    /// Entries in the definition.
    pub entries: Vec<Entry>,
}

/// Specific entry or column in a DBC.
#[derive(Debug, Clone, Ord, PartialOrd, Eq, PartialEq, Hash)]
pub struct Entry {
    /// Name of the column.
    pub name: String,

    /// Type of the column.
    pub ty: Type,

    /// Comment on the entry itself.
    ///
    /// This applies to this specific entry and these versions.
    pub comment: Option<String>,
    /// Comment on the column definition under `COLUMNS`.
    ///
    /// This applies to the column for all versions.
    pub column_comment: Option<String>,

    /// Column content is verified.
    pub verified: bool,
    /// Column is a primary key in the table.
    pub primary_key: bool,
    /// Data is stored inline with the row and not inside [DB2](https://wowdev.wiki/DB2#Structure_8) files.
    pub inline: bool,
    /// True if the column is a foreign key.
    pub relation: bool,
}

/// Type of the column.
#[derive(Debug, Clone, Ord, PartialOrd, Eq, PartialEq, Hash)]
pub enum Type {
    /// `i8`
    Int8,
    /// `i16`
    Int16,
    /// `i32`
    Int32,
    /// `i64`
    Int64,

    /// `u8`
    UInt8,
    /// `u16`
    UInt16,
    /// `u32`
    UInt32,
    /// `u64`
    UInt64,

    /// `f32`
    Float,
    /// Localized string indices.
    ///
    /// Specific layout depends on the version, but it's an array of indices into the string block.
    LocString,
    /// Index into string block.
    String,

    /// Foreign key with a specific type.
    ForeignKey {
        /// Type representation of the foreign key.
        ty: Box<Type>,
        /// Foreign key information.
        key: ForeignKey,
    },

    /// Array of types.
    Array {
        /// Type inside the array.
        ty: Box<Type>,
        /// Size of the array.
        width: usize,
    },
}

/// Parsed and validated file.
///
/// Created from [`RawDbdFile::into_proper`].
#[derive(Debug, Clone, Eq, PartialEq, Ord, PartialOrd, Hash)]
pub struct DbdFile {
    /// Name of the `dbd` file, including `.dbd`.
    ///
    /// Can not be assumed to always be correct since [`load_file_from_string`](crate::load_file_from_string) can provide an invalid name.
    pub name: String,
    /// Parsed and validated definitions.
    pub definitions: Vec<Definition>,
}

impl DbdFile {
    /// Finds the definition for a specific version, if it exists.
    pub fn specific_version(&self, version: &Version) -> Option<&Definition> {
        self.definitions
            .iter()
            .find(|a| compare_versions(version, &a.version_ranges, &a.versions))
    }
}
