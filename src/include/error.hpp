#pragma once

#include <exception>
#include <iostream>
#include <optional>
#include <string>
#include <fmt/core.h>

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
      fmt::print(stderr, "Error: {} At {}:{}:{}\n", what(), file, line.value(), column.value());
      fmt::print(stderr, "\t{}\n", lineOfCode);
      fmt::print(stderr, "\t{}^\n", std::string(column.value(), '~'));
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
      fmt::print(stderr, "Error: {} At {}:{}:{}\n", what(), file, line.value(), column.value());
      fmt::print(stderr, "\t{}\n", lineOfCode);
      fmt::print(stderr, "\t{}^\n", std::string(column.value(), '~'));
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
      fmt::print(stderr, "Error: {} At {}:{}:{}\n", what(), file, line.value(), column.value());
      fmt::print(stderr, "\t{}\n", lineOfCode);
      fmt::print(stderr, "\t{}^\n", std::string(column.value(), '~'));
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
      fmt::print(stderr, "Error: {} At {}:{}:{}\n", what(), file, line.value(), column.value());
      fmt::print(stderr, "\t{}\n", lineOfCode);
      fmt::print(stderr, "\t{}^\n", std::string(column.value(), '~'));
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
      fmt::print(stderr, "Error: {} At {}:{}:{}\n", what(), file, line.value(), column.value());
      fmt::print(stderr, "\t{}\n", lineOfCode);
      fmt::print(stderr, "\t{}^\n", std::string(column.value(), '~'));
      fmt::print(stderr, "\t Expected: '{}' but Found '{}'\n", exp, fnd, line.value());
      exit(EXIT_FAILURE);
   }
   virtual const char *what()
   {
      return "<Wrong Token Found>";
   }
};