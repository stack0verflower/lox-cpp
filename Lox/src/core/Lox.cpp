#include "core/Lox.h"

#include <vector>
#include <sstream>
#include <iostream>
#include <fstream>

#include "core/Error.h"
#include "interpreter/Interpreter.h"
#include "scanner/Lexer.h"
#include "parser/Parser.h"
#include "parser/ASTPrinter.h"

// TODO improve code readibility, like use Classname::something rather than something

// Initialize the static member
bool Lox::hadError = false;
bool Lox::hadRuntimeError = false;

void Lox::main(int argc, char* argv[]) {
    if (argc > 2) {
        std::cout << "Usage: cpplox [script]" << std::endl;
        exit(64);
    }
    else if (argc == 2) {
        runFile(argv[1]);
    }
    else {
        runPrompt();
    }
}

void Lox::error(int line, const std::string& message) {
    report(line, "", message);
    hadError = true;
}

void Lox::runtimeError(const RuntimeError& error) {
    report(error.line, "", error.what());
	hadRuntimeError = true;
}

void Lox::runFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Could not open file: " << path << std::endl;
        exit(74);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    run(buffer.str());

    // Indicate error in exit code
    if (hadError) exit(65);
    if (hadRuntimeError) exit(70);
}

void Lox::runPrompt() {
    std::string line;

    for (;;) {
        std::cout << "> ";
        if (!std::getline(std::cin, line)) break;

        run(line);

        // Don't kill session on error
        hadError = false;
        hadRuntimeError = false;
    }
}

void Lox::run(const std::string& source) {
    try {
        Lexer lexer(source);
        auto tokens = lexer.scanSource();

        if (hadError) return;

        // Parser calls back to report errors
        Parser parser(tokens, [](int line, const std::string& message) {
            std::cerr << "[line " << line << "] Parse error: "
                << message << std::endl;
            Lox::hadError = true;
        });

        std::vector<std::unique_ptr<Stmt>> statements = parser.parse();

        // Stop if parsing had errors
        if (hadError) return;

        // Interpretation phase
        Interpreter interpreter;
        interpreter.interpret(statements);

    }
    catch (const LexError& e) {
        // Lexer throws error, at a point where tokenization broke
        error(e.line, e.what());

    } catch (const RuntimeError& e) {
        // Interpreter throws RuntimeError with token info
        runtimeError(e);

    }
    catch (const std::exception& e) {
        // Catch any other unexpected errors
        std::cerr << "Unexpected error: " << e.what() << std::endl;
        hadError = true;
    }

	// Note, ParseErrors are thrown by Parser itself, so that it can recover from those error and synchronize itself to continue parsing the rest of the file, and report all errors at once. So, we don't catch ParseErrors here, since they are handled internally by the Parser.
}

void Lox::report(int line, const std::string& where, const std::string& message) {
    std::cerr << "[line " << line << "] Error" << where << ": " << message << std::endl;
    hadError = true;
}

