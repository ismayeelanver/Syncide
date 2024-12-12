#pragma once
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <fmt/core.h>

#include "error.hpp"
#include <unordered_map>

using token_value_t = char;
using token_values_t = std::vector<token_value_t>;



typedef enum class token_t
{
  // Symbols
  Tkn_Const_Assignment,
  Tkn_Mut_Assignment,
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
  Tkn_Comma,
  Tkn_Colon,
  Tkn_At, // @

  // Identifiers and numbers
  Tkn_Identifier,
  Tkn_Number,
  Tkn_Float,
  Tkn_String,

  // Keywords
  Tkn_If,
  Tkn_Let,
  Tkn_Begin,
  Tkn_End,
  Tkn_Ret,

  // EOF
  Tkn_Eof
} token_t;

inline std::unordered_map<token_t, std::string> tokenToStringMap = {
    {token_t::Tkn_Const_Assignment, "Tkn_Const_Assignment"},
    {token_t::Tkn_Mut_Assignment, "Tkn_Mut_Assignment"},
    {token_t::Tkn_Tilde, "Tkn_Tilde"},
    {token_t::Tkn_Rparen, "Tkn_Rparen"},
    {token_t::Tkn_Lparen, "Tkn_Lparen"},
    {token_t::Tkn_Semi, "Tkn_Semi"},
    {token_t::Tkn_Plus, "Tkn_Plus"},
    {token_t::Tkn_Minus, "Tkn_Minus"},
    {token_t::Tkn_Div, "Tkn_Div"},
    {token_t::Tkn_Mul, "Tkn_Mul"},
    {token_t::Tkn_Mod, "Tkn_Mod"},
    {token_t::Tkn_Bang, "Tkn_Bang"},
    {token_t::Tkn_Float, "Tkn_Float"},
    {token_t::Tkn_Comma, "Tkn_Comma"},
    {token_t::Tkn_Colon, "Tkn_Colon"},
    {token_t::Tkn_At, "Tkn_At"},
    {token_t::Tkn_Identifier, "Tkn_Identifier"},
    {token_t::Tkn_Number, "Tkn_Number"},
    {token_t::Tkn_String, "Tkn_String"},
    {token_t::Tkn_If, "Tkn_If"},
    {token_t::Tkn_Let, "Tkn_Let"},
    {token_t::Tkn_Begin, "Tkn_Begin"},
    {token_t::Tkn_End, "Tkn_End"},
    {token_t::Tkn_Ret, "Tkn_Ret"},
    {token_t::Tkn_Eof, "Tkn_Eof"}
};


inline std::string TokenToString(token_t tk) {
  return tokenToStringMap.at(tk);
}
typedef struct token_position_t
{
  int line = 1;
  int col = 1;
} token_position_t;

using token_visual_value_t = std::string;

class token_visual_t
{
public:
  token_t Kind;
  std::string Value;
  token_position_t Position;
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

  token_visual_t makeToken(std::string, token_t, token_position_t);
};
