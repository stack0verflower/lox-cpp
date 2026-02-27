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
    if (match({ TokenType::FOR })) return forStatement();
    if (match({ TokenType::IF })) return ifStatement();
    if (match({ TokenType::PRINT })) return printStatement();
    if (match({ TokenType::WHILE })) return whileStatement();
    // BlockSmt constructor requires a vector of unique_ptr of statements.
	if (match({ TokenType::LEFT_BRACE })) return std::make_unique<BlockStmt>(blockStatement());
    return expressionStatement();
}

std::unique_ptr<Stmt> Parser::varDeclaration() {
	Token name = consume(TokenType::IDENTIFIER, "Expect variable name.");

    /*
    NOTE: If someone does var temp;
    We are default initializing temp to nullptr. If `=` exists after identifier, then it means, we have a value to assign. This is useful, look

    var a = 0;
    var temp;           => Automatically initialized to nullptr.

    for (var b = 1; a < 10000; b = temp + b) {
      print a;
      temp = a;         => Overriden nullptr with value of a.
      a = b;
    }
    */
    std::unique_ptr<Expr> initializer = nullptr;

    if (match({ TokenType::EQUAL })) {
        initializer = parseExpression();
    }

    // At this point, you have to consume the semicolon
    consume(TokenType::SEMICOLON, "Expect ';' after variable declaration.");
	return std::make_unique<VarDeclStmt>(name, std::move(initializer));
}

// This is just a sugared while loop. This method essentially converts a forStatement into set of statements, transforming into while loop.
std::unique_ptr<Stmt> Parser::forStatement() {
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'for'.");

    // The first clause following that is the initializer. That is a statement, eg. var a = 10;
    std::unique_ptr<Stmt> initializer = nullptr;
    if (match({ TokenType::SEMICOLON })) initializer = nullptr;             // For cases like, for(; a<something; a++) => Skip initializer completely.
    else if (match({ TokenType::VAR })) initializer = varDeclaration();     // You are declaring a new variable. var a = 12;
    else initializer = expressionStatement();                               // Existing variable, just going like for (a=0; ...). This goes to expressionStatement. There, we fall to parseAssignment(), where this works


    // The semicolons after, say a > 2; => This is a whole expression statement. It is consumed, see expressionStatement() defination.
    
    // Same concept here, look
    std::unique_ptr<Expr> condition = nullptr;
    if (!check({ TokenType::SEMICOLON })) {
        condition = parseExpression();
    }
    // parseExpression() doesnt consume ;, and why should it? Expressions are not statements.
    consume(TokenType::SEMICOLON, "Expect ';' after loop condition.");

    // Look, condition; increment) => Previous step consumed condition and ;, so if no increment, then we would have ;) => ) directly after ;.
    std::unique_ptr<Expr> increment = nullptr;
    if (!check({ TokenType::RIGHT_PAREN })) {
        increment = parseExpression();
    }

    consume(TokenType::RIGHT_PAREN, "Expect ')' after loop condition.");

    std::unique_ptr<Stmt> body = statement();

    // Let us construct body of a while loop from for loop. Look,
    /*
    for (declaration; condition; increment) { body } 
    translates to
    declaration;
    while (condition) { 
        body;
        increment;
    }
    */

    // 1. Scrap body with increment
    if (increment != nullptr) {
        std::vector<std::unique_ptr<Stmt>> bodyWithIncrement;
        bodyWithIncrement.push_back(std::move(body));
        // increment is an expression. Just utitlise an ExprStmt constructor and transform it into an Expr.
        // If you are scraping increment into body, increment transforms into a statement, as body is a set of statements.
        bodyWithIncrement.push_back(std::make_unique<ExprStmt>(std::move(increment)));
        body = std::make_unique<BlockStmt>(std::move(bodyWithIncrement));
    }

    // 2. Wrap it in a while. In short, make a WhileStmt
    if (condition == nullptr) condition = std::make_unique<Literal>(true);  // This means that, for loops of type for(instantiation; ; increment) {}; transforms to a while(True) type loops, think about that.
    // body is till now a BlockStmt, transforms into WhileStmt. Look constructor of WhileStmt.
    body = std::make_unique<WhileStmt>(std::move(condition), std::move(body));

    // 4. If initializer exists, wrap everything in outer block
    if (initializer != nullptr) {
        std::vector<std::unique_ptr<Stmt>> outerBlock;
        outerBlock.push_back(std::move(initializer));
        outerBlock.push_back(std::move(body));
        body = std::make_unique<BlockStmt>(std::move(outerBlock));
    }

    return body;

    // And Viola, the for loop is desugared into while loop!!
}

