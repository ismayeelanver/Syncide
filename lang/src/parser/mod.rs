pub mod ast;
pub mod lexer;
pub mod error;

use ast::*;
use colored::Colorize;
use error::{
    ExpectedFound,
    ExpectedMultipleFound,
    InvalidFloat,
    InvalidString,
    InvalidToken,
    CompilerError,
};
use lexer::*;
use once_cell::sync::Lazy;
use rustc_hash::FxHashMap;

// Precedence table for binary operators
const PRECEDENCE: Lazy<FxHashMap<Token, usize>> = Lazy::new(|| {
    FxHashMap::from_iter([
        (Token::Colon, 90),
        (Token::As, 85),
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

    pub fn consume(&mut self, token: Token) -> Result<(), CompilerError> {
        if self.at().token == token {
            self.advance();
            Ok(())
        } else {
            ExpectedFound::new(
                &self.file,
                self.at().position.line,
                self.at().position.column,
                format!("{:?}", token).as_str(),
                format!("{:?}", self.at().token).as_str()
            )
        }
    }

    pub fn parse(&mut self) -> Result<Stmt, CompilerError> {
        let mut errors_in_total: Vec<CompilerError> = vec![];
        let mut had_error = false;

        for token in &self.tokens {
            match token.token {
                Token::InvalidFloat(ref line, ref col) => {
                    had_error = true;
                    let err = InvalidFloat::new(&self.file, *line, *col);
                    if let Err(e) = err {
                        errors_in_total.push(e);
                    }
                }
                Token::InvalidToken(ref line, ref col) => {
                    had_error = true;
                    let err = InvalidToken::new(&self.file, *line, *col);
                    if let Err(e) = err {
                        errors_in_total.push(e);
                    }
                }
                Token::InvalidString(ref line, ref col) => {
                    had_error = true;
                    let err = InvalidString::new(&self.file, *line, *col);
                    if let Err(e) = err {
                        errors_in_total.push(e);
                    }
                }
                _ => {}
            }
        }

        if had_error {
            eprintln!("Total errors: {}", errors_in_total.len().to_string().red());
            // Return first error if there were any
            if let Some(error) = errors_in_total.into_iter().next() {
                return Err(error);
            }
        }

        let mut statements = Vec::new();
        while self.not_eof() {
            let stmt = self.statement()?;
            statements.push(stmt);
        }
        Ok(Stmt::Program(statements))
    }

    pub fn statement(&mut self) -> Result<Stmt, CompilerError> {
        match self.at().token {
            Token::Let => self.let_statement(),
            Token::Return => self.return_statement(),
            Token::If => self.if_statement(),
            Token::Type => self.type_statement(),
            Token::Loop => self.loop_statement(),
            Token::Import => self.import_statement(),
            Token::Semicolon => {
                self.advance();
                Ok(Stmt::Empty)
            }
            Token::Pub => {
                self.advance();

                let decl = match self.at().token {
                    Token::Type => self.type_statement(),
                    Token::Let => self.let_statement(),
                    _ => {
                        Err(
                            ExpectedMultipleFound::new(
                                &self.file,
                                self.at().position.line,
                                self.at().position.column,
                                vec!["Identifier"],
                                format!("{:?}", self.at().token).as_str()
                            ).unwrap_err()
                        )
                    }
                };

                Ok(Stmt::Pub(Box::new(decl?)))
            }
            _ => {
                let expr = self.expression()?;
                self.consume(Token::Semicolon)?;
                Ok(Stmt::Expr(expr))
            }
        }
    }

    pub fn not_eof(&self) -> bool {
        self.at().token != Token::Eof
    }

    pub fn struct_type(&mut self) -> Result<DType, CompilerError> {
        self.consume(Token::Struct)?;
        let mut fields: std::collections::HashMap<
            String,
            Type,
            std::hash::BuildHasherDefault<rustc_hash::FxHasher>
        > = FxHashMap::default();
        while self.not_eof() && self.at().token != Token::End {
            let field_name = self.eat().token;
            let mut field_rname = String::new();
            match field_name {
                Token::Identifier(ref name) => {
                    field_rname = name.clone();
                }
                _ => {
                    return Err(
                        ExpectedFound::new(
                            &self.file,
                            self.at().position.line,
                            self.at().position.column,
                            "Identifier",
                            format!("{:?}", self.at().token).as_str()
                        ).unwrap_err()
                    );
                }
            }

            self.consume(Token::Colon)?;

            let field_type = self._type()?;

            fields.insert(field_rname, field_type);

            if self.at().token == Token::Comma {
                self.advance();
            } else {
                break;
            }
        }
        self.consume(Token::End)?;
        return Ok(DType::Struct(fields));
    }

    pub fn new_expr(&mut self) -> Result<Expr, CompilerError> {
        self.consume(Token::New)?;
        let mut fs = FxHashMap::default();

        while self.not_eof() && self.at().token != Token::End {
            let fname = self.eat().token;

            let frname = match fname {
                Token::Identifier(ref name) => name.clone(),
                _ => {
                    return Err(
                        ExpectedFound::new(
                            &self.file,
                            self.at().position.line,
                            self.at().position.column,
                            "Identifier",
                            format!("{:?}", self.at().token).as_str()
                        ).unwrap_err()
                    );
                }
            };

            self.consume(Token::MutAssignment)?;

            let expr = self.expression()?;

            fs.insert(frname, expr);
            if self.at().token == Token::Comma {
                self.advance();
            } else {
                break;
            }
        }
        self.advance();
        Ok(Expr::New(fs))
    }

    pub fn type_statement(&mut self) -> Result<Stmt, CompilerError> {
        self.consume(Token::Type)?;

        let type_rname = self._type()?;

        if let Type::FunctionPointer(_, _) = type_rname {
            return Err(
                ExpectedFound::new(
                    &self.file,
                    self.at().position.line,
                    self.tokens[self.pos - 1].position.column,
                    "Other Type",
                    format!("Function Pointer").as_str()
                ).unwrap_err()
            );
        }

        let dtype = match self.at().token {
            Token::Identifier(_) => {
                let type_ = self._type()?;
                DType::Custom(type_)
            }
            Token::Struct => { self.struct_type()? }
            _ => {
                return Err(ExpectedMultipleFound::new(
                                                    &self.file,
                                                    self.at().position.line,
                                                    self.at().position.column,
                                                    vec!["Struct Implementation", "Identifier"],
                                                    format!("{:?}", self.at().token).as_str()
                                                ).unwrap_err());
            }
        };

        return Ok(Stmt::TypeDeclaration(type_rname, dtype));
    }

    pub fn import_statement(&mut self) -> Result<Stmt, CompilerError> {
        self.advance();
        let mut imports = vec![];
        while self.not_eof() && self.at().token != Token::Semicolon {
            let import_name = self.eat().token;
            let import_rname = match import_name {
                Token::Identifier(ref name) => name.clone(),
                _ => {
                    return Err(
                        ExpectedFound::new(
                            &self.file,
                            self.at().position.line,
                            self.at().position.column,
                            "Identifier",
                            format!("{:?}", self.at().token).as_str()
                        ).unwrap_err()
                    );
                }
            };
            if self.at().token == Token::As {
                self.advance();
                let as_name = self.eat().token;
                let as_rname = match as_name {
                    Token::Identifier(ref name) => name.clone(),
                    _ => {
                        return Err(
                            ExpectedFound::new(
                                &self.file,
                                self.at().position.line,
                                self.at().position.column,
                                "Identifier",
                                format!("{:?}", self.at().token).as_str()
                            ).unwrap_err()
                        );
                    }
                };
                imports.push(as_rname);
            } else {
                imports.push(import_rname.clone());
            }
            if self.at().token == Token::Comma {
                self.advance();
            } else {
                break;
            }
            imports.push(import_rname);
        }

        Ok(Stmt::Import(imports))
    }

    pub fn if_statement(&mut self) -> Result<Stmt, CompilerError> {
        self.consume(Token::If)?;

        // Parse condition (now handling parentheses)
        let condition = if self.at().token == Token::LeftParen {
            self.consume(Token::LeftParen)?;
            let expr = self.expression()?;
            self.consume(Token::RightParen)?;
            expr
        } else {
            self.expression()?
        };

        self.consume(Token::Then)?;

        // Parse the main if block
        let mut consequence = Vec::new();
        while self.not_eof() && !matches!(self.at().token, Token::Else | Token::Elif | Token::End) {
            consequence.push(self.statement()?);
        }

        let mut alternative = Vec::new();

        // Handle elif and else cases
        let mut final_alternative = None;

        while self.not_eof() {
            match self.at().token {
                Token::Elif => {
                    self.consume(Token::Elif)?;

                    // Parse elif condition
                    let elif_condition = if self.at().token == Token::LeftParen {
                        self.consume(Token::LeftParen)?;
                        let expr = self.expression()?;
                        self.consume(Token::RightParen)?;
                        expr
                    } else {
                        self.expression()?
                    };

                    self.consume(Token::Then)?;

                    // Parse elif block
                    let mut elif_block = Vec::new();
                    while
                        self.not_eof() &&
                        !matches!(self.at().token, Token::Else | Token::Elif | Token::End)
                    {
                        elif_block.push(self.statement()?);
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
                    self.consume(Token::Else)?;
                    while self.not_eof() && self.at().token != Token::End {
                        alternative.push(self.statement()?);
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
                    return Err(
                        ExpectedMultipleFound::new(
                            &self.file,
                            self.at().position.line,
                            self.at().position.column,
                            vec!["Else", "Elif", "End"],
                            format!("{:?}", self.at().token).as_str()
                        ).unwrap_err()
                    );
                }
            }
        }

        self.consume(Token::End)?;

        // Create the final if statement
        Ok(
            Stmt::If(
                condition,
                Box::new(Stmt::Block { block: consequence }),
                Box::new(final_alternative.unwrap_or(Stmt::Block { block: vec![] }))
            )
        )
    }

    pub fn loop_statement(&mut self) -> Result<Stmt, CompilerError> {
        self.consume(Token::Loop)?;

        self.consume(Token::Comma)?;

        match self.at().token {
            Token::Do => {
                self.advance();
                let mut statements = vec![];
                while self.not_eof() && self.at().token != Token::End {
                    statements.push(self.statement()?);
                }
                self.advance();
                Ok(Stmt::Do(statements))
            }
            Token::For => {
                self.advance();
                let element = self.eat();
                self.consume(Token::Comma)?;
                let index = self.eat();

                match element.token {
                    Token::Identifier(_) => (),
                    _ => {
                        return Err(
                            ExpectedMultipleFound::new(
                                &self.file,
                                self.at().position.line,
                                self.at().position.column,
                                vec!["Identifier"],
                                format!("{:?}", self.at().token).as_str()
                            ).unwrap_err()
                        );
                    }
                }
                match index.token {
                    Token::Identifier(_) => (),
                    _ => {
                        return Err(
                            ExpectedMultipleFound::new(
                                &self.file,
                                self.at().position.line,
                                self.at().position.column,
                                vec!["Identifier"],
                                format!("{:?}", self.at().token).as_str()
                            ).unwrap_err()
                        );
                    }
                }

                let rel = element.lexeme;
                let idx = index.lexeme;

                self.consume(Token::MutAssignment)?;

                let expr = self.expression()?;

                self.consume(Token::Begin)?;

                let mut statements = vec![];
                while self.not_eof() && self.at().token != Token::End {
                    statements.push(self.statement()?);
                }
                self.advance();

                Ok(Stmt::For(rel, idx, expr, statements))
            }
            Token::Times => {
                self.advance();
                let number = self.expression()?;
                let mut statements = vec![];
                while self.not_eof() && self.at().token != Token::End {
                    statements.push(self.statement()?);
                }
                self.advance();
                Ok(Stmt::Times(number, statements))
            }
            Token::While => {
                self.advance();
                let condition = self.expression()?;
                let mut statements = vec![];
                while self.not_eof() && self.at().token != Token::End {
                    statements.push(self.statement()?);
                }
                self.advance();
                Ok(Stmt::While(condition, statements))
            }
            _ => {
                return Err(
                    ExpectedMultipleFound::new(
                        &self.file,
                        self.at().position.line,
                        self.at().position.column,
                        vec!["Do", "For"],
                        format!("{:?}", self.at().token).as_str()
                    ).unwrap_err()
                );
            }
        }
    }

    pub fn let_statement(&mut self) -> Result<Stmt, CompilerError> {
        self.consume(Token::Let)?;

        // Get the function or variable name
        let name_token = self.at().clone();
        match name_token.token {
            Token::Identifier(_) => {
                let name = self.eat().lexeme;

                if self.at().token == Token::LeftParen {
                    let mut params = FxHashMap::default();
                    self.consume(Token::LeftParen)?;

                    while self.not_eof() && self.at().token != Token::RightParen {
                        match self.at().token {
                            Token::Identifier(_) => {
                                let param_name = self.eat().lexeme;
                                self.consume(Token::Colon)?;
                                let param_type = self._type()?;
                                params.insert(param_name, param_type);

                                if self.at().token == Token::Comma {
                                    self.consume(Token::Comma)?;
                                }
                            }
                            _ => {
                                return Err(
                                    ExpectedFound::new(
                                        &self.file,
                                        self.to_owned().at().position.line,
                                        self.to_owned().at().position.column,
                                        "identifier",
                                        self.to_owned().at().lexeme.as_str()
                                    ).unwrap_err()
                                );
                            }
                        }
                    }

                    self.consume(Token::RightParen)?;

                    let mut return_type = Type::Identifier("Void".to_string());
                    if self.at().token == Token::Tilde {
                        self.consume(Token::Tilde)?;
                        return_type = self._type()?;
                    }

                    self.consume(Token::ConstAssignment)?;
                    self.consume(Token::Begin)?;

                    let mut body = Vec::new();
                    while self.not_eof() && self.at().token != Token::End {
                        body.push(self.statement()?);
                    }

                    self.consume(Token::End)?;
                    return Ok(Stmt::Function(name, params, return_type, body));
                }

                // Variable declaration handling
                let mut var_type = Type::Identifier("Unknown".to_string());
                if self.at().token == Token::Tilde {
                    self.consume(Token::Tilde)?;
                    var_type = self._type()?;
                }

                let is_const = match self.at().token {
                    Token::MutAssignment => {
                        self.consume(Token::MutAssignment)?;
                        false
                    }
                    Token::ConstAssignment => {
                        self.consume(Token::ConstAssignment)?;
                        true
                    }
                    _ => {
                        return Err(
                            ExpectedMultipleFound::new(
                                &self.file,
                                self.to_owned().at().position.line,
                                self.to_owned().at().position.column,
                                vec![":=", "::"],
                                self.to_owned().at().lexeme.as_str()
                            ).unwrap_err()
                        );
                    }
                };

                let initializer = self.expression()?;
                self.consume(Token::Semicolon)?;
                return Ok(Stmt::Variable(name, var_type, initializer, is_const));
            }
            _ => {
                return Err(
                    ExpectedFound::new(
                        &self.file,
                        self.to_owned().at().position.line,
                        self.to_owned().at().position.column,
                        "identifier",
                        self.to_owned().at().lexeme.as_str()
                    ).unwrap_err()
                );
            }
        }
    }

    pub fn return_statement(&mut self) -> Result<Stmt, CompilerError> {
        self.consume(Token::Return)?;
        let expr = self.expression()?;
        self.consume(Token::Semicolon)?;
        Ok(Stmt::Return(expr))
    }

    pub fn expression(&mut self) -> Result<Expr, CompilerError> {
        self.binary(0)
    }

    pub fn binary(&mut self, precedence: usize) -> Result<Expr, CompilerError> {
        let mut left = self.unary()?;
        while precedence < self.precedence() {
            let op = self.eat();
            let right = self.unary()?;
            left = Expr::Binary(Box::new(left), op.token, Box::new(right));
        }
        Ok(left)
    }

    pub fn precedence(&self) -> usize {
        *PRECEDENCE.get(&self.at().token).unwrap_or(&0)
    }

    pub fn unary(&mut self) -> Result<Expr, CompilerError> {
        match self.at().token {
            | Token::Minus
            | Token::Bang
            | Token::Question
            | Token::Concat
            | Token::Mul
            | Token::At => {
                let op = self.eat().token;
                Ok(Expr::Unary(Box::new(self.unary()?), op))
            }
            _ => self.primary(),
        }
    }

    pub fn primary(&mut self) -> Result<Expr, CompilerError> {
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
                let expr = self.expression()?;
                self.consume(Token::RightParen)?;
                Expr::Enclosed(Box::new(expr))
            }
            Token::LeftCurly => {
                self.advance();
                let mut arr = Vec::new();
                while self.not_eof() && self.at().token != Token::RightCurly {
                    arr.push(self.expression()?);
                    if self.at().token == Token::Comma {
                        self.advance();
                    } else {
                        break;
                    }
                }
                self.consume(Token::RightCurly)?;
                Expr::Array(arr)
            }
            Token::Proc => {
                self.advance();
                let mut params = FxHashMap::default();
                self.consume(Token::FatArrow)?;
                self.consume(Token::LeftParen)?;
                while self.not_eof() && self.at().token != Token::RightParen {
                    let name = self.eat();
                    if let Token::Identifier(_) = name.token {
                        self.advance();
                    } else {
                        return Err(
                            ExpectedFound::new(
                                &self.file,
                                self.to_owned().at().position.line,
                                self.to_owned().at().position.column,
                                "identifier",
                                self.to_owned().at().lexeme.as_str()
                            ).unwrap_err()
                        );
                    }
                    let var_type = self._type()?;
                    if self.at().token == Token::Comma {
                        self.advance();
                    } else {
                        break;
                    }
                    params.insert(name.lexeme, var_type);
                }
                self.consume(Token::RightParen)?;
                self.consume(Token::LeftCurly)?;
                let mut body = vec![];
                while self.not_eof() && self.at().token != Token::RightCurly {
                    body.push(self.statement()?);
                }
                self.consume(Token::RightCurly)?;
                return Ok(Expr::Proc(params, body));
            }
            Token::New => self.new_expr()?,
            _ => {
                return Err(
                    ExpectedFound::new(
                        &self.file,
                        self.at().position.line,
                        self.at().position.column,
                        "expression",
                        format!("{:?}", self.at().token).as_str()
                    ).unwrap_err()
                );
            }
        };

        // Handle recursive expressions
        loop {
            match self.at().token.clone() {
                Token::LeftParen => {
                    self.advance();
                    let mut args = vec![];
                    while self.not_eof() && self.at().token != Token::RightParen {
                        args.push(self.expression()?);
                        if self.at().token == Token::Comma {
                            self.advance();
                        }
                    }
                    self.consume(Token::RightParen)?;
                    expr = Expr::FunctionCall(Box::new(expr), args);
                }
                Token::LeftSquare => {
                    self.advance();
                    let index_expr = self.expression()?;
                    self.consume(Token::RightSquare)?;
                    expr = Expr::Member(Box::new(expr), Box::new(index_expr));
                }
                Token::Dot => {
                    self.advance();
                    let index_expr = self.expression()?;
                    expr = Expr::Member(Box::new(expr), Box::new(index_expr))
                }
                Token::LeftCurly => {
                    let mut struct_name = String::new();
                    match expr {
                        Expr::Identifier(ref name) => {
                            struct_name = name.to_string();
                        }
                        _ => {
                            return Err(
                                ExpectedFound::new(
                                    &self.file,
                                    self.at().position.line,
                                    self.at().position.column,
                                    "Identifier",
                                    format!("{:?}", self.at().token).as_str()
                                ).unwrap_err()
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
                                return Err(
                                    ExpectedFound::new(
                                        &self.file,
                                        self.at().position.line,
                                        self.at().position.column,
                                        "Identifier",
                                        format!("{:?}", self.at().token).as_str()
                                    ).unwrap_err()
                                );
                            }
                        }

                        self.consume(Token::MutAssignment)?;

                        let field_expr = self.expression()?;

                        fields.insert(field_rname, field_expr);
                        if self.at().token == Token::Comma {
                            self.advance();
                        } else {
                            break;
                        }
                    }
                    self.consume(Token::RightCurly)?;

                    return Ok(Expr::StructInstantiation(struct_name, fields));
                }
                _ => {
                    break;
                }
            }
        }

        Ok(expr)
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

    pub fn _type(&mut self) -> Result<Type, CompilerError> {
        match self.at().token {
            Token::Identifier(ref _type_name) => {
                let base_type = self.eat().lexeme;
                // Check if it's a generic type like Gen<T, T, T>
                if self.at().token == Token::LeftAngle {
                    self.consume(Token::LeftAngle)?;
                    let mut generic_types = Vec::new();

                    while self.not_eof() && self.at().token != Token::RightAngle {
                        generic_types.push(self._type()?);
                        if self.at().token == Token::Comma {
                            self.consume(Token::Comma)?;
                        } else {
                            break;
                        }
                    }
                    self.consume(Token::RightAngle)?;
                    return Ok(Type::Template(base_type, generic_types));
                }
                // Check if it's a function type like Int(Int, Int)
                if self.at().token == Token::LeftParen {
                    self.consume(Token::LeftParen)?;
                    let mut param_types = Vec::new();
                    while self.not_eof() && self.at().token != Token::RightParen {
                        param_types.push(self._type()?);
                        if self.at().token == Token::Comma {
                            self.consume(Token::Comma)?;
                        } else {
                            break;
                        }
                    }
                    self.consume(Token::RightParen)?;
                    return Ok(Type::FunctionPointer(base_type, param_types));
                }
                return Ok(Type::Identifier(base_type));
            }
            _ => {
                return Err(
                    ExpectedFound::new(
                        &self.file,
                        self.to_owned().at().position.line,
                        self.to_owned().at().position.column,
                        "type",
                        self.to_owned().at().lexeme.as_str()
                    ).unwrap_err()
                );
            }
        }
    }
}
