#include "include/lexer.hpp"


void Lexer::getTokens(std::string filename)
{
  this->Filename = filename;
  std::ifstream file(filename);
  if (!file)
  {
    fmt::print(stderr, "Cannot open file!");
    exit(EXIT_FAILURE);
  }
  std::ostringstream buffer;
  buffer << file.rdbuf();
  std::string fileContent = buffer.str();
  std::vector<char> vec(fileContent.begin(), fileContent.end());
  this->values = vec;
  file.close();
}

token_visual_t Lexer::makeToken(token_visual_value_t v, token_t t,
                                token_position_t pos)
{
  token_visual_t tkn = {};
  tkn.pos = pos;
  tkn.token_type = t;
  tkn.value = v;
  return tkn;
}

void Lexer::tokenize()
{
  for (size_t i = 0; i < values.size(); ++i)
  {
    char ch = values.at(i);
    switch (ch)
    {
    case '~':
    {
      tokens.push_back(makeToken(std::string(1, ch), token_t::Tkn_Tilde, pos));
      pos.col++;
      break;
    }
    case '!':
    {
      tokens.push_back(makeToken(std::string(1, ch), token_t::Tkn_Bang, pos));
      pos.col++;
      break;
    }
    case '@':
    {
      tokens.push_back(makeToken(std::string(1, ch), token_t::Tkn_At, pos));
      pos.col++;
      break;
    }
    case '+':
    {
      tokens.push_back(makeToken(std::string(1, ch), token_t::Tkn_Plus, pos));
      pos.col++;
      break;
    }
    case '-':
    {
      if (i < values.size() && values[i + 1] == '-')
      {
        for (++i; i < values.size() && values[i] != '\n'; i++)
        {
          pos.col++;
        }
        pos.line++;
        pos.col = 1;
      }
      else
      {
        tokens.push_back(makeToken(std::string(1, ch), token_t::Tkn_Minus, pos));
        pos.col++;
      }
      break;
    }

    case '*':
    {
      tokens.push_back(makeToken(std::string(1, ch), token_t::Tkn_Mul, pos));
      pos.col++;
      break;
    }

    case '/':
    {
      tokens.push_back(makeToken(std::string(1, ch), token_t::Tkn_Div, pos));
      pos.col++;
      break;
    }

    case '%':
    {
      tokens.push_back(makeToken(std::string(1, ch), token_t::Tkn_Mod, pos));
      pos.col++;
      break;
    }
    case ';':
    {
      tokens.push_back(makeToken(std::string(1, ch), token_t::Tkn_Semi, pos));
      pos.col++;
      break;
    }
    case ':':
    {
      size_t expected_next = i + 1;
      if (i > 0 && values[i - 1] == ':')
      {
        tokens.push_back(makeToken("::", token_t::Tkn_Assignment, pos));
        pos.col++;
      }
      else if (i + 1 < values.size() && values[expected_next] != ':')
      {
        ExpectedFound(Filename, pos.line, pos.col, ":", std::string(1, values[expected_next]));
      }
      else if (i + 1 >= values.size())
      {
        ExpectedFound(Filename, pos.line, pos.col, ":", "EOF");
      }
      else
      {
        pos.col++;
      }
      break;
    }
    case '"':
    {
      size_t start = i++;
      std::string strValue;
      bool escape = false;

      while (i < values.size())
      {
        char current = values[i];

        if (escape)
        {

          switch (current)
          {
          case 'n':
            strValue += '\n';
            break;
          case 't':
            strValue += '\t';
            break;
          case '"':
            strValue += '"';
            break;
          case '\\':
            strValue += '\\';
            break;
          default:
            pos.col += (i - start + 1);
            InvalidString(Filename, pos.line, pos.col);
          }
          escape = false;
        }
        else if (current == '\\')
        {
          escape = true;
        }
        else if (current == '"')
        {

          tokens.push_back(makeToken(strValue, token_t::Tkn_String, pos));
          pos.col += (i - start + 1);
          break;
        }
        else
        {
          strValue += current;
        }
        i++;
      }

      if (i == values.size())
      {
        pos.col += (i - start + 1);
        InvalidString(Filename, pos.line, pos.col);
      }
      break;
    }
    case ')':
    {
      tokens.push_back(makeToken(std::string(1, ch), token_t::Tkn_Rparen, pos));
      pos.col++;
      break;
    }
    case '(':
    {
      tokens.push_back(makeToken(std::string(1, ch), token_t::Tkn_Lparen, pos));
      pos.col++;
      break;
    }
    case '\n':
    {
      pos.line++;
      pos.col = 1;
      break;
    }
    case ' ': // Skip whitespace
    case '\t':
    {
      pos.col++;
      break;
    }
    default:
    {
      if (std::isdigit(ch))
      {
        bool isFloat = false;
        size_t start = i;
        for (++i; i < values.size() && (std::isdigit(values[i]) || values[i] == '.' || values[i] == '_'); ++i)
        {
          if (values[i] == '_')
          {
            continue;
          }
          if (values[i] == '.')
          {
            if (isFloat)
            {
              pos.col += (i - start);
              InvalidFloat(Filename, pos.line, pos.col);
            }
            if (i < values.size() && !std::isdigit(values[i + 1]))
            {
              pos.col += (i - start);
              InvalidFloat(Filename, pos.line, pos.col);
            }
          }
          isFloat = true;
        }
        std::string number =
            std::string(values.begin() + start, values.begin() + i);
        tokens.push_back(makeToken(number, token_t::Tkn_Number, pos));
        pos.col += (i - start);
        --i;
      }
      else if (std::isalpha(ch) || ch == '_')
      {
        size_t start = i;
        for (++i;
             i < values.size() && (std::isalnum(values[i]) || values[i] == '_');
             ++i)
        {
        }
        std::string identifier =
            std::string(values.begin() + start, values.begin() + i);
        if (identifier == "if")
        {
          tokens.push_back(makeToken(identifier, token_t::Tkn_If, pos));
        }
        else if (identifier == "let")
        {
          tokens.push_back(makeToken(identifier, token_t::Tkn_Let, pos));
        }
        else if (identifier == "const")
        {
          tokens.push_back(makeToken(identifier, token_t::Tkn_Const, pos));
        }
        else if (identifier == "begin")
        {
          tokens.push_back(makeToken(identifier, token_t::Tkn_Const, pos));
        }
        else if (identifier == "end")
        {
          tokens.push_back(makeToken(identifier, token_t::Tkn_Const, pos));
        }
        else
        {
          tokens.push_back(makeToken(identifier, token_t::Tkn_Identifier, pos));
        }
        pos.col += (i - start);
        --i;
      }
      else
      {
        InvalidToken(Filename, pos.line, pos.col);
      }
    }
    }
  }
  tokens.push_back(makeToken("\0", token_t::Tkn_Eof, {.line = 1, .col = 1}));
}

