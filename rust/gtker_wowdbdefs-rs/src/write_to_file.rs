use crate::writer::Writer;
use crate::RawDbdFile;

/// Write the [`RawDbdFile`] to a string in the `.dbd` format.
///
/// Versions will be written deduplicated.
pub fn write_to_file(file: &RawDbdFile) -> String {
    let mut s = Writer::new();
    s.wln("COLUMNS");

    for column in file.columns.values() {
        let foreign_key = if let Some(foreign_key) = &column.foreign_key {
            foreign_key.to_string()
        } else {
            "".to_string()
        };

        s.w(format!(
            "{ty}{foreign_key} {name}",
            ty = column.ty,
            name = column.name
        ));
        if !column.verified {
            s.w("?");
        }

        if let Some(comment) = &column.comment {
            s.wln(format!(" // {}", comment));
        } else {
            s.newline();
        }
    }

    s.newline();

    for definition in &file.definitions {
        if !definition.versions.is_empty() {
            s.w("BUILD ");

            for (i, v) in definition.versions.iter().enumerate() {
                if i != 0 {
                    s.w(", ");
                }
                s.w(format!("{}", v));
            }

            s.newline();
        }

        for v in &definition.version_ranges {
            s.wln(format!("BUILD {}-{}", v.from, v.to));
        }

        for l in &definition.layouts {
            s.wln(format!("LAYOUT {}", l));
        }

        for entry in &definition.entries {
            let name = entry.name.as_str();

            let tags = if entry.has_any_tag() {
                let mut s = "$".to_string();
                if entry.primary_key {
                    s += "id";
                }

                if !entry.inline {
                    if entry.primary_key {
                        s += ",";
                    }

                    s += "noninline";
                }

                if entry.relation {
                    if entry.primary_key || !entry.inline {
                        s += ",";
                    }

                    s += "relation";
                }

                s += "$";

                s
            } else {
                "".to_string()
            };

            let integer_width = if let Some(width) = entry.integer_width {
                let unsigned = if entry.unsigned { "u" } else { "" };

                format!("<{width}{unsigned}>")
            } else {
                "".to_string()
            };

            let array_size = if let Some(array_size) = entry.array_size {
                format!("[{array_size}]")
            } else {
                "".to_string()
            };

            s.wln(format!("{tags}{name}{integer_width}{array_size}"));
        }

        s.newline();
    }

    s.inner
}
