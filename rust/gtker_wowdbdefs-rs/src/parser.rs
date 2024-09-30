use crate::error::DbdErrorReason;
use crate::{
    ForeignKey, Layout, ParseError, RawColumn, RawDbdFile, RawDefinition, RawEntry, RawType,
    Version, VersionRange,
};
use std::collections::BTreeSet;

enum Mode {
    Column,
    Build,
    Entry,
}

pub(crate) fn parse_file(contents: &str, name: String) -> Result<RawDbdFile, ParseError> {
    let mut file = RawDbdFile::empty(name);
    let mut mode = Mode::Column;

    let mut layouts = BTreeSet::new();
    let mut versions = BTreeSet::new();
    let mut version_ranges = Vec::new();
    let mut entries = Vec::new();

    for (line_count, line) in contents.lines().enumerate() {
        if line.trim().is_empty() {
            continue;
        }

        let should_continue = parse_builds(
            &mut file,
            &mut mode,
            &mut layouts,
            &mut versions,
            &mut version_ranges,
            &mut entries,
            line,
            line_count,
        )?;

        if should_continue {
            continue;
        }

        match mode {
            Mode::Column => {
                let (ty_name, split) = match line.split_once(' ') {
                    None => {
                        return Err(ParseError::new(
                            0,
                            line_count,
                            DbdErrorReason::NoSpaceInColumn,
                        ));
                    }
                    Some(i) => i,
                };
                let (name, comment) = split_comment(split);

                let (ty, foreign_key) = if let Some(i) = ty_name.find('<') {
                    if let Some(j) = ty_name[i..].find('>') {
                        let ty = ty_to_type(&ty_name[..i], line_count, i)?;

                        let foreign_key = &ty_name[i + 1..j + i];
                        let foreign_key =
                            if let Some((database, column)) = foreign_key.split_once("::") {
                                ForeignKey::new(database.to_string(), column.to_string())
                            } else {
                                return Err(ParseError::new(
                                    i,
                                    line_count,
                                    DbdErrorReason::NoDoubleColonInForeignKey,
                                ));
                            };

                        (ty, Some(foreign_key))
                    } else {
                        return Err(ParseError::new(
                            i,
                            line_count,
                            DbdErrorReason::NoClosingForeignKeyAngleBracket,
                        ));
                    }
                } else {
                    (ty_to_type(ty_name, line_count, ty_name.len() + 1)?, None)
                };

                let (name, verified) = if name.ends_with('?') {
                    (name.replace('?', ""), false)
                } else {
                    (name.to_string(), true)
                };

                let column = RawColumn::new(name, ty, foreign_key, verified, comment);
                file.add_column(column);
            }
            Mode::Entry | Mode::Build => {
                parse_entry(&mut mode, &mut entries, line, line_count)?;
            }
        }
    }

    Ok(file)
}

