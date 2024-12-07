#include "include/main.hpp"

int main(void)
{

  Lexer lexer = Lexer{};

  lexer.getTokens("ringfiles/parser.ri");
  lexer.tokenize();

  Parser p = Parser{"ringfiles/parser.ri"};
  p.produceAST(lexer.tokens);

  for (auto token : lexer.tokens)
  {
    lexer.printToken(token);
  }

  return 0;
}
