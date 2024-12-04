#pragma once
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <fmt/core.h>

#include <error.hpp>

using token_value_t = char;
using token_values_t = std::vector<token_value_t>;

typedef enum class token_t
{
  // Symbols
  Tkn_Assignment,
  Tkn_Tilde,
  Tkn_Rparen,
  Tkn_Lparen,
  Tkn_Semi,
  Tkn_Plus,
  Tkn_Minus,
  Tkn_Div,
  Tkn_Mul,
  Tkn_Mod,
  Tkn_Bang,
  Tkn_At, // @

  // Identifiers and numbers
  Tkn_Identifier,
  Tkn_Number,
  Tkn_String,

  // Keywords
  Tkn_If,
  Tkn_Let,
  Tkn_Const,
  Tkn_Begin,
  Tkn_End,

  // EOF
  Tkn_Eof
} token_t;

typedef struct token_position_t
{
  int line = 1;
  int col = 1;
} token_position_t;

using token_visual_value_t = std::string;

class token_visual_t
{
public:
  token_t token_type;
  token_visual_value_t value;
  token_position_t pos;
};

class Lexer
{
public:
  std::vector<token_visual_t> tokens;
  std::string Filename;
  token_values_t values;
  token_position_t pos;

  void getTokens(std::string);
  void printToken(const token_visual_t &token);

  void tokenize();

  token_visual_t makeToken(token_visual_value_t, token_t, token_position_t);
};