fn parse_entry(
    mode: &mut Mode,
    entries: &mut Vec<RawEntry>,
    line: &str,
    line_count: usize,
) -> Result<(), ParseError> {
    *mode = Mode::Entry;

    let (name, comment) = split_comment(line);

    let (primary_key, inline, relation, name) = if name.starts_with('$') {
        const OFFSET: usize = 1;

        let j = match name[OFFSET..].find('$') {
            None => {
                return Err(ParseError::new(
                    OFFSET,
                    line_count,
                    DbdErrorReason::NoClosingAnnotationDollarSign,
                ));
            }
            Some(j) => j,
        };
        let annotations = name[OFFSET..j + OFFSET].split(',');
        let mut primary_key = false;
        let mut inline = true;
        let mut relation = false;

        for s in annotations {
            match s {
                "id" => {
                    primary_key = true;
                }
                "noninline" => {
                    inline = false;
                }
                "relation" => {
                    relation = true;
                }
                _ => {}
            }
        }
        let name = &name[j + OFFSET + 1..];

        (primary_key, inline, relation, name)
    } else {
        (false, true, false, name)
    };

    let (integer_width, array_size, unsigned, name) = if let Some(i) = name.find('<') {
        let identifier = &name[..i];
        let j = match name[i..].find('>') {
            None => {
                return Err(ParseError::new(
                    i,
                    line_count,
                    DbdErrorReason::NoClosingIntegerSizeAngleBracket,
                ));
            }
            Some(j) => j,
        };

        let integer_width = &name[i + 1..i + j];
        let (unsigned, integer_width) = if let Some(integer_width) = integer_width.strip_prefix('u')
        {
            if let Ok(integer_width) = integer_width.parse::<u8>() {
                (true, integer_width)
            } else {
                return Err(ParseError::new(
                    i,
                    line_count,
                    DbdErrorReason::InvalidIntegerSizeNumber(integer_width.to_string()),
                ));
            }
        } else if let Ok(integer_width) = integer_width.parse::<u8>() {
            (false, integer_width)
        } else {
            return Err(ParseError::new(
                i,
                line_count,
                DbdErrorReason::InvalidIntegerSizeNumber(integer_width.to_string()),
            ));
        };
        let name = &name[i + j..];

        let array_size = if let Some(i) = name.find('[') {
            let j = match name.find(']') {
                None => {
                    return Err(ParseError::new(
                        i,
                        line_count,
                        DbdErrorReason::NoClosingArraySizeSquareBracket,
                    ));
                }
                Some(i) => i,
            };

            match name[i + 1..j].parse::<usize>() {
                Ok(array_size) => Some(array_size),
                Err(_) => {
                    return Err(ParseError::new(
                        i,
                        line_count,
                        DbdErrorReason::InvalidArraySizeNumber(name[i + 1..j].to_string()),
                    ));
                }
            }
        } else {
            None
        };

        (Some(integer_width), array_size, unsigned, identifier)
    } else {
        let (array_size, name) = if let Some(i) = name.find('[') {
            let j = match name.find(']') {
                None => {
                    return Err(ParseError::new(
                        i,
                        line_count,
                        DbdErrorReason::NoClosingArraySizeSquareBracket,
                    ));
                }
                Some(i) => i,
            };

            let array_size = match name[i + 1..j].parse::<usize>() {
                Ok(array_size) => array_size,
                Err(_) => {
                    return Err(ParseError::new(
                        i,
                        line_count,
                        DbdErrorReason::InvalidArraySizeNumber(name[i + 1..j].to_string()),
                    ));
                }
            };

            (Some(array_size), &name[..i])
        } else {
            (None, name)
        };

        (None, array_size, false, name)
    };

    entries.push(RawEntry::new(
        name.to_string(),
        comment,
        integer_width,
        array_size,
        unsigned,
        primary_key,
        inline,
        relation,
    ));

    Ok(())
}

fn normalize_versions(versions: &mut BTreeSet<Version>, version_ranges: &[VersionRange]) {
    for v in version_ranges {
        versions.remove(&v.from);
        versions.remove(&v.to);
    }

    let vs = versions.clone();
    for v in vs {
        for range in version_ranges {
            if range.within_range(&v) {
                versions.remove(&v);
            }
        }
    }
}

#[test]
fn version_normalization() {
    let mut versions = BTreeSet::new();
    versions.insert(Version::new(3, 1, 6, 0));

    let mut version_ranges = vec![VersionRange::new(
        Version::new(3, 0, 0, 0),
        Version::new(3, 2, 5, 0),
    )];

    normalize_versions(&mut versions, &mut version_ranges);

    assert!(versions.is_empty());
}

