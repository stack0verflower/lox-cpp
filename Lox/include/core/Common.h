#ifndef COMMON_H
#define COMMON_H

#include <variant>
#include <string>
#include <memory>

class LoxCallable;

using LiteralValue = std::variant<std::string, double, bool, std::nullptr_t, std::shared_ptr<LoxCallable>>;

#endif // !COMMON_H
