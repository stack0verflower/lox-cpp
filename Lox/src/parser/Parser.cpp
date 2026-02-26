#include "parser/Parser.h"
#include "core/Error.h"

// ================================================================================================================================================

Parser::Parser(const std::vector<Token>& tokens, ErrorCallback onError) : tokens(tokens), current(0), errorCallback(onError) {}

std::vector<std::unique_ptr<Stmt>> Parser::parse() {
	std::vector<std::unique_ptr<Stmt>> statements;

    while (!isAtEnd()) {
        try {
            // statements.push_back(parseDeclaration());
            // For now, we only have expression statements, so we will just parse expressions.
            statements.push_back(declaration());
        } catch (const ParseError& error) {
            // Call the callback if provided
            if (errorCallback) {
                errorCallback(error.token.line, error.what());
            }

            // Recover and parse remainder, so that we are able to catch all the errors of the file contents at once.
            synchronize();
        }
	}

    return statements;
}

// ================================================================================================================================================

Token Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) return advance();

    throw ParseError(peek(), message);
}

void Parser::synchronize() {
    /*
    You encountered error in a line. If this method is not present, the parser will go haywire and report a lot of errors in the following lines, which are just consequences of the first error. 
	What does that mean? If a token is invalid, other tokens that follow it may not be parsed correctly, leading to more errors that are not actually errors in the code, but just consequences of the first error.
    This method is used to skip tokens until we find a statement boundary, which is usually a semicolon or a keyword that can start a statement.

	Such errors are called "cascading errors" or "ghost errors", and they can be very confusing for the user, since they are not the actual error, but just consequences of the first error.
    */

    advance();
    while (!isAtEnd()) {
        if (previous().type == TokenType::SEMICOLON) return;
        switch (peek().type) {
            case TokenType::CLASS:
            case TokenType::FUN:
            case TokenType::VAR:
            case TokenType::FOR:
            case TokenType::IF:
            case TokenType::WHILE:
            case TokenType::PRINT:
            case TokenType::RETURN:
                return;
            default:
                break;
        }
        advance();
    }
}

// ================================================================================================================================================

// Helper functions

Token Parser::peek() const {
	return tokens[current];
}

bool Parser::isAtEnd() const {
    return peek().type == TokenType::EoF;
}

Token Parser::advance() {
    if (!isAtEnd()) current++;
    return previous();
}

Token Parser::previous() const {
    return tokens[current - 1];
}

bool Parser::match(std::initializer_list<TokenType> types) {
    for (TokenType type : types) {
        if (check(type)) {
			advance();
            return true;
		}
    }

    return false;
}

bool Parser::check(TokenType type) const {
    if (isAtEnd()) return false;  
    return peek().type == type;    
}

// ================================================================================================================================================
    
std::unique_ptr<Stmt> Parser::declaration() {
    if (match({ TokenType::VAR })) return varDeclaration();

	// Pipeline, first match a declaration, if not, then match a statement, if not, then we have a syntax error, which will be handled in the error handling phase.
	return statement();
}

// Main function for parsing statements
std::unique_ptr<Stmt> Parser::statement() {
    if (match({ TokenType::PRINT })) return printStatement();
    // BlockSmt constructor requires a vector of unique_ptr of statements.
	if (match({ TokenType::LEFT_BRACE })) return std::make_unique<BlockStmt>(blockStatement());
    return expressionStatement();
}

std::unique_ptr<Stmt> Parser::varDeclaration() {
	Token name = consume(TokenType::IDENTIFIER, "Expect variable name.");
    std::unique_ptr<Expr> initializer = nullptr;

    if (match({ TokenType::EQUAL })) {
        initializer = parseExpression();
    }

    // At this point, you have to consume the semicolon
    consume(TokenType::SEMICOLON, "Expect ';' after variable declaration.");
	return std::make_unique<VarDeclStmt>(name, std::move(initializer));
}

std::unique_ptr<Stmt> Parser::printStatement() {
	std::unique_ptr<Expr> value = parseExpression();
    consume(TokenType::SEMICOLON, "Expect ';' after value.");
	return std::make_unique<PrintStmt>(std::move(value));
}

std::vector<std::unique_ptr<Stmt>> Parser::blockStatement() {
    std::vector<std::unique_ptr<Stmt>> statements;

    while (!isAtEnd() && !check(TokenType::RIGHT_BRACE)) {
        statements.push_back(declaration());
    }

    consume(TokenType::RIGHT_BRACE, "Expect '}' after block.");
    return statements;
}

std::unique_ptr<Stmt> Parser::expressionStatement() {
    std::unique_ptr<Expr> value = parseExpression();
    consume(TokenType::SEMICOLON, "Expect ';' after value.");
    return std::make_unique<ExprStmt>(std::move(value));
}

// =================================================================================================================================================

