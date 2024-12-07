#include "parser/parser.hpp"

bool Parser::notEof()
{
 return at().Kind != token_t::Tkn_Eof;
}

void Parser::consume(token_t Kind)
{
 advance();
 auto current = at();
 if (current.Kind != Kind)
 {
  ExpectedFound(filename, current.Position.line)
 }
}

void Parser::advance()
{
 pos++;
}

token_visual_t Parser::at()
{
 return tokens->at(pos);
}

token_visual_t Parser::eat()
{
 auto currentToken = at();
 advance();
 return currentToken;
}

Stmt Parser::stmt()
{
 return ExprStmt{expr()};
};

Expr Parser::expr()
{
 return primary();
}

Expr Parser::primary()
{
 auto tk = this->at().Kind;

 switch (tk)
 {
 case token_t::Tkn_Identifier:
 {
  return IdentifierExpr{this->eat().Value};
 }
 case token_t::Tkn_Number:
 {
  return LiteralExpr{std::stoi(this->eat().Value)};
 }
 case token_t::Tkn_String:
 {
  return LiteralExpr{this->eat().Value};
 }
 case token_t::Tkn_Lparen:
 {
 }
 }
}

Program Parser::produceAST(std::vector<token_visual_t> tks)
{
 Program p;
 this->tokens = std::make_shared<std::vector<token_visual_t>>(std::move(tks));

 while (notEof())
 {
  p.addStmt(stmt());
 }

 return p;
}
