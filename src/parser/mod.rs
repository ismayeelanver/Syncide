pub mod ast;
pub mod lexer;
pub mod error;

use ast::*;
use error::{ ExpectedFound, ExpectedMultipleFound };
use lexer::*;
use once_cell::sync::Lazy;
use rustc_hash::FxHashMap;

// Precedence table for binary operators
const PRECEDENCE: Lazy<FxHashMap<Token, usize>> = Lazy::new(|| {
    FxHashMap::from_iter([
        (Token::Colon, 90),
        (Token::Mul, 80),
        (Token::Div, 80),
        (Token::Mod, 80),
        (Token::Plus, 70),
        (Token::Minus, 70),
        (Token::Concat, 70),
        (Token::LeftAngle, 60),
        (Token::RightAngle, 60),
        (Token::LesserEqual, 60),
        (Token::GreaterEqual, 60),
        (Token::Equal, 50),
        (Token::BangEqual, 50),
        (Token::And, 40),
        (Token::Or, 40),
        (Token::PlusEqual, 30),
        (Token::MinusEqual, 30),
        (Token::MutAssignment, 30),
    ])
});

#[derive(Debug, Clone)]
pub struct Parser {
    tokens: Vec<TokenWithContext>,
    pos: usize,
    current_token: TokenWithContext,
    file: String,
}

impl Parser {
    pub fn new(tokens: Vec<TokenWithContext>, file: String) -> Self {
        let current_token = tokens
            .get(0)
            .cloned()
            .unwrap_or_else(|| TokenWithContext::default());
        Self {
            tokens,
            pos: 0,
            current_token,
            file,
        }
    }

    pub fn advance(&mut self) {
        if self.pos < self.tokens.len() - 1 {
            self.pos += 1;
            self.current_token = self.tokens[self.pos].clone();
        }
    }

    pub fn at(&self) -> &TokenWithContext {
        &self.current_token
    }

    pub fn consume(&mut self, token: Token) {
        if self.at().token == token {
            self.advance();
        } else {
            ExpectedFound::new(
                &self.file,
                self.at().position.line,
                self.at().position.column,
                format!("{:?}", token).as_str(),
                format!("{:?}", self.at().token).as_str()
            );
        }
    }

    pub fn parse(&mut self) -> Stmt {
        let mut statements = Vec::new();
        while self.not_eof() {
            let stmt = self.statement();
            statements.push(stmt);
        }
        Stmt::Program(statements)
    }

    pub fn statement(&mut self) -> Stmt {
        match self.at().token {
            Token::Let => self.let_statement(),
            Token::Return => self.return_statement(),
            Token::If => self.if_statement(),
            _ => {
                let expr = self.expression();
                self.consume(Token::Semicolon);
                Stmt::Expr(expr)
            }
        }
    }

    pub fn not_eof(&self) -> bool {
        self.at().token != Token::Eof
    }

    pub fn if_statement(&mut self) -> Stmt {
        self.consume(Token::If);

        // Parse condition (now handling parentheses)
        let condition = if self.at().token == Token::LeftParen {
            self.consume(Token::LeftParen);
            let expr = self.expression();
            self.consume(Token::RightParen);
            expr
        } else {
            self.expression()
        };

        self.consume(Token::Then);

        // Parse the main if block
        let mut consequence = Vec::new();
        while self.not_eof() && !matches!(self.at().token, Token::Else | Token::Elif | Token::End) {
            consequence.push(self.statement());
        }

        let mut alternative = Vec::new();

        // Handle elif and else cases
        let mut final_alternative = None;

        while self.not_eof() {
            match self.at().token {
                Token::Elif => {
                    self.consume(Token::Elif);

                    // Parse elif condition
                    let elif_condition = if self.at().token == Token::LeftParen {
                        self.consume(Token::LeftParen);
                        let expr = self.expression();
                        self.consume(Token::RightParen);
                        expr
                    } else {
                        self.expression()
                    };

                    self.consume(Token::Then);

                    // Parse elif block
                    let mut elif_block = Vec::new();
                    while
                        self.not_eof() &&
                        !matches!(self.at().token, Token::Else | Token::Elif | Token::End)
                    {
                        elif_block.push(self.statement());
                    }

                    // Create new if statement for elif
                    let elif_stmt = Stmt::If(
                        elif_condition,
                        Box::new(Stmt::Block { block: elif_block }),
                        Box::new(Stmt::Block { block: vec![] })
                    );

                    alternative.push(elif_stmt);
                }
                Token::Else => {
                    self.consume(Token::Else);
                    while self.not_eof() && self.at().token != Token::End {
                        alternative.push(self.statement());
                    }
                    final_alternative = Some(Stmt::Block { block: alternative });
                    break;
                }
                Token::End => {
                    if !alternative.is_empty() {
                        final_alternative = Some(Stmt::Block { block: alternative });
                    }
                    break;
                }
                _ => {
                    ExpectedMultipleFound::new(
                        &self.file,
                        self.at().position.line,
                        self.at().position.column,
                        vec!["Else", "Elif", "End"],
                        format!("{:?}", self.at().token).as_str()
                    );
                    break;
                }
            }
        }

        self.consume(Token::End);

        // Create the final if statement
        Stmt::If(
            condition,
            Box::new(Stmt::Block { block: consequence }),
            Box::new(final_alternative.unwrap_or(Stmt::Block { block: vec![] }))
        )
    }