// Main functions for parsing expressions
std::unique_ptr<Expr> Parser::parseExpression() {
    // This will be entry point to parsing assignment expressions
    // Recall, our pipeline, assignment operator has least precedence. Hence, first, we will enter into assignment operator
    return parseAssignment();
}

std::unique_ptr<Expr> Parser::parseAssignment() {
    /*
    Suppose take, a = 20, pipeline would be:
    parseExpression()
    → parseAssignment()
        → parseEquality()  [parses left side normally]
            → ... → parsePrimary()
                → sees IDENTIFIER "a"
                → returns VariableExpr("a")
        ← back in parseAssignment()
        → sees "=" token — match!
        → check: is left a VariableExpr? YES
        → grab name token "a"
        → parseAssignment() [right side, recursive!]
            → parseEquality()
                → ... → parsePrimary()
                    → sees NUMBER 20
                    → returns Literal(20)
        → return Assign("a", Literal(20))
    → consume SEMICOLON → This is done by expressionStatement, Assignment is a type of expressionStatement
    */

    std::unique_ptr<Expr> expr = parseEquality();
    
    if (match({ TokenType::EQUAL })) {
        if (auto* var = dynamic_cast<VariableExpr*>(expr.get())) {
            Token name = var->name;
            std::unique_ptr<Expr> value = parseAssignment();
            return std::make_unique<Assign>(name, std::move(value));
        }

        throw ParseError(previous(), "Invalid assignment target.");
    }

    return expr;
}

std::unique_ptr<Expr> Parser::parseEquality() {
    // comparison ( ( "!=" | "==" ) comparison )*
	std::unique_ptr<Expr> expr = parseComparison();
    while (match({ TokenType::BANG_EQUAL, TokenType::EQUAL_EQUAL })) {
        Token op = previous();
		std::unique_ptr<Expr> right = parseComparison();
		expr = std::make_unique<Binary>(std::move(expr), op, std::move(right));
    }

	return expr;
}

std::unique_ptr<Expr> Parser::parseComparison() {
    // term((">" | ">=" | "<" | "<=") term)*
	std::unique_ptr<Expr> expr = parseTerm();
    while (match({ TokenType::GREATER, TokenType::GREATER_EQUAL, TokenType::LESS, TokenType::LESS_EQUAL })) {
        Token op = previous();
        std::unique_ptr<Expr> right = parseTerm();
        expr = std::make_unique<Binary>(std::move(expr), op, std::move(right));
    }

    return expr;
}

std::unique_ptr<Expr> Parser::parseTerm() {
    // factor ( ( "-" | "+" ) factor )*
    std::unique_ptr<Expr> expr = parseFactor();
    while (match({ TokenType::MINUS, TokenType::PLUS })) {
        Token op = previous();
        std::unique_ptr<Expr> right = parseFactor();
        expr = std::make_unique<Binary>(std::move(expr), op, std::move(right));
    }

    return expr;
}

std::unique_ptr<Expr> Parser::parseFactor() {
    // unary ( ( "/" | "*" ) unary )*
    std::unique_ptr<Expr> expr = parseUnary();
    while (match({ TokenType::SLASH, TokenType::STAR })) {
        Token op = previous();
        std::unique_ptr<Expr> right = parseUnary();
        expr = std::make_unique<Binary>(std::move(expr), op, std::move(right));
    }

    return expr;
}

std::unique_ptr<Expr> Parser::parseUnary() {
    // ( "!" | "-" ) unary | primary
    if (match({ TokenType::BANG, TokenType::MINUS })) {
        Token op = previous();
        std::unique_ptr<Expr> right = parseUnary();
        return std::make_unique<Unary>(op, std::move(right));
    }

    return parsePrimary();
}

std::unique_ptr<Expr> Parser::parsePrimary() {
    // NUMBER | STRING | "true" | "false" | "nil" | "(" expression ")"
    if (match({ TokenType::FALSE })) return std::make_unique<Literal>(false);
    if (match({ TokenType::TRUE })) return std::make_unique<Literal>(true);
    if (match({ TokenType::NIL })) return std::make_unique<Literal>(nullptr);

    if (match({ TokenType::NUMBER, TokenType::STRING })) {
        return std::make_unique<Literal>(previous().literal);
    }

	// If the token in the current position is an identifier, we will create a VariableExpr for it, and return it.
    if (match({ TokenType::IDENTIFIER })) {
        return std::make_unique<VariableExpr>(previous());
    }

    if (match({ TokenType::LEFT_PAREN })) {
        std::unique_ptr<Expr> expr = parseExpression();
        // We expect a right parenthesis after the expression, otherwise we have a syntax error.
		consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
        return std::make_unique<Grouping>(std::move(expr));
    }
    // If we reach here, it means we have a syntax error, since we expected a primary expression, but we got something else.
    // We will handle this in the error handling phase, for now we just assume the input is correct.
    throw ParseError(peek(), "Expect expression.");
}