#[allow(clippy::too_many_arguments)]
fn parse_builds(
    file: &mut RawDbdFile,
    mode: &mut Mode,
    layouts: &mut BTreeSet<Layout>,
    versions: &mut BTreeSet<Version>,
    version_ranges: &mut Vec<VersionRange>,
    entries: &mut Vec<RawEntry>,
    line: &str,
    line_count: usize,
) -> Result<bool, ParseError> {
    if line.starts_with("COLUMNS") {
        *mode = Mode::Column;
        return Ok(true);
    } else if line.starts_with("COMMENT") {
        return Ok(true);
    } else if line.starts_with("BUILD") || line.starts_with("LAYOUT") {
        match mode {
            Mode::Column | Mode::Build => {
                *mode = Mode::Build;
            }
            Mode::Entry => {
                normalize_versions(versions, version_ranges);

                let database = RawDefinition::new(
                    versions.clone(),
                    version_ranges.clone(),
                    layouts.clone(),
                    entries.clone(),
                );

                file.add_database(database);

                versions.clear();
                version_ranges.clear();
                layouts.clear();
                entries.clear();

                *mode = Mode::Build;
            }
        }

        if let Some(b) = line.strip_prefix("BUILD ") {
            const BUILD_OFFSET: usize = 6;
            if let Some((first, mut b)) = b.split_once(',') {
                versions.insert(string_to_version(first.trim(), line_count, BUILD_OFFSET)?);

                while let Some(i) = b.find(',') {
                    versions.insert(string_to_version(b[..i].trim(), line_count, i)?);

                    b = &b[i + 1..];
                }
            } else if b.contains('-') {
                let i = b.find('-').unwrap();

                let from = string_to_version(&b[..i], line_count, BUILD_OFFSET)?;
                let to = string_to_version(&b[i + 1..], line_count, BUILD_OFFSET + i + 1)?;

                version_ranges.push(VersionRange::new(from, to));
            } else {
                versions.insert(string_to_version(b, line_count, BUILD_OFFSET)?);
            }
        } else if let Some(mut b) = line.strip_prefix("LAYOUT ") {
            if b.contains(',') {
                while let Some(i) = b.find(',') {
                    let value = match u32::from_str_radix(&b[..i], 16) {
                        Ok(i) => i,
                        Err(_) => {
                            return Err(ParseError::new(
                                i,
                                line_count,
                                DbdErrorReason::InvalidLayout(b[..i].to_string()),
                            ));
                        }
                    };
                    b = &b[..i];

                    layouts.insert(Layout::new(value));
                }
            } else {
                const LAYOUT_SPACE_OFFSET: usize = 7;

                let value = match u32::from_str_radix(b, 16) {
                    Ok(i) => i,
                    Err(_) => {
                        return Err(ParseError::new(
                            LAYOUT_SPACE_OFFSET,
                            line_count,
                            DbdErrorReason::InvalidLayout(b.to_string()),
                        ));
                    }
                };

                layouts.insert(Layout::new(value));
            }
        } else {
            unreachable!()
        };

        return Ok(true);
    }

    Ok(false)
}

fn string_to_version(s: &str, line_count: usize, column: usize) -> Result<Version, ParseError> {
    let err = || -> Result<Version, ParseError> {
        Err(ParseError::new(
            column,
            line_count,
            DbdErrorReason::InvalidBuild(s.to_string()),
        ))
    };

    let mut split = s.split('.');

    let major = if let Some(s) = split.next() {
        if let Ok(s) = s.parse::<u8>() {
            s
        } else {
            return err();
        }
    } else {
        return err();
    };

    let minor = if let Some(s) = split.next() {
        if let Ok(s) = s.parse::<u8>() {
            s
        } else {
            return err();
        }
    } else {
        return err();
    };

    let patch = if let Some(s) = split.next() {
        if let Ok(s) = s.parse::<u8>() {
            s
        } else {
            return err();
        }
    } else {
        return err();
    };

    let build = if let Some(s) = split.next() {
        if let Ok(s) = s.parse::<u16>() {
            s
        } else {
            return err();
        }
    } else {
        return err();
    };

    Ok(Version::new(major, minor, patch, build))
}

fn split_comment(line: &str) -> (&str, Option<String>) {
    if let Some((name, comment)) = line.split_once("//") {
        (name.trim(), Some(comment.trim().to_string()))
    } else {
        (line.trim(), None)
    }
}

fn ty_to_type(ty: &str, line_count: usize, column: usize) -> Result<RawType, ParseError> {
    Ok(match ty {
        "int" => RawType::Int,
        "float" => RawType::Float,
        "locstring" => RawType::LocString,
        "string" => RawType::String,
        v => {
            return Err(ParseError::new(
                column,
                line_count,
                DbdErrorReason::InvalidType(v.to_string()),
            ));
        }
    })
}