    pub fn let_statement(&mut self) -> Stmt {
        self.consume(Token::Let);

        // Get the function or variable name
        let name_token = self.at().clone();
        match name_token.token {
            Token::Identifier(_) => {
                let name = self.eat().lexeme;

                if self.at().token == Token::LeftParen {
                    let mut params = FxHashMap::default();
                    self.consume(Token::LeftParen);

                    while self.not_eof() && self.at().token != Token::RightParen {
                        match self.at().token {
                            Token::Identifier(_) => {
                                let param_name = self.eat().lexeme;
                                self.consume(Token::Colon);
                                let param_type = self._type();
                                params.insert(param_name, param_type);

                                if self.at().token == Token::Comma {
                                    self.consume(Token::Comma);
                                }
                            }
                            _ => {
                                ExpectedFound::new(
                                    &self.file,
                                    self.to_owned().at().position.line,
                                    self.to_owned().at().position.column,
                                    "identifier",
                                    self.to_owned().at().lexeme.as_str()
                                );
                                break;
                            }
                        }
                    }

                    self.consume(Token::RightParen);

                    let mut return_type = Type::Identifier("Void".to_string());
                    if self.at().token == Token::Tilde {
                        self.consume(Token::Tilde);
                        return_type = self._type();
                    }

                    self.consume(Token::ConstAssignment);
                    self.consume(Token::Begin);

                    let mut body = Vec::new();
                    while self.not_eof() && self.at().token != Token::End {
                        body.push(self.statement());
                    }

                    self.consume(Token::End);
                    return Stmt::Function(name, params, return_type, body);
                }

                // Variable declaration handling
                let mut var_type = Type::Identifier("Unknown".to_string());
                if self.at().token == Token::Tilde {
                    self.consume(Token::Tilde);
                    var_type = self._type();
                }

                let is_const = match self.at().token {
                    Token::MutAssignment => {
                        self.consume(Token::MutAssignment);
                        false
                    }
                    Token::ConstAssignment => {
                        self.consume(Token::ConstAssignment);
                        true
                    }
                    _ => {
                        ExpectedMultipleFound::new(
                            &self.file,
                            self.to_owned().at().position.line,
                            self.to_owned().at().position.column,
                            vec![":=", "::"],
                            self.to_owned().at().lexeme.as_str()
                        );
                        true
                    }
                };

                let initializer = self.expression();
                self.consume(Token::Semicolon);
                return Stmt::Variable(name, var_type, initializer, is_const);
            }
            _ => {
                ExpectedFound::new(
                    &self.file,
                    self.to_owned().at().position.line,
                    self.to_owned().at().position.column,
                    "identifier",
                    self.to_owned().at().lexeme.as_str()
                );
                return Stmt::Variable(
                    "Unknown".to_string(),
                    Type::Identifier("Unknown".to_string()),
                    Expr::Nil,
                    true
                );
            }
        }
    }

    pub fn return_statement(&mut self) -> Stmt {
        self.consume(Token::Return);
        let expr = self.expression();
        self.consume(Token::Semicolon);
        Stmt::Return(expr)
    }

    pub fn expression(&mut self) -> Expr {
        self.binary(0)
    }

    pub fn binary(&mut self, precedence: usize) -> Expr {
        let mut left = self.unary();
        while precedence < self.precedence() {
            let op = self.eat();
            let right = self.unary();
            left = Expr::Binary(Box::new(left), op.token, Box::new(right));
        }
        left
    }

    pub fn precedence(&self) -> usize {
        *PRECEDENCE.get(&self.at().token).unwrap_or(&0)
    }

    pub fn unary(&mut self) -> Expr {
        match self.at().token {
            Token::Minus | Token::Bang | Token::Question => {
                let op = self.eat().token;
                Expr::Unary(Box::new(self.unary()), op)
            }
            _ => self.primary(),
        }
    }

