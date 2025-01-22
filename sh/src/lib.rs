use std::collections::HashMap;
use colored::*;

#[derive(Debug, Clone, PartialEq)]
pub enum HighlightType {
    Keyword,
    Operator,
    String,
    Number,
    Comment,
    Identifier,
    Symbol,
    Boolean,
    Error,
}

pub struct SyntaxHighlighter {
    keywords: HashMap<String, HighlightType>,
    operators: Vec<&'static str>,
    symbols: Vec<char>,
}

impl Default for SyntaxHighlighter {
    fn default() -> Self {
        let mut keywords = HashMap::new();

        // Keywords
        for keyword in [
            "if", "let", "begin", "end", "return", "then", "else", "elif",
            "struct", "enum", "type", "new", "loop", "recur", "while", "for",
            "do", "pub", "as", "proc", "import", "use"
        ].iter() {
            keywords.insert(keyword.to_string(), HighlightType::Keyword);
        }

        // Boolean literals
        for bool_lit in ["true", "false", "nil"].iter() {
            keywords.insert(bool_lit.to_string(), HighlightType::Boolean);
        }

        let operators = vec![
            "::", ":=", "==", "!=", ">=", "<=", "&&", "||", "+=", "-=",
            "=>", "+", "-", "*", "/", "%", "!", "&"
        ];

        let symbols = vec![
            '~', '(', ')', '{', '}', '<', '>', '[', ']', ';', '?', ',', ':', '@', '.'
        ];

        Self {
            keywords,
            operators,
            symbols,
        }
    }
}

impl SyntaxHighlighter {
    pub fn highlight(&self, code: &str) -> String {
        let mut result = String::new();
        let mut chars = code.chars().peekable();
        let mut in_comment = false;

        while let Some(c) = chars.next() {
            match c {
                // Handle comments
                '-' if chars.peek() == Some(&'-') => {
                    in_comment = true;
                    result.push_str(&format!("{}", "-".dimmed())); // Dimmed for comments
                }
                '\n' => {
                    in_comment = false;
                    result.push('\n');
                }
                _ if in_comment => {
                    result.push_str(&format!("{}", c.to_string().dimmed()));
                    continue;
                }

                // Handle strings
                '"' => {
                    result.push_str(&format!("{}", "\"".yellow())); // Yellow for strings
                    while let Some(next) = chars.next() {
                        result.push_str(&format!("{}", next.to_string().yellow()));
                        if next == '"' && chars.peek() != Some(&'"') {
                            break;
                        }
                    }
                }

                // Handle numbers
                c if c.is_ascii_digit() => {
                    result.push_str(&format!("{}", c.to_string().green())); // Green for numbers
                    while let Some(&next) = chars.peek() {
                        if next.is_ascii_digit() || next == '.' || next == '_' {
                            result.push_str(&format!("{}", chars.next().unwrap().to_string().green()));
                        } else {
                            break;
                        }
                    }
                }

                // Handle identifiers and keywords
                c if c.is_alphabetic() || c == '_' => {
                    let mut identifier = String::new();
                    identifier.push(c);

                    while let Some(&next) = chars.peek() {
                        if next.is_alphanumeric() || next == '_' {
                            identifier.push(chars.next().unwrap());
                        } else {
                            break;
                        }
                    }

                    match self.keywords.get(&identifier) {
                        Some(HighlightType::Keyword) => {
                            result.push_str(&format!("{}", identifier.blue())); // Blue for keywords
                        }
                        Some(HighlightType::Boolean) => {
                            result.push_str(&format!("{}", identifier.magenta())); // Magenta for booleans
                        }
                        _ => {
                            result.push_str(&format!("{}", identifier.white())); // White for identifiers
                        }
                    }
                }

                // Handle operators
                c => {
                    let mut op = String::new();
                    op.push(c);

                    if let Some(&next) = chars.peek() {
                        let potential_op = format!("{}{}", c, next);
                        if self.operators.contains(&potential_op.as_str()) {
                            op.push(chars.next().unwrap());
                        }
                    }

                    if self.operators.contains(&op.as_str()) {
                        result.push_str(&format!("{}", op.cyan())); // Cyan for operators
                    } else if self.symbols.contains(&c) {
                        result.push_str(&format!("{}", c.to_string().bright_cyan())); // Bright cyan for symbols
                    } else if !c.is_whitespace() {
                        result.push(c);
                    } else {
                        result.push(c);
                    }
                }
            }
        }

        result
    }
}