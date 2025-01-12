/// Imports the `Token` type from the `lexer` module in the parent directory.
/// This is an implementation detail and is not part of the public API.
use super::lexer::Token;
/// Imports the `FxHashMap` type from the `rustc_hash` crate.
/// This is an implementation detail and is not part of the public API.
use rustc_hash::FxHashMap;

/// The `Expr` enum represents different types of expressions in the language.
/// It is used throughout the parser and other parts of the compiler.
#[derive(Debug, Clone, PartialEq)]
pub enum Expr {
    Integer(i64),
    Float(f64),
    String(String),
    Member(Box<Expr>, Box<Expr>),
    Array(Vec<Expr>),
    Boolean(bool),
    Unary(Box<Expr>, Token),
    Binary(Box<Expr>, Token, Box<Expr>),
    Identifier(String),
    Nil,
    Enclosed(Box<Expr>),
    StructInstantiation(String, FxHashMap<String, Expr>),
    FunctionCall(Box<Expr>, Vec<Expr>),
    New(FxHashMap<String, Expr>),
}

#[derive(Debug, Clone, PartialEq)]
pub enum Type {
    Identifier(String),
    Template(String, Vec<Type>),
    FunctionPointer(String, Vec<Type>),
}

#[derive(Debug, Clone)]
pub enum DType {
    Struct(FxHashMap<String, Type>),
    Custom(Type)
}



/// The `Stmt` enum represents different types of statements in the language.
/// It is used throughout the parser and other parts of the compiler.
/// Represents different types of statements in the language, including variable declarations, function definitions, code blocks, program entry points, conditional statements, and return statements.
/// These statement types are used throughout the parser and other parts of the compiler.
#[derive(Debug, Clone)]
pub enum Stmt {
    Empty,
    Variable(String, Type, Expr, bool),
    Function(String, FxHashMap<String, Type>, Type, Vec<Stmt>),
    Program(Vec<Stmt>),
    Block{
        block: Vec<Stmt>,
    },
    If(Expr, Box<Stmt>, Box<Stmt>),
    Return(Expr),
    Do(Vec<Stmt>),
    For(String, String, Expr, Vec<Stmt>),
    Times(Expr, Vec<Stmt>),
    Pub(Box<Stmt>),
    Expr(Expr),
    While(Expr, Vec<Stmt>),
    TypeDeclaration(Type, DType)
}
