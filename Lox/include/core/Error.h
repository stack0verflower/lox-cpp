#ifndef ERROR_H
#define ERROR_H

#include <stdexcept>
#include "core/Token.h"

class LoxError;
class LexError;
class ParseError;
class RuntimeError;

class LoxError : public std::runtime_error {
public:
	int line;
	LoxError(int line, const std::string& message) : std::runtime_error(message), line(line) {}
};

class LexError : public LoxError {
public:
	LexError(int line, const std::string& message) : LoxError(line, message) {}
};

class ParseError : public LoxError {
public:
    Token token;
    ParseError(const Token& token, const std::string& message) : LoxError(token.line, message), token(token) {}
};

class RuntimeError : public LoxError {
public:
    Token token;
    RuntimeError(const Token& token, const std::string& message) : LoxError(token.line, message), token(token) {}
};

#endif // !ERROR_H
