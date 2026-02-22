#ifndef PARSER_H
#define PARSER_H

#include "core/Token.h"
#include "core/Lox.h"
#include "parser/Expr.h"
#include <vector>

/*
* Precedence table for Lox operators:
---------------------------------------------------------------
| Name       | Operators           | Associativity            |
---------------------------------------------------------------
| Equality   | ==  !=              | Left                     |
| Comparison | >  >=  <  <=        | Left                     |
| Term       | -  +                | Left                     |
| Factor     | /  *                | Left                     |
| Unary      | !  -                | Right                    |
---------------------------------------------------------------

This applies to expressions. We have various other types of statements.

GRAMMAR (Precedence from lowest to highest):
────────────────────────────────────────────────────────────────────────────

expression  → assignment
assignment  → IDENTIFIER "=" assignment | equality
equality    → comparison ( ( "==" | "!=" ) comparison )*
comparison  → term ( ( ">" | ">=" | "<" | "<=" ) term )*
term        → factor ( ( "+" | "-" ) factor )*
factor      → unary ( ( "*" | "/" ) unary )*
unary       → ( "!" | "-" ) unary | primary
primary     → NUMBER | IDENTIFIER | "(" expression ")"

PRECEDENCE TABLE (lowest to highest):
────────────────────────────────────────────────────────────────────────────

Level 1 (Lowest):  = (assignment) - RIGHT associative
Level 2:           == !=
Level 3:           > >= < <=
Level 4:           + -
Level 5:           * /
Level 6:           ! - (unary)
Level 7 (Highest): () (grouping), NUMBER, IDENTIFIER

RULE: Higher precedence = Deeper in tree = Evaluated first

*/

class Parser {
private:
	const std::vector<Token>& tokens;
	int current;

private:
	struct ParseError {};
	Token consume(TokenType type, const std::string& message);
	static ParseError error(Token token, const std::string& message);
	void synchronize();

public:
	Parser(const std::vector<Token>& tokens);

	std::unique_ptr<Expr> parse();

private:
	std::unique_ptr<Expr> parseExpression();
	std::unique_ptr<Expr> parseEquality();
	std::unique_ptr<Expr> parseComparison();
	std::unique_ptr<Expr> parseTerm();
	std::unique_ptr<Expr> parseFactor();
	std::unique_ptr<Expr> parseUnary();
	std::unique_ptr<Expr> parsePrimary();
private:
	Token peek() const;
	bool isAtEnd() const;
	Token advance();
	Token previous() const;
	bool match(std::initializer_list<TokenType> types);
	bool check(TokenType type) const;
};

#endif // !PARSER_H