std::unique_ptr<Stmt> Parser::ifStatement() {
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'if'.");
    std::unique_ptr<Expr> condition = parseExpression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after 'if' condition.");

    std::unique_ptr<Stmt> thenBranch = statement();
    std::unique_ptr<Stmt> elseBranch = nullptr;

    if (match({ TokenType::ELSE })) {
        elseBranch = statement();
    }

    /*
    Instead, most languages and parsers avoid the problem in an ad hoc way. 
    No matter what hack they use to get themselves out of the trouble, 
            they always choose the same interpretation—the else is bound to the nearest if that precedes it.

    Our parser conveniently does that already. Since ifStatement() eagerly looks for an else before returning, 
            the innermost call to a nested series will claim the else clause for itself before returning to the outer if statements.
    */

    return std::make_unique<IfStmt>(std::move(condition), std::move(thenBranch), std::move(elseBranch));
}

std::unique_ptr<Stmt> Parser::printStatement() {
	std::unique_ptr<Expr> value = parseExpression();
    consume(TokenType::SEMICOLON, "Expect ';' after value.");
	return std::make_unique<PrintStmt>(std::move(value));
}

std::unique_ptr<Stmt> Parser::whileStatement() {
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'while'");
    std::unique_ptr<Expr> condition = parseExpression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after 'while' condition");
    std::unique_ptr<Stmt> body = statement();

    return std::make_unique<WhileStmt>(std::move(condition), std::move(body));
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
/*
    a = 20 pipeline:
    parseExpression()
    → parseAssignment()
        → parseOr()
            → parseAnd()
                → parseEquality()
                    → ... → parsePrimary()
                        → sees IDENTIFIER "a"
                        → returns VariableExpr("a")
                → no "and" token, returns up
            → no "or" token, returns up
        ← back in parseAssignment()
        → sees "=" token — match!
        → check: is left a VariableExpr? YES
        → grab name token "a"
        → parseAssignment() [right side, recursive!]
            → parseOr()
                → parseAnd()
                    → parseEquality()
                        → ... → parsePrimary()
                            → sees NUMBER 20
                            → returns Literal(20)
                    → no "and" token, returns up
                → no "or" token, returns up
        → return Assign("a", Literal(20))
    → consume SEMICOLON — done by expressionStatement()
*/
    std::unique_ptr<Expr> expr = parseOr();
    
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

std::unique_ptr<Expr> Parser::parseOr() {
    std::unique_ptr<Expr> expr  = parseAnd();

    // Think, what if you have nested ORs? Say, a || b || c? Then the tree will look like
    /*
                        OR
                OR              c                    => Dry run this while and look for yourself.
            a       b
    */
    while (match({ TokenType::OR })) {
        Token op = previous();
        std::unique_ptr<Expr> right = parseAnd();
        expr = std::make_unique<Logical>(std::move(expr), op, std::move(right));
    }

    return expr;
}

std::unique_ptr<Expr> Parser::parseAnd() {
    std::unique_ptr<Expr> expr = parseEquality();

    // Think, what if you have nested ORs? Say, a && b && c? Then the tree will look like
    /*
                        AND
                AND              c                    => Dry run this while and look for yourself.
            a       b
    */
    while (match({ TokenType::AND })) {
        Token op = previous();
        std::unique_ptr<Expr> right = parseEquality();
        expr = std::make_unique<Logical>(std::move(expr), op, std::move(right));
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

