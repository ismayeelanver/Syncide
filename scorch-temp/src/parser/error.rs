use std::fs::File;
use std::io::{ BufRead, BufReader };
use std::process::exit;
use colored::*;

fn get_line(filename: &str, line_number: usize) -> String {
    let file = File::open(filename).unwrap_or_else(|_| {
        eprintln!("Error: Unable to open file.");
        exit(1);
    });
    let reader = BufReader::new(file);
    reader
        .lines()
        .nth(line_number - 1)
        .unwrap_or(Ok("".to_string()))
        .unwrap_or_default()
}


pub struct InvalidFloat;

impl InvalidFloat {
    pub fn new(file: &str, line: usize, column: usize) -> Self {
        let line_of_code = get_line(file, line);

        eprintln!(
            "{} {} {}",
            ("(".to_owned() + line.to_string().as_str() + " ∷ " + column.to_string().as_str() + ")")
                .bold()
                .black(),
            "[Error ✘]".red(),
            ("[".to_owned() + file + "]").blue()
        );
        eprintln!("╰─▶ 〈{} 〉", "Invalid Float".red().underline().bold().italic());
        eprintln!("");
        eprintln!("\t{} ║ {}", line, line_of_code.purple());
        let total_line_len = line_of_code.len();
        let remaining_len = total_line_len - (column - 1) - 1;
        eprintln!(
            "\t{}   {}{}{}",
            " ".repeat(line.to_string().len()),
            " "
                .repeat(column - 1)
                .blue()
                .bold(),
            "↑".green().bold(),
            "─".repeat(remaining_len).red()
        );

        exit(1);
    }
}

pub struct InvalidString;

impl InvalidString {
    pub fn new(file: &str, line: usize, column: usize) -> Self {
        let line_of_code = get_line(file, line);

        eprintln!(
            "{} {} {}",
            ("(".to_owned() + line.to_string().as_str() + " ∷ " + column.to_string().as_str() + ")")
                .bold()
                .black(),
            "[Error ✘]".red().bold(),
            ("[".to_owned() + file + "]").blue().bold()
        );
        eprintln!("╰─▶ 〈{} 〉", "Invalid String".red().underline().bold().italic());
        eprintln!("");
        eprintln!("\t{} ║ {}", line, line_of_code.purple());
        let total_line_len = line_of_code.len();
        let remaining_len = total_line_len - (column - 1) - 1;
        eprintln!(
            "\t{}   {}{}{}",
            " ".repeat(line.to_string().len()),
            " ".repeat(column - 1).blue(),
            "↑".green().bold(),
            "─".repeat(remaining_len).red()
        );

        exit(1);

    }
}

pub struct InvalidToken;

impl InvalidToken {
    pub fn new(file: &str, line: usize, column: usize) -> Self {
        let line_of_code = get_line(file, line);

        eprintln!(
            "{} {} {}",
            ("(".to_owned() + line.to_string().as_str() + " ∷ " + column.to_string().as_str() + ")")
                .bold()
                .black()
                .bold(),
            "[Error ✘]".red().bold(),
            ("[".to_owned() + file + "]").blue().bold()
        );
        eprintln!("╰─▶ 〈{} 〉", "Invalid Token".red().underline().bold().italic());
        eprintln!("");
        eprintln!("\t{} ║ {}", line, line_of_code.purple());
        let total_line_len = line_of_code.len();
        let remaining_len = total_line_len - (column - 1) - 1;
        eprintln!(
            "\t{}   {}{}{}",
            " ".repeat(line.to_string().len()),
            " ".repeat(column - 1).blue(),
            "↑".green().bold(),
            "─".repeat(remaining_len).red()
        );

        exit(1);

    }
}

pub struct ExpectedFound;

impl ExpectedFound {
    pub fn new(file: &str, line: usize, column: usize, expected: &str, found: &str) -> Self {
        let line_of_code = get_line(file, line);

        eprintln!(
            "{} {} {}",
            ("(".to_owned() + line.to_string().as_str() + " ∷ " + column.to_string().as_str() + ")")
                .bold()
                .black(),
            "[Error ✘]".red(),
            ("[".to_owned() + file + "]").blue().bold()
        );
        eprintln!("╰─▶ 〈{} 〉", "Wrong Token Found".red().underline().bold().italic());
        eprintln!("");
        eprintln!("\t{} ║ {}", line, line_of_code.purple());
        let total_line_len = line_of_code.len();
        let remaining_len = total_line_len - (column - 1) - 1;
        eprintln!(
            "\t{}   {}{}{}",
            " ".repeat(line.to_string().len()),
            " ".repeat(column - 1).blue(),
            "↑".green().bold(),
            "─".repeat(remaining_len).red()
        );
        eprintln!("{}", format!("• [Expected: {} But Found: {}]", expected, found).blue());

        exit(1);
    }
}

pub struct ExpectedMultipleFound;

impl ExpectedMultipleFound {
    pub fn new(file: &str, line: usize, column: usize, expected: Vec<&str>, found: &str) -> Self {
        let line_of_code = get_line(file, line);
        let expected_string = expected.join(" Or "); // if is at end then no " ,"
        let expected_string = expected_string.trim_end_matches(" Or ");
        eprintln!(
            "{} {} {}",
            ("(".to_owned() + line.to_string().as_str() + " ∷ " + column.to_string().as_str() + ")")
                .bold()
                .black(),
            "[Error ✘]".red(),
            ("[".to_owned() + file + "]").blue().bold()
        );
        eprintln!("╰─▶ 〈{} 〉", "Wrong Token Found".red().underline().bold().italic());
        eprintln!("");
        eprintln!("\t{} ║ {}", line, line_of_code.purple());
        let total_line_len = line_of_code.len();
        let remaining_len = total_line_len - (column - 1) - 1;
        eprintln!(
            "\t{}   {}{}{} {}",
            " ".repeat(line.to_string().len()),
            " ".repeat(column - 1).blue(),
            "↑".green().bold(),
            "─".repeat(remaining_len).red(),
            format!("Expected: {} But Found: {}", expected_string, found).blue().blue()
        );

        exit(1);
    }
}
