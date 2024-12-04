#include "include/main.hpp"

int main(void)
{

  Lexer *lexer = new Lexer();

  lexer->getTokens("ringfiles/parser.ri");
  lexer->tokenize();

  for (auto token : lexer->tokens)
  {
    lexer->printToken(token);
  }

  return 0;
}
