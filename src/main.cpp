#include "include/main.hpp"

int main(void)
{

  Lexer lexer = Lexer{};

  lexer.getTokens("examples/foo_bar_baz.sy");
  lexer.tokenize();

  Parser p = Parser{"examples/foo_bar_baz.sy"};
  AST::DebugVisitor debugger;
  auto program = std::make_shared<Program>(std::move(p.produceAST(lexer.tokens)));

  debugger.visitProgramStmt(program);


  return 0;
}