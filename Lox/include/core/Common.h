#ifndef COMMON_H
#define COMMON_H

#include <variant>
#include <string>

using LiteralValue = std::variant<std::string, double, bool, std::nullptr_t>;

#endif // !COMMON_H
