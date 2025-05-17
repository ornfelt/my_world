mod staticdata;
mod shared;
mod log;
mod config_reader;
mod findfiles;
mod parser;

use indicatif::{ProgressBar, ProgressStyle};
use std::thread;
use std::time::Duration;
use console::Term;
use findfiles::instance as FindFiles;
use parser::Parser as Parser;

fn print_header() {
    write_log_and_print!("WoWParser Version {} ({}) in Development State for {} {} (Revision: {})\n", staticdata::version(), staticdata::codename(), staticdata::os(), staticdata::arch(), staticdata::revision());
    write_log_and_print!("Hash: {}\tDate: {}\n", staticdata::hash(), staticdata::date());
    write_log_and_print!("\n");
    write_log_and_print!("Tool to Parse World of Warcraft files (DBC DB2 ADB WDB).\n");
    write_log_and_print!("Copyright(c) 2023 Carlos Ramzuel - Huamantla, Tlaxcala, Mexico.\n");
    write_log_and_print!("\n");

    write_log!("====================================LOG FILE START====================================\n");
}

fn pass1_loadconfig() {
    let config = config_reader::ConfigReader::new();

    if !config.load_configuration_file() {
        write_log!("Trying to find files in recursive mode with the following extensions: dbc db2 adb\n");

        FindFiles().file_to_find(".", "", "", true, "dbc", shared::OutputFormat::new(), 0);
        FindFiles().file_to_find(".", "", "", true, "db2", shared::OutputFormat::new(), 0);
        FindFiles().file_to_find(".", "", "", true, "adb", shared::OutputFormat::new(), 0);

        if FindFiles().list_empty() {
            write_log_and_print!("No dbc, db2, or adb files found using recursive mode.\n");
        }

        return;
    }

    if FindFiles().list_empty() {
        write_log_and_print!("Configuration file loaded, but no files found.\n");
    }
}

fn pass2_printfilestolog() {
    if FindFiles().list_empty() {
        return;
    }

    print!("-----> Printing all files found in the log... ");
    FindFiles().print_all_file_names_by_file_type();
    print!("DONE.\n");
}

fn pass3_checkheadersanddataconsistency() {
    if FindFiles().list_empty() {
        return;
    }

    write_log!("\n");
    write_log_and_print!("-----> Checking header and data consistency of each file added to parse it...\n");

    let xml_file_info = FindFiles().xml_file_info();
    let bar = ProgressBar::new(xml_file_info.len() as u64);
    let style = ProgressStyle::default_bar()
        .template("  Loading file: {msg:34} [{elapsed_precise}] [{bar:40.cyan/blue}] ({pos}/{len} {percent}%)").expect("Failed")
        .progress_chars("* ");

    bar.set_style(style);

    for (current_file_name, current_file_info) in xml_file_info {
        let new_filename: String;
        if current_file_name.len() > 34 {
            new_filename = format!("...{}", &current_file_name[current_file_name.len() - 31..]);
        } else {
            new_filename = current_file_name.to_string();
        }
        bar.set_message(new_filename);
        thread::sleep(Duration::from_secs(1));

        let mut parser = Parser::new(current_file_info);

        if parser.load() {
            parser.parse_file();
        }

        bar.inc(1);
    }

    bar.set_message("");
    bar.finish();

    let term = Term::stdout();
    term.clear_line().unwrap();
}
fn print_end() {
    write_log_and_print!("-----> Finished\n");
    write_log!("=====================================LOG FILE END=====================================\n");
    getch!();
}

fn main() {
    print_header();

    pass1_loadconfig();

    pass2_printfilestolog();

    pass3_checkheadersanddataconsistency();

    print_end();
}
