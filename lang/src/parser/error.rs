use std::fs::File;
use std::io::{BufRead, BufReader};
use colored::*;
use sh::SyntaxHighlighter;

#[derive(Debug)]
pub enum CompilerError {
    InvalidFloat(String, usize, usize),
    InvalidString(String, usize, usize),
    InvalidToken(String, usize, usize),
    ExpectedFound(String, usize, usize, String, String),
    ExpectedMultipleFound(String, usize, usize, Vec<String>, String),
}

pub struct ErrorDisplay {
    file: String,
    line: usize,
    column: usize,
    error_type: String,
}

impl ErrorDisplay {
    fn new(file: &str, line: usize, column: usize, error_type: &str) -> Self {
        Self {
            file: file.to_string(),
            line,
            column,
            error_type: error_type.to_string(),
        }
    }

    fn display(&self, extra_info: Option<String>) {
        let line_of_code = get_line(&self.file, self.line);

        eprintln!(
            "{} {} {}",
            format!("({} ∷ {})", self.line, self.column).bold().black(),
            "[Error ✘]".red().bold(),
            format!("[{}]", self.file).blue().bold()
        );

        eprintln!(
            "↪ 〈{} 〉",
            self.error_type.red().underline().bold().italic()
        );
        eprintln!("");

        eprintln!("\t{} ║ {}", self.line, line_of_code);

        let total_line_len = String::from_utf8(
            strip_ansi_escapes::strip(&line_of_code.as_bytes()).unwrap()
        ).unwrap().len();
        
        let remaining_len = total_line_len - (self.column - 1) - 1;
        eprintln!(
            "\t{}   {}{}{}",
            " ".repeat(self.line.to_string().len()),
            " ".repeat(self.column - 1).blue(),
            "↑".green().bold(),
            "─".repeat(remaining_len).red()
        );

        if let Some(info) = extra_info {
            eprintln!("{}", info.blue());
        }
    }
}

fn get_line(filename: &str, line_number: usize) -> String {
    let file = File::open(filename).unwrap_or_else(|_| {
        panic!("Error: Unable to open file: {}", filename);
    });
    let reader = BufReader::new(file);
    let highlighter = SyntaxHighlighter::default();
    let line = reader
        .lines()
        .nth(line_number - 1)
        .unwrap_or(Ok("".to_string()))
        .unwrap_or_default();

    highlighter.highlight(&line)
}

pub struct InvalidFloat;
impl InvalidFloat {
    pub fn new(file: &str, line: usize, column: usize) -> Result<(), CompilerError> {
        let error = ErrorDisplay::new(file, line, column, "Invalid Float");
        error.display(None);
        Err(CompilerError::InvalidFloat(file.to_string(), line, column))
    }
}

pub struct InvalidString;
impl InvalidString {
    pub fn new(file: &str, line: usize, column: usize) -> Result<(), CompilerError> {
        let error = ErrorDisplay::new(file, line, column, "Invalid String");
        error.display(None);
        Err(CompilerError::InvalidString(file.to_string(), line, column))
    }
}

pub struct InvalidToken;
impl InvalidToken {
    pub fn new(file: &str, line: usize, column: usize) -> Result<(), CompilerError> {
        let error = ErrorDisplay::new(file, line, column, "Invalid Token");
        error.display(None);
        Err(CompilerError::InvalidToken(file.to_string(), line, column))
    }
}

pub struct ExpectedFound;
impl ExpectedFound {
    pub fn new(file: &str, line: usize, column: usize, expected: &str, found: &str) -> Result<(), CompilerError> {
        let error = ErrorDisplay::new(file, line, column, "Wrong Token Found");
        error.display(Some(format!("• [Expected: {} But Found: {}]", expected, found)));
        Err(CompilerError::ExpectedFound(
            file.to_string(),
            line,
            column,
            expected.to_string(),
            found.to_string()
        ))
    }
}

pub struct ExpectedMultipleFound;
impl ExpectedMultipleFound {
    pub fn new(file: &str, line: usize, column: usize, expected: Vec<&str>, found: &str) -> Result<(), CompilerError> {
        let expected_string = expected.join(" Or ").trim_end_matches(" Or ").to_string();
        let error = ErrorDisplay::new(file, line, column, "Wrong Token Found");
        error.display(Some(format!(
            "Expected: {} But Found: {}",
            expected_string, found
        )));
        Err(CompilerError::ExpectedMultipleFound(
            file.to_string(),
            line,
            column,
            expected.iter().map(|&s| s.to_string()).collect(),
            found.to_string()
        ))
    }
}