void Lexer::printToken(const token_visual_t &token)
{
  std::cout << "Line: " << token.pos.line << ", Column: " << token.pos.col
            << " - ";

  switch (token.token_type)
  {
  case token_t::Tkn_Tilde:
    std::cout << "Tkn_Tilde";
    break;
  case token_t::Tkn_Bang:
    std::cout << "Tkn_Bang";
    break;
  case token_t::Tkn_At:
    std::cout << "Tkn_At";
    break;
  case token_t::Tkn_Plus:
    std::cout << "Tkn_Plus";
    break;
  case token_t::Tkn_Minus:
    std::cout << "Tkn_Minus";
    break;
  case token_t::Tkn_Mul:
    std::cout << "Tkn_Mul";
    break;
  case token_t::Tkn_Div:
    std::cout << "Tkn_Div";
    break;
  case token_t::Tkn_Mod:
    std::cout << "Tkn_Mod";
    break;
  case token_t::Tkn_Semi:
    std::cout << "Tkn_Semi";
    break;
  case token_t::Tkn_Rparen:
    std::cout << "Tkn_Rparen";
    break;
  case token_t::Tkn_Lparen:
    std::cout << "Tkn_Lparen";
    break;
  case token_t::Tkn_Number:
    std::cout << "Tkn_Number";
    break;
  case token_t::Tkn_Identifier:
    std::cout << "Tkn_Id";
    break;
  case token_t::Tkn_Let:
    std::cout << "Tkn_Id";
    break;
  case token_t::Tkn_Const:
    std::cout << "Tkn_Id";
    break;
  case token_t::Tkn_If:
    std::cout << "Tkn_Id";
    break;
  case token_t::Tkn_Begin:
    std::cout << "Tkn_Begin";
    break;
  case token_t::Tkn_End:
    std::cout << "Tkn_End";
    break;
  case token_t::Tkn_Eof:
    std::cout << "End of File";
    break;
  default:
    std::cout << "Unknown Token";
    break;
  }
  std::cout << " - Value: " << token.value;
  std::cout << std::endl;
}