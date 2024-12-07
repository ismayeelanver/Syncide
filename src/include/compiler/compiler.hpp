#include <lexer.hpp>
#include <parser/parser.hpp>

class RingCompiler
{
 Lexer *lexer;
 Parser *parser;
 void emit()
 {
  lexer->getTokens("ringfiles/parser.ri");
  fmt::print("{}\n", lexer->tokens);
 }
};