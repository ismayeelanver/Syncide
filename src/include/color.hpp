#include <string>

inline std::string red(const std::string &text)
{
 return "\033[31m" + text + "\033[0m";
}

inline std::string blue(const std::string &text)
{
 return "\033[36m" + text + "\033[0m";
}

inline std::string gold(const std::string &text)
{
 return "\033[38;2;255;215;0m" + text + "\033[0m";
}

inline std::string green(const std::string &text)
{
 return "\033[38;2;100;240;200m" + text + "\033[0m";
}

inline std::string white(const std::string &text)
{
 return "\033[38;2;0;0;0m" + text + "\033[0m";
}