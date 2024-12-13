#include "include/main.hpp"

int main(void)
{

  Lexer lexer = Lexer{};

  lexer.getTokens("examples/arrays.sy");
  lexer.tokenize();

  Parser p = Parser{"examples/arrays.sy"};
  AST::DebugVisitor debugger;
  auto program = std::make_shared<Program>(std::move(p.produceAST(lexer.tokens)));

  debugger.visitProgramStmt(program);


  return 0;
}