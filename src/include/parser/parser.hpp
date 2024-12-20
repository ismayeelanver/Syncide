#pragma once
#include "ast.hpp"
#include "../lexer.hpp"
#include <memory>
#include <unordered_map>
#include <limits>

using Expr = AST::Expr;
using Stmt = AST::Stmt;

// Exprs and Stmts
using BinaryExpr = AST::BinaryExpr;
using UnaryExpr = AST::UnaryExpr;
using ArrayExpr = AST::ArrayExpr;
using BooleanExpr = AST::BooleanExpr;
using ParenthesizedExpr = AST::EnclosedExpr;
using CallExpr = AST::CallExpr;
using NilExpr = AST::ConcreteNilExpr;
using LiteralExpr = AST::LiteralExpr;
using IdentifierExpr = AST::IdentifierExpr;

using Program = AST::ProgramStmt;
using BlockStmt = AST::BlockStmt;
using VariableStmt = AST::VariableStmt;
using IfStmt = AST::IfStmt;
using FunctionStmt = AST::FunctionStmt;
using ExprStmt = AST::ExprStmt;
using ReturnStmt = AST::ReturnStmt;

inline std::unordered_map<token_t, int> opPr = {
    // Unary operators (highest precedence)
    {token_t::Tkn_Bang, 90},  // !
    {token_t::Tkn_Tilde, 90}, // ~

    // Multiplicative and Modulo operators
    {token_t::Tkn_Mul, 80}, // *
    {token_t::Tkn_Div, 80}, // /
    {token_t::Tkn_Mod, 80}, // %

    // Additive operators
    {token_t::Tkn_Plus, 70},  // +
    {token_t::Tkn_Minus, 70}, // -

    // Relational and equality operators
    {token_t::Tkn_Langle, 60},        // <
    {token_t::Tkn_Rangle, 60},        // >
    {token_t::Tkn_Lesser_Equal, 60},  // <=
    {token_t::Tkn_Greater_Equal, 60}, // >=
    {token_t::Tkn_Equal, 50},         // ==
    {token_t::Tkn_Bang_Equal, 50},    // !=

    // Logical AND/OR operators
    {token_t::Tkn_And, 40}, // &&
    {token_t::Tkn_Or, 30},  // ||

    // Assignment operators (lower precedence)
    {token_t::Tkn_Mut_Assignment, 20}};

inline int getPrecedence(token_t type)
{
    auto attempt = opPr.find(type);

    if (attempt != opPr.end())
    {
        return attempt->second;
    }
    return std::numeric_limits<int>::min();
};

class Parser
{
private:
    int pos;
    std::string filename;
    std::shared_ptr<std::vector<token_visual_t>> tokens;
    bool notEof();

    std::shared_ptr<Stmt> variableOrfunction();
    std::shared_ptr<Stmt> stmtsOutside();
    std::shared_ptr<Stmt> returnStmt();
    std::shared_ptr<Stmt> ifStmt();

    std::shared_ptr<Expr> makeAfterIdentifier();

    std::shared_ptr<Stmt> stmt();
    std::shared_ptr<Expr> expr();

    std::shared_ptr<AST::Type> parseType();

    void consume(token_t);
    void advance();
    token_visual_t eat();
    token_visual_t at();

    std::shared_ptr<Expr> primary();
    std::shared_ptr<Expr> binary(int p);

public:
    Parser(std::string s) : filename(s)
    {
    }
    Program produceAST(std::vector<token_visual_t> tks);
};