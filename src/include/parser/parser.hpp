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
using ParenthesizedExpr = AST::EnclosedExpr;
using CallExpr = AST::CallExpr;
using LiteralExpr = AST::LiteralExpr;
using IdentifierExpr = AST::IdentifierExpr;

using Program = AST::ProgramStmt;
using BlockStmt = AST::BlockStmt;
using VariableStmt = AST::VariableStmt;
using FunctionStmt = AST::FunctionStmt;
using ExprStmt = AST::ExprStmt;
using ReturnStmt = AST::ReturnStmt;

inline std::unordered_map<token_t, int> opPr = {
    {token_t::Tkn_Div, 20},
    {token_t::Tkn_Mul, 20},
    {token_t::Tkn_Mod, 20},
    {token_t::Tkn_Plus, 10},
    {token_t::Tkn_Minus, 10}};

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

    std::shared_ptr<Stmt> stmt();
    std::shared_ptr<Expr> expr();

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