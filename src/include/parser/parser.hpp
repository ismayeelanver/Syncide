#pragma once
#include <parser/ast.hpp>
#include <lexer.hpp>
#include <memory>
#include <unordered_map>

using Expr = AST::Expr;
using Stmt = AST::Stmt;

// Exprs and Stmts
using BinaryExpr = AST::BinaryExpr;
using ParenthesizedExpr = AST::EnclosedExpr;
using CallExpr = AST::CallExpr;
using LiteralExpr = AST::LiteralExpr;
using IdentifierExpr = AST::IdentifierExpr;

using Program = AST::ProgramStmt;
using BlockStmt = AST::BlockStmt;
using VariableStmt = AST::VariableStmt;
using ExprStmt = AST::ExprStmt;

enum Bp
{
    lowest = 0,
    multiplicative = 10,
    additive = 20
};

class Parser
{
private:
    int pos;
    std::string filename;
    std::shared_ptr<std::vector<token_visual_t>> tokens;
    bool notEof();

    Stmt stmt();
    Expr expr();

    void consume(token_t);
    void advance();
    token_visual_t eat();
    token_visual_t at();

    Expr primary();

public:
    Parser(std::string s) : filename(s)
    {
    }
    Program produceAST(std::vector<token_visual_t> tks);
};