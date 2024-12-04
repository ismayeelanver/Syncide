#pragma once
#include <parser/ast.hpp>
#include <lexer.hpp>
#include <memory>

using ParserTokens = std::vector<token_visual_t>;

// Exprs and Stmts
using BinaryExpr = AST::BinaryExpr;
using ParenthesizedExpr = AST::EnclosedExpr;
using CallExpr = AST::CallExpr;
using LiteralExpr = AST::LiteralExpr;
using Identifier = AST::IdentifierExpr;

using Program = AST::ProgramStmt;
using BlockStmt = AST::BlockStmt;
using VariableStmt = AST::VariableStmt;
using ExprStmt = AST::ExprStmt;

class Parser
{
 std::shared_ptr<ParserTokens> tkns;
 std::shared_ptr<token_visual_t> current;
 int e;
 Program AST;

 Parser(ParserTokens tkens) : tkns(), current()
 {
 }

 void advance();
 void parse();
};