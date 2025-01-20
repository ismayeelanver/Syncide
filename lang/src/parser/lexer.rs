use super::error::{ InvalidFloat, InvalidToken, InvalidString };
use std::fs;
use std::io;

#[derive(Debug, Clone, PartialEq, Hash, Eq)]
pub enum Token {
    // Symbols
    ConstAssignment, // ::
    MutAssignment, // :=
    Tilde, // ~
    RightParen, // )
    LeftParen, // (
    RightCurly, // }
    LeftCurly, // {
    LeftAngle, // <
    RightAngle, // >
    LeftSquare,
    RightSquare,
    Semicolon, // ;
    Plus, // +
    Minus, // -
    Div, // /
    Question, // ?
    Mul, // *
    Mod, // %
    Bang, // !
    Comma, // ,
    Colon, // :
    At, // @
    Dot, // .
    DotDot, // ..

    // Identifiers and literals
    Identifier(String),
    Number(String),
    Float(String),
    True,
    Nil,
    False,
    String(String),

    // Operators
    Equal, // ==
    BangEqual, // !=
    GreaterEqual, // >=
    LesserEqual, // <=
    And, // &&
    Or, // ||
    Concat, // &
    PlusEqual, // +=
    MinusEqual, // -=
    FatArrow, // =>

    // Keywords
    If,
    Else,
    Elif,
    Then,
    Let,
    Begin,
    End,
    Return,
    Struct,
    Enum,
    Type,
    New,
    Loop,
    Do,
    Times,
    While,
    For,
    Pub,
    As,
    Import,
    Use,
    Proc,

    InvalidString(usize, usize),
    InvalidFloat(usize, usize),
    InvalidToken(usize, usize),
    UnterminatedString(usize, usize),

    // EOF
    Eof,
}

impl Default for Token {
    fn default() -> Self {
        Token::Eof
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Default)]
pub struct Position {
    pub line: usize,
    pub column: usize,
}

#[derive(Debug, Clone, PartialEq, Default)]
pub struct TokenWithContext {
    pub token: Token,
    pub lexeme: String,
    pub position: Position,
}

#[derive(Debug, Clone)]
pub struct Lexer {
    pub filename: String,
    input: Vec<char>,
    tokens: Vec<TokenWithContext>,
    current: usize,
    position: Position,
    start_of_token: Position,
}

impl Lexer {
    pub fn new(filename: &str) -> io::Result<Self> {
        let content = fs::read_to_string(filename)?;
        Ok(Self {
            filename: filename.to_string(),
            input: content.chars().collect(),
            tokens: Vec::new(),
            current: 0,
            position: Position { line: 1, column: 1 },
            start_of_token: Position { line: 1, column: 1 },
        })
    }

    pub fn tokenize(&mut self) -> Vec<TokenWithContext> {
        while self.current < self.input.len() {
            self.start_of_token = self.position;
            let c = self.input[self.current];
            match c {
                ',' => self.add_token(Token::Comma, String::from(",")),
                '~' => self.add_token(Token::Tilde, String::from("~")),
                '!' => {
                    if self.match_next('=') {
                        self.add_token(Token::BangEqual, String::from("!="));
                    } else {
                        self.add_token(Token::Bang, String::from("!"));
                    }
                }
                '@' => self.add_token(Token::At, String::from("@")),
                '+' => {
                    if self.peek() == Some('=') {
                        self.advance();
                        self.add_token(Token::PlusEqual, String::from("+="));
                    } else {
                        self.add_token(Token::Plus, String::from("+"));
                    }
                }
                '-' => {
                    if self.peek() == Some('-') {
                        // Skip comment
                        while self.peek() != Some('\n') && self.current < self.input.len() {
                            self.advance();
                        }
                    } else if self.peek() == Some('=') {
                        self.advance();
                        self.add_token(Token::MinusEqual, String::from("-="));
                    } else {
                        self.add_token(Token::Minus, String::from("-"));
                    }
                }
                '?' => self.add_token(Token::Question, String::from("?")),
                '*' => self.add_token(Token::Mul, String::from("*")),
                '/' => self.add_token(Token::Div, String::from("/")),
                '=' => {
                    if self.match_next('=') {
                        self.add_token(Token::Equal, String::from("=="));
                    } else if self.match_next('>') {
                        self.add_token(Token::FatArrow, String::from("=>"));
                    } else {
                        self.add_token(
                            Token::InvalidToken(self.position.line, self.position.column),
                            String::from("=")
                        );
                    }
                }
                '%' => self.add_token(Token::Mod, String::from("%")),
                ';' => self.add_token(Token::Semicolon, String::from(";")),
                ':' => {
                    if self.match_next(':') {
                        self.add_token(Token::ConstAssignment, String::from("::"));
                    } else if self.match_next('=') {
                        self.add_token(Token::MutAssignment, String::from(":="));
                    } else {
                        self.add_token(Token::Colon, String::from(":"));
                    }
                }
                '&' => {
                    if self.match_next('&') {
                        self.add_token(Token::And, String::from("&&"));
                    } else {
                        self.add_token(Token::Concat, String::from("&"));
                    }
                }
                '"' => self.string(),
                '{' => self.add_token(Token::LeftCurly, String::from("{")),
                '}' => self.add_token(Token::RightCurly, String::from("}")),
                '(' => self.add_token(Token::LeftParen, String::from("(")),
                ')' => self.add_token(Token::RightParen, String::from(")")),
                '[' => self.add_token(Token::LeftSquare, String::from("[")),
                ']' => self.add_token(Token::RightSquare, String::from("]")),

                '.' => {
                    if self.peek() == Some('.') {
                        self.advance();
                        self.add_token(Token::DotDot, String::from(".."));
                    } else {
                        self.add_token(Token::Dot, String::from("."));
                    }
                }
                '<' => {
                    if self.match_next('=') {
                        self.add_token(Token::LesserEqual, String::from("<="));
                    } else {
                        self.add_token(Token::LeftAngle, String::from("<"));
                    }
                }
                '>' => {
                    if self.match_next('=') {
                        self.add_token(Token::GreaterEqual, String::from(">="));
                    } else {
                        self.add_token(Token::RightAngle, String::from(">"));
                    }
                }
                '\n' => {
                    self.position.line += 1;
                    self.position.column = 1;
                }
                ' ' | '\t' | '\r' => {
                    self.position.column += 1;
                }
                _ => {
                    if c.is_digit(10) {
                        self.number();
                    } else if c.is_alphabetic() || c == '_' {
                        self.identifier();
                    } else {
                        self.add_token(
                            Token::InvalidToken(self.position.line, self.position.column),
                            String::from(c.to_string())
                        );
                    }
                }
            }
            if c != '\n' && c != ' ' && c != '\t' && c != '\r' {
                self.advance();
            } else {
                self.current += 1;
            }
        }

        self.add_token(Token::Eof, String::from("\0"));
        std::mem::take(&mut self.tokens)
    }

