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

token_visual_t Lexer::makeToken(std::string v, token_t t,
                                token_position_t pos)
{
  token_visual_t tkn = {};
  tkn.Position = pos;
  tkn.Kind = t;
  tkn.Value = v;
  return tkn;
}

void Lexer::tokenize()
{
  for (size_t i = 0; i < values.size(); ++i)
  {
    char ch = values.at(i);
    switch (ch)
    {
    case ',':
    {
      tokens.push_back(makeToken(std::string(1, ch), token_t::Tkn_Comma, pos));
      pos.col++;
      break;
    }
    case '~':
    {
      tokens.push_back(makeToken(std::string(1, ch), token_t::Tkn_Tilde, pos));
      pos.col++;
      break;
    }
    case '!':
    {
      if (i + 1 < values.size() && values[i + 1] == '=')
      {
        tokens.push_back(makeToken("!=", token_t::Tkn_Bang_Equal, pos));
        i++;          // Skip the '='
        pos.col += 2; // Advance column for both characters
      }
      else
      {
        tokens.push_back(makeToken("!", token_t::Tkn_Bang, pos));
        pos.col++;
      }
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

    case '?':
    {
      tokens.push_back(makeToken(std::string(1, ch), token_t::Tkn_Question, pos));
      pos.col++;
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

    case '=':
    {
      if (i + 1 < values.size() && values[i + 1] == '=')
      {
        tokens.push_back(makeToken("==", token_t::Tkn_Equal, pos));
        i++;          // Skip the '='
        pos.col += 2; // Advance column for both characters
      }
      else
      {
        InvalidToken(Filename, pos.line, pos.col);
      }
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

      if (expected_next < values.size() && values[expected_next] == ':')
      {
        // Handle `::` token
        tokens.push_back(makeToken("::", token_t::Tkn_Const_Assignment, pos));
        i++;          // Skip the second `:`
        pos.col += 2; // Advance for both `:` characters
      }
      else if (expected_next < values.size() && values[expected_next] == '=')
      {
        // Handle `:=` token
        tokens.push_back(makeToken(":=", token_t::Tkn_Mut_Assignment, pos));
        i++;          // Skip the `=`
        pos.col += 2; // Advance for both `:` and `=`
      }
      else if (expected_next >= values.size())
      {
        // Handle EOF after `:`
        ExpectedFound(Filename, pos.line, pos.col, ": or :: or :=", "EOF");
      }
      else
      {
        tokens.push_back(makeToken(std::string(1, ch), token_t::Tkn_Colon, pos));
        pos.col++; // Increment column for single `:`
      }

      break;
    }

    case '&':
    {
      if (i + 1 < values.size() && values[i + 1] == '&')
      {
        tokens.push_back(makeToken("&&", token_t::Tkn_And, pos));
        pos.col += 2;
        i++;
      }
      else
      {
        tokens.push_back(makeToken("&", token_t::Tkn_Concat, pos));
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
    case '}':
    {
      tokens.push_back(makeToken(std::string(1, ch), token_t::Tkn_RCurly, pos));
      pos.col++;
      break;
    }
    case '{':
    {
      tokens.push_back(makeToken(std::string(1, ch), token_t::Tkn_LCurly, pos));
      pos.col++;
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
    case '<':
    {
      if (i + 1 < values.size() && values[i + 1] == '=')
      {
        tokens.push_back(makeToken("<=", token_t::Tkn_Lesser_Equal, pos));
        i += 1;
        pos.col++;
      }
      else
      {
        tokens.push_back(makeToken(std::string(1, ch), token_t::Tkn_Langle, pos));
        pos.col++;
      }
      break;
    }
    case '>':
    {
      if (i + 1 < values.size() && values[i + 1] == '=')
      {
        tokens.push_back(makeToken(">=", token_t::Tkn_Greater_Equal, pos));
        i += 1;
        pos.col++;
      }
      else
      {
        tokens.push_back(makeToken(std::string(1, ch), token_t::Tkn_Rangle, pos));
        pos.col++;
      }
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
        bool decimalPointSeen = false;

        for (++i; i < values.size() && (std::isdigit(values[i]) || values[i] == '.' || values[i] == '_'); ++i)
        {
          if (values[i] == '_') // Skip underscores
          {
            continue;
          }

          if (values[i] == '.')
          {
            if (decimalPointSeen) // Only allow one decimal point
            {
              pos.col += (i - start);
              InvalidFloat(Filename, pos.line, pos.col);
              break;
            }

            // Ensure that there are digits after the decimal point
            if (i + 1 >= values.size() || !std::isdigit(values[i + 1]))
            {
              pos.col += (i - start);
              InvalidFloat(Filename, pos.line, pos.col);
              break;
            }

            decimalPointSeen = true;
          }
        }

        std::string number = std::string(values.begin() + start, values.begin() + i);
        if (decimalPointSeen)
        {
          tokens.push_back(makeToken(number, token_t::Tkn_Float, pos));
        }
        else
        {
          tokens.push_back(makeToken(number, token_t::Tkn_Number, pos));
        }

        pos.col += (i - start); // Adjust column position after the token
        --i;                    // Decrement i as it's incremented in the for loop
      }
      else if (std::isalpha(ch) || ch == '_')
      {
        size_t start = i;
        for (++i; i < values.size() && (std::isalnum(values[i]) || values[i] == '_'); ++i)
        {
        }
        std::string identifier = std::string(values.begin() + start, values.begin() + i);
        if (identifier == "if")
        {
          tokens.push_back(makeToken(identifier, token_t::Tkn_If, pos));
        }
        else if (identifier == "let")
        {
          tokens.push_back(makeToken(identifier, token_t::Tkn_Let, pos));
        }
        else if (identifier == "begin")
        {
          tokens.push_back(makeToken(identifier, token_t::Tkn_Begin, pos));
        }
        else if (identifier == "end")
        {
          tokens.push_back(makeToken(identifier, token_t::Tkn_End, pos));
        }
        else if (identifier == "return")
        {
          tokens.push_back(makeToken(identifier, token_t::Tkn_Ret, pos));
        }
        else if (identifier == "true")
        {
          tokens.push_back(makeToken(identifier, token_t::Tkn_True, pos));
        }
        else if (identifier == "false")
        {
          tokens.push_back(makeToken(identifier, token_t::Tkn_False, pos));
        }
        else if (identifier == "nil")
        {
          tokens.push_back(makeToken(identifier, token_t::Tkn_Nil, pos));
        }
        else if (identifier == "then")
        {
          tokens.push_back(makeToken(identifier, token_t::Tkn_Then, pos));
        }
        else if (identifier == "else")
        {
          tokens.push_back(makeToken(identifier, token_t::Tkn_Else, pos));
        }
        else if (identifier == "elif")
        {
          tokens.push_back(makeToken(identifier, token_t::Tkn_Elif, pos));
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
  std::cout << "Line: " << token.Position.line << ", Column: " << token.Position.col
            << " - ";
  std::string str = tokenToStringMap.at(token.Kind);
  std::cout << " - Value: " << token.Value;
  std::cout << " - Type: " << TokenToString(token.Kind);
  std::cout << std::endl;
}