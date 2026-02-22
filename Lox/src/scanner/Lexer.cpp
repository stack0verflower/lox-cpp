#include "scanner/Lexer.h"
#include <unordered_map>

static const std::unordered_map<std::string, TokenType> keywords = {
    {"and", TokenType::AND},
    {"class", TokenType::CLASS},
    {"else", TokenType::ELSE},
    {"false", TokenType::FALSE},
    {"for", TokenType::FOR},
    {"fun", TokenType::FUN},
    {"if", TokenType::IF},
    {"nil", TokenType::NIL},
    {"or", TokenType::OR},
    {"print", TokenType::PRINT},
    {"return", TokenType::RETURN},
    {"super", TokenType::SUPER},
    {"this", TokenType::THIS},
    {"true", TokenType::TRUE},
    {"var", TokenType::VAR},
    {"while", TokenType::WHILE}
};

std::vector<Token> Lexer::scanSource() {
    while (!isAtEnd()) {
        // We are at the beginning of the next lexeme.
        start = current;
        tokenize();
    }
    tokens.emplace_back(TokenType::EoF, "", nullptr, line);
    return tokens;
}

void Lexer::tokenize() {
	char c = advance();
	
    switch (c) {
        case '(': addToken(TokenType::LEFT_PAREN); break;
        case ')': addToken(TokenType::RIGHT_PAREN); break;
        case '{': addToken(TokenType::LEFT_BRACE); break;
        case '}': addToken(TokenType::RIGHT_BRACE); break;
        case ',': addToken(TokenType::COMMA); break;
        case '.': addToken(TokenType::DOT); break;
        case '-': addToken(TokenType::MINUS); break;
        case '+': addToken(TokenType::PLUS); break;
        case ';': addToken(TokenType::SEMICOLON); break;
        case '*': addToken(TokenType::STAR); break;

        // Currently, our c is character before current character (used advance()).
		// We will check current character to see if it forms a two-character token, like <=, !=, etc. 
        case '!':
            addToken(match('=') ? TokenType::BANG_EQUAL : TokenType::BANG);
			break;
        case '=':
            addToken(match('=') ? TokenType::EQUAL_EQUAL : TokenType::EQUAL);
            break;
        case '<':
            addToken(match('=') ? TokenType::LESS_EQUAL : TokenType::LESS);
            break;
		case '>':
            addToken(match('=') ? TokenType::GREATER_EQUAL : TokenType::GREATER);
			break;

        /*
        We’re still missing one operator: / for division. 
        That character needs a little special handling because comments begin with a slash too.
        */

        case '/':
            if (match('/')) {
                // A comment goes until the end of the line.
                while (peek() != '\n' && !isAtEnd()) advance();
            } else {
                addToken(TokenType::SLASH);
            }
			break;

        // Skip meaningless characters.
        case ' ':
		case '\r':
		case '\t':
            break;

		case '\n':
            line++;
			break;

		// Moving on to more complex tokens, like string literals, numbers, identifiers, and keywords.
        
		// 1. String literals begin with ", goes till the next unescaped ", and can contain any characters in between.
		case '"': handleStringLiteral(); break;

        default:
            // 2. Number literals
            // Why this here? Because, case statements must be followed by a CONSTANT, and isdigit() is not that.
            if (isdigit(c)) {
                handleNumberLiteral();
            }

            // 3. Identifiers and keywords
            else if (isalpha(c)) {
                handleIdentifier();
            }

            else {
                std::cout << "Unknown char: [" << c << "] ASCII: " << (int)c << "\n";
                Lox::error(line, "Unexpected character.");
            }
    }
}

void Lexer::handleStringLiteral() {
    // Lox supports multi-line strings.
    // Loop till the current character is closing "
    while (!isAtEnd() && peek() != '"') {
        if (peek() == '\n') line++;
        advance();
    }

    if (isAtEnd()) {
        Lox::error(line, "Unterminated string.");
        return;
    }
    
    // This literal now is ". Consume it.
    advance();

    // Trim the surrounding quotes.
    std::string value = source.substr(start + 1, current - start - 1);
    addToken(TokenType::STRING, value);
}

void Lexer::handleNumberLiteral() {
    while (isdigit(peek())) advance();

    // You broke, either the '.' was encountered of decimal, or, it terminated due to end of number literal.
    // Handle decimal case
    if (peek() == '.' && isdigit(peekNext())) {
        // Consume the "."
        advance();

        // Move on to fractional parts
        while (isdigit(peek())) advance();
    }

    // std::stod is a standard library function that converts a std::string into a double.
    // Lives in #include <string>
    std::string text = source.substr(start, current - start);
    double value = std::stod(text);

    addToken(TokenType::NUMBER, value);
}

void Lexer::handleIdentifier() {
    /*
    That rule states that if we can match orchid as an identifier and or as a keyword, then the former wins. 
    This is also why we tacitly assumed, previously, that <= should be scanned as a single <= token and not < followed by =
    */


    while (isalnum(peek())) advance();
    std::string text = source.substr(start, current - start);

    auto it = keywords.find(text);
    if (it != keywords.end()) addToken(it->second);
    else addToken(TokenType::IDENTIFIER);
}

bool Lexer::isAtEnd() const {
	return current >= source.length();
}

char Lexer::advance() {
	return source[current++];
}

void Lexer::addToken(TokenType type) {
	addToken(type, nullptr);
}

void Lexer::addToken(TokenType type, LiteralValue literal) {
	std::string text = source.substr(start, current - start);
	tokens.emplace_back(type, text, literal, line);
}

bool Lexer::match(char expected) {
    if (isAtEnd()) return false;
    if (source[current] != expected) return false;
    current++;
    return true;
}

char Lexer::peek() const {
    if (isAtEnd()) return '\0';
    return source[current];
}

char Lexer::peekNext() const {
    if (current + 1 >= source.length()) return '\0';
    return source[current + 1];
}