    fn string(&mut self) {
        let start_pos = self.position;
        self.advance(); // Skip opening quote
        let mut value = String::new();
        let mut escape = false;

        while self.current < self.input.len() {
            let c = self.input[self.current];

            if escape {
                match c {
                    'n' => value.push('\n'),

                    'r' => value.push('\r'),
                    't' => value.push('\t'),
                    '"' => value.push('"'),
                    '\\' => value.push('\\'),
                    _ => {
                        self.advance();
                        self.add_token(
                            Token::InvalidString(self.position.line, self.position.column),
                            String::from("Invalid escape sequence")
                        );
                        return;
                    }
                }
                escape = false;
            } else if c == '\\' {
                escape = true;
            } else if c == '"' {
                self.add_token(Token::String(value.clone()), value);
                return;
            } else {
                value.push(c);
            }
            self.advance();
        }

        self.add_token(
            Token::UnterminatedString(start_pos.line, start_pos.column),
            String::from("Unterminated string")
        );
    }

    fn number(&mut self) {
        let start = self.current;
        let mut is_float = false;
        let mut has_decimal: bool = false;

        while let Some(c) = self.peek() {
            if c.is_digit(10) {
                self.advance();
            } else if c == '.' && !has_decimal {
                has_decimal = true;
                is_float = true;
                self.advance();
                if !self.peek().map_or(false, |next| next.is_digit(10)) {
                    self.add_token(
                        Token::InvalidFloat(self.position.line, self.position.column),
                        String::from("Invalid float number")
                    );
                    return;
                }
            } else if c == '_' {
                self.advance();
            } else {
                break;
            }
        }

        let number: String = self.input[start..=self.current]
            .iter()
            .filter(|&&c| c != '_')
            .collect();

        if is_float {
            self.add_token(Token::Float(number.clone()), number);
        } else {
            self.add_token(Token::Number(number.clone()), number);
        }
    }

    fn identifier(&mut self) {
        let start = self.current;
        while let Some(c) = self.peek() {
            if c.is_alphanumeric() || c == '_' {
                self.advance();
            } else {
                break;
            }
        }

        let text: String = self.input[start..=self.current].iter().collect();
        let token = match text.as_str() {
            "if" => Token::If,
            "let" => Token::Let,
            "begin" => Token::Begin,
            "end" => Token::End,
            "return" => Token::Return,
            "true" => Token::True,
            "false" => Token::False,
            "nil" => Token::Nil,
            "then" => Token::Then,
            "else" => Token::Else,
            "elif" => Token::Elif,
            "struct" => Token::Struct,
            "enum" => Token::Enum,
            "type" => Token::Type,
            "new" => Token::New,
            "loop" => Token::Loop,
            "recur" => Token::Times,
            "while" => Token::While,
            "for" => Token::For,
            "do" => Token::Do,
            "pub" => Token::Pub,
            "as" => Token::As,
            "proc" => Token::Proc,
            _ => Token::Identifier(text.clone()),
        };

        self.add_token(token, text);
    }

    fn add_token(&mut self, token: Token, lexeme: String) {
        self.tokens.push(TokenWithContext {
            token,
            lexeme,
            position: self.start_of_token,
        });
    }

    fn advance(&mut self) {
        self.current += 1;
        self.position.column += 1;
    }

    fn peek(&self) -> Option<char> {
        self.input.get(self.current + 1).copied()
    }

    fn match_next(&mut self, expected: char) -> bool {
        if self.peek() == Some(expected) {
            self.advance();
            true
        } else {
            false
        }
    }
}
