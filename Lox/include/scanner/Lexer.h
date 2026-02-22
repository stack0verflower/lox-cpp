#ifndef LEXER_H
#define LEXER_H

#include "core/Token.h"
#include "core/Lox.h"
#include <vector>
#include <string>

class Lexer {
private:
	std::string source;
	std::vector<Token> tokens;
	/*
		The start and current fields are offsets that index into the string. 
		The start field points to the first character in the lexeme being scanned,
		and current points at the character currently being considered. The line 
		field tracks what source line current is on so we can produce tokens that know their location.
	*/
	int start = 0;
	int current = 0;
	int line = 1;
public:
	Lexer(const std::string& source) : source(source) {}
	std::vector<Token> scanSource();
	void tokenize();
	
	bool isAtEnd() const;
	char advance();
private:
	void addToken(TokenType type);
	void addToken(TokenType type, LiteralVal literal);
	bool match(char expected);

	/*
	It’s sort of like advance(), but doesn’t consume the character. This is called lookahead. 
	Since it only looks at the current unconsumed character, we have one character of lookahead. 
	The smaller this number is, generally, the faster the scanner runs. 
	The rules of the lexical grammar dictate how much lookahead we need. 
	Fortunately, most languages in wide use peek only one or two characters ahead.
	*/

	char peek() const;
	char peekNext() const;

	// Handle scan literals helper functions
	void handleStringLiteral();
	void handleNumberLiteral();
	void handleIdentifier();
};

#endif // !LEXER_H
