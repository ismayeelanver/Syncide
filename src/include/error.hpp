#pragma once

#include <exception>
#include <iostream>
#include <fstream>
#include <optional>
#include <string>
#include <fmt/core.h>

#include "color.hpp"

inline std::string getLine(const std::string &filename, int line_number)
{
   std::ifstream file(filename);
   std::string line;

   for (int i = 1; i < line_number; i++)
   {
      if (!std::getline(file, line))
         return "";
   }

   std::getline(file, line);

   return line;
}

class Error : public std::exception
{
public:
   std::optional<int> line;
   std::optional<int> column;
   virtual ~Error() = default;
   virtual const char *what()
   {
      return "<Error>";
   };
};

class InvalidFloat : public Error
{
public:
   InvalidFloat(std::string file, int l, int c)
   {
      line = l;
      column = c;
      std::string lineOfCode = getLine(file, static_cast<std::size_t>(line.value()));
      fmt::print(stderr, blue("✦ [Error ✘] \n") + gold("└── {}\n"), red(what()));
      fmt::print(stderr, "{} {}\n {}\n", blue("• [Line of Error]"), red("[" + file + ":" + std::to_string(l) + ":" + std::to_string(c) + "]"), gold("↓"));
      fmt::print(stderr, gold("───────≼≽────────\n"));
      fmt::print(stderr, "\t{}\n", white(lineOfCode));

      std::ostringstream pointer;
      for (int i = 1; i < column.value(); ++i)
      {
         pointer << '~';
      }

      fmt::print(stderr, "\t{}{}\n", blue(pointer.str()), green(std::string("↑")));
      fmt::print(stderr, gold("───────≼≽────────\n"));
      exit(EXIT_FAILURE);
   };

   virtual const char *what()
   {
      return "<Invalid Float>";
   };
};
class InvalidNumber : public Error
{
public:
   InvalidNumber(std::string file, int l, int c)
   {
      line = l;
      column = c;
      std::string lineOfCode = getLine(file, static_cast<std::size_t>(line.value()));
      fmt::print(stderr, blue("✦ [Error ✘] \n") + gold("└── {}\n"), red(what()));
      fmt::print(stderr, "{} {}\n {}\n", blue("• [Line of Error]"), red("[" + file + ":" + std::to_string(l) + ":" + std::to_string(c) + "]"), gold("↓"));
      fmt::print(stderr, gold("───────≼≽────────\n"));
      fmt::print(stderr, "\t{}\n", white(lineOfCode));

      std::ostringstream pointer;
      for (int i = 1; i < column.value(); ++i)
      {
         pointer << '~';
      }

      fmt::print(stderr, "\t{}{}\n", blue(pointer.str()), green(std::string("↑")));
      fmt::print(stderr, gold("───────≼≽────────\n"));
      exit(EXIT_FAILURE);
   }
   virtual const char *what()
   {
      return "<Invalid Number>";
   };
};

class InvalidString : public Error
{
public:
   InvalidString(std::string file, int l, int c)
   {
      line = l;
      column = c;
      std::string lineOfCode = getLine(file, static_cast<std::size_t>(line.value()));
      fmt::print(stderr, blue("✦ [Error ✘] \n") + gold("└── {}\n"), red(what()));
      fmt::print(stderr, "{} {}\n {}\n", blue("• [Line of Error]"), red("[" + file + ":" + std::to_string(l) + ":" + std::to_string(c) + "]"), gold("↓"));
      fmt::print(stderr, gold("───────≼≽────────\n"));
      fmt::print(stderr, "\t{}\n", white(lineOfCode));

      std::ostringstream pointer;
      for (int i = 1; i < column.value(); ++i)
      {
         pointer << '~';
      }

      fmt::print(stderr, "\t{}{}\n", blue(pointer.str()), green(std::string("↑")));
      fmt::print(stderr, gold("───────≼≽────────\n"));
      exit(EXIT_FAILURE);
   }
   virtual const char *what()
   {
      return "<Invalid String>";
   }
};

class InvalidToken : public Error
{
public:
   InvalidToken(std::string file, int l, int c)
   {
      line = l;
      column = c;
      std::string lineOfCode = getLine(file, static_cast<std::size_t>(line.value()));
      fmt::print(stderr, blue("✦ [Error ✘] \n") + gold("└── {}\n"), red(what()));
      fmt::print(stderr, "{} {}\n {}\n", blue("• [Line of Error]"), red("[" + file + ":" + std::to_string(l) + ":" + std::to_string(c) + "]"), gold("↓"));
      fmt::print(stderr, gold("───────≼≽────────\n"));
      fmt::print(stderr, "\t{}\n", white(lineOfCode));

      std::ostringstream pointer;
      for (int i = 1; i < column.value(); ++i)
      {
         pointer << '~';
      }

      fmt::print(stderr, "\t{}{}\n", blue(pointer.str()), green(std::string("↑")));
      fmt::print(stderr, gold("───────≼≽────────\n"));
      exit(EXIT_FAILURE);
   }
   virtual const char *what()
   {
      return "<Invalid Token>";
   }
};

class ExpectedFound : public Error
{
public:
   ExpectedFound(std::string file, int l, int c, std::string exp, std::string fnd)
   {
      line = l;
      column = c;
      std::string lineOfCode = getLine(file, static_cast<std::size_t>(line.value()));
      fmt::print(stderr, blue("✦ [Error ✘] \n") + gold("└── {}\n"), red(what()));
      fmt::print(stderr, "{} {}\n {}\n", blue("• [Line of Error]"), red("[" + file + ":" + std::to_string(l) + ":" + std::to_string(c) + "]"), gold("↓"));
      fmt::print(stderr, gold("───────≼≽────────\n"));
      fmt::print(stderr, "\t{}\n", white(lineOfCode));

      std::ostringstream pointer;
      for (int i = 1; i < column.value(); ++i)
      {
         pointer << '~';
      }

      fmt::print(stderr, "\t{}{}\n", blue(pointer.str()), green(std::string("↑")));
      fmt::print(stderr, gold("───────≼≽────────\n"));
      std::ostringstream whut;
      whut << "• [Expected:" << " " << exp << " " << "But Found:" << " " << fnd << "]" << std::endl;
      fmt::print(stderr, blue(whut.str()));
      exit(EXIT_FAILURE);
   }
   virtual const char *what()
   {
      return "<Wrong Token Found>";
   }
};