    pub fn primary(&mut self) -> Expr {
        let mut expr = match self.to_owned().at().token.clone() {
            Token::Number(n) => Expr::Integer(self.eat_and_parse(n.parse())),
            Token::Float(n) => Expr::Float(self.eat_and_parse(n.parse())),
            Token::String(s) => {
                self.advance();
                Expr::String(s.clone())
            }
            Token::True => {
                self.advance();
                Expr::Boolean(true)
            }
            Token::False => {
                self.advance();
                Expr::Boolean(false)
            }
            Token::Identifier(s) => {
                self.advance();
                Expr::Identifier(s.clone())
            }
            Token::Nil => {
                self.advance();
                Expr::Nil
            }
            Token::LeftParen => {
                self.advance();
                let expr = self.expression();
                self.consume(Token::RightParen);
                Expr::Enclosed(Box::new(expr))
            }
            Token::LeftCurly => {
                self.advance();
                let mut arr = Vec::new();
                while self.not_eof() && self.at().token != Token::RightCurly {
                    arr.push(self.expression());
                    if self.at().token == Token::Comma {
                        self.advance();
                    } else {
                        break;
                    }
                }
                self.consume(Token::RightCurly);
                Expr::Array(arr)
            }
            _ => {
                ExpectedFound::new(
                    &self.file,
                    self.at().position.line,
                    self.at().position.column,
                    "expression",
                    format!("{:?}", self.at().token).as_str()
                );
                Expr::Nil
            }
        };

        // Handle recursive expressions
        loop {
            match self.at().token.clone() {
                Token::LeftParen => {
                    self.advance();
                    let mut args = vec![];
                    while self.not_eof() && self.at().token != Token::RightParen {
                        args.push(self.expression());
                        if self.at().token == Token::Comma {
                            self.advance();
                        }
                    }
                    self.consume(Token::RightParen);
                    expr = Expr::FunctionCall(Box::new(expr), args);
                }
                Token::LeftSquare => {
                    self.advance();
                    let index_expr = self.expression();
                    self.consume(Token::RightSquare);
                    expr = Expr::Member(Box::new(expr), Box::new(index_expr));
                }
                Token::LeftCurly => {
                    let mut struct_name = String::new();
                    match expr {
                        Expr::Identifier(ref name) => {
                            struct_name = name.to_string();
                        }
                        _ => {
                            ExpectedFound::new(
                                &self.file,
                                self.at().position.line,
                                self.at().position.column,
                                "Identifier",
                                format!("{:?}", self.at().token).as_str()
                            );
                        }
                    }
                    self.advance();
                    let mut fields = FxHashMap::default();
                    while self.not_eof() && self.at().token != Token::RightCurly {
                        let field_name = self.eat();
                        let mut field_rname = String::from("Unknown");
                        match field_name.token {
                            Token::Identifier(ref str_name) => {
                                field_rname = str_name.to_string();
                            }
                            _ => {
                                ExpectedFound::new(
                                    &self.file,
                                    self.at().position.line,
                                    self.at().position.column,
                                    "Identifier",
                                    format!("{:?}", self.at().token).as_str()
                                );
                            }
                        }

                        self.consume(Token::Colon);

                        let field_expr = self.expression();

                        fields.insert(field_rname, field_expr);
                        if self.at().token == Token::Comma {
                            self.advance();
                        } else {
                            break;
                        }
                    }
                    self.consume(Token::RightCurly);

                    return Expr::IStruct(struct_name, fields);
                }
                _ => {
                    break;
                }
            }
        }

        expr
    }

    // Helper functions for parsing
    fn eat(&mut self) -> TokenWithContext {
        let token = self.current_token.clone();
        self.advance();
        token
    }

    fn eat_and_parse<T, E>(&mut self, result: Result<T, E>) -> T where E: std::fmt::Display {
        self.advance();
        result.unwrap_or_else(|e| panic!("Parsing error: {}", e))
    }

    pub fn _type(&mut self) -> Type {
        match self.at().token {
            Token::Identifier(ref _type_name) => {
                let base_type = self.eat().lexeme;
                // Check if it's a generic type like Gen<T, T, T>
                if self.at().token == Token::LeftAngle {
                    self.consume(Token::LeftAngle);
                    let mut generic_types = Vec::new();

                    while self.not_eof() && self.at().token != Token::RightAngle {
                        generic_types.push(self._type());
                        if self.at().token == Token::Comma {
                            self.consume(Token::Comma);
                        } else {
                            break;
                        }
                    }
                    self.consume(Token::RightAngle);
                    return Type::Template(base_type, generic_types);
                }
                // Check if it's a function type like Int(Int, Int)
                if self.at().token == Token::LeftParen {
                    self.consume(Token::LeftParen);
                    let mut param_types = Vec::new();
                    while self.not_eof() && self.at().token != Token::RightParen {
                        param_types.push(self._type());
                        if self.at().token == Token::Comma {
                            self.consume(Token::Comma);
                        } else {
                            break;
                        }
                    }
                    self.consume(Token::RightParen);
                    return Type::FunctionPointer(base_type, param_types);
                }
                return Type::Identifier(base_type);
            }
            _ => {
                ExpectedFound::new(
                    &self.file,
                    self.to_owned().at().position.line,
                    self.to_owned().at().position.column,
                    "type",
                    self.to_owned().at().lexeme.as_str()
                );
                return Type::Identifier("Unknown".to_string());
            }
        }
    }
}
