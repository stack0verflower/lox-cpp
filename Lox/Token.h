#ifndef TOKEN_H
#define TOKEN_H

#include <string>
#include <variant>

// Define the only types a literal can actually be, it actually means a custom data type, which can hold a value of any one of these values
using Literal = std::variant<std::string, double, bool, std::nullptr_t>;

enum class TokenType {
	// Single-character tokens.
	LEFT_PAREN, RIGHT_PAREN, LEFT_BRACE, RIGHT_BRACE,
	COMMA, DOT, MINUS, PLUS, SEMICOLON, SLASH, STAR,

	// One or two character tokens.
	BANG, BANG_EQUAL,
	EQUAL, EQUAL_EQUAL,
	GREATER, GREATER_EQUAL,
	LESS, LESS_EQUAL,

	// Literals.
	IDENTIFIER, STRING, NUMBER,

	// Keywords.
	AND, CLASS, ELSE, FALSE, FUN, FOR, IF, NIL, OR,
	PRINT, RETURN, SUPER, THIS, TRUE, VAR, WHILE,

	EoF,
};

struct Token {
	TokenType type;
	std::string lexeme;	// Stores literal value, like "123", "hello", "true", etc. It is the actual text of the token as it appears in the source code.
	Literal literal;	// This stores the actual lexemme value intended to be used in the program, like 123, "hello", true, etc. It is the value of the token after it has been processed and converted to its appropriate type.
	int line;

	Token(TokenType token, std::string lexeme, Literal literal, int line) 
		: type(token), lexeme(lexeme), literal(literal), line(line) {}

public:
	std::string toString() const {
		return "Token(" + std::to_string(static_cast<int>(type)) + ", " + lexeme + ", " + std::to_string(line) + ")";
	}
};

#endif // !TOKEN_H\
