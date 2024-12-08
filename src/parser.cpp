#include "parser/parser.hpp"

bool Parser::notEof()
{
 return pos < tokens->size() && at().Kind != token_t::Tkn_Eof;
}

void Parser::consume(token_t Kind)
{
 auto current = at();
 advance();
 if (notEof() && current.Kind != Kind)
 {
  ExpectedFound(filename,
                current.Position.line,
                current.Position.col,
                TokenToString(Kind),
                TokenToString(current.Kind));
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

std::shared_ptr<Stmt> Parser::variable()
{
 consume(token_t::Tkn_Let); // Consume the 'let' keyword
 auto name = eat();         // Eat the next token (which should be an identifier)

 if (name.Kind != token_t::Tkn_Identifier)
 {
  ExpectedFound(filename, name.Position.line, name.Position.col, "Identifier", TokenToString(name.Kind));
  return nullptr;
 }

 std::string typedname = "Unknown"; // Default typedname value

 if (at().Kind == token_t::Tkn_Tilde)
 {           // Check for tilde (~)
  advance(); // Consume the tilde
  if (at().Kind != token_t::Tkn_Identifier)
  {
   ExpectedFound(filename, at().Position.line, at().Position.col, "Identifier", TokenToString(at().Kind));
   return nullptr;
  }
  else
  {
   typedname = eat().Value; // Consume the identifier after tilde
  }
 }

 bool constant = false;

 if (at().Kind == token_t::Tkn_Const_Assignment || at().Kind == token_t::Tkn_Mut_Assignment)
 {
  if (at().Kind == token_t::Tkn_Const_Assignment)
  {
   constant = true; // Mark as constant if it's a const assignment
  }
  advance(); // Consume the assignment operator
 }
 else
 {
  ExpectedFound(filename, at().Position.line, at().Position.col, ":: or :=", at().Value);
  return nullptr;
 }

 auto valueExpr = expr(); // Parse the expression

 consume(token_t::Tkn_Semi); // Consume the semicolon

 return std::make_shared<VariableStmt>(name.Value, valueExpr, constant, typedname); // Return the variable statement
}

std::shared_ptr<Stmt> Parser::stmt()
{
 if (at().Kind == token_t::Tkn_Let)
 {
  return variable();
 }
 auto expression = expr(); // Get the expression
 consume(token_t::Tkn_Semi);
 return std::make_shared<ExprStmt>(expression);
};

std::shared_ptr<Expr> Parser::expr()
{
 return binary(0);
}

std::shared_ptr<Expr> Parser::primary()
{
 auto tk = at().Kind;

 switch (tk)
 {
 case token_t::Tkn_Identifier:
 {
  return std::make_shared<IdentifierExpr>(eat().Value);
 }
 case token_t::Tkn_Number:
 {
  return std::make_shared<LiteralExpr>(std::stoi(eat().Value));
 }
 case token_t::Tkn_Float:
 {
  return std::make_shared<LiteralExpr>(std::stof(eat().Value));
 }
 case token_t::Tkn_String:
 {
  return std::make_shared<LiteralExpr>(eat().Value);
 }
 case token_t::Tkn_Lparen:
 {
  consume(token_t::Tkn_Lparen);
  auto inner = expr();
  consume(token_t::Tkn_Rparen);
  return std::make_shared<ParenthesizedExpr>(inner);
 }
 default:
 {
  ExpectedFound(filename, at().Position.line, at().Position.col, "a Number, String or an Identifier", TokenToString(at().Kind));
  return nullptr;
 }
 }
}

std::shared_ptr<Expr> Parser::binary(int p = 0)
{
 auto left = primary();

 while (notEof())
 {
  auto op = at();
  int opPrecedence = getPrecedence(op.Kind);
  if (opPrecedence < p)
   break;

  consume(op.Kind);
  auto right = binary(opPrecedence + 1);

  left = std::make_shared<BinaryExpr>(left, op, right);
 }
 return left;
}

Program Parser::produceAST(std::vector<token_visual_t> tks)
{
 Program p;
 pos = 0;
 tokens = std::make_shared<std::vector<token_visual_t>>(std::move(tks));

 while (notEof())
 {
  p.addStmt(std::move(stmt()));
 }

 return p;
}