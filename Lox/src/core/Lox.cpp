#include "core/Lox.h"

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
}

void Lox::runFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Could not open file " << path << std::endl;
        exit(74);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    run(buffer.str());

    // If there's an error in the code, don't execute
    if (hadError) exit(65);
}

void Lox::runPrompt() {
    std::string line;
    for (;;) {
        std::cout << "> ";
        if (!std::getline(std::cin, line)) break; // Ctrl+D to exit
        run(line);
        hadError = false; // Reset error so it doesn't kill the REPL
    }
}

void Lox::run(const std::string& source) {
    // This is where your Lexer/Scanner will eventually go
    std::cout << "Running: " << source << std::endl;

    // Example error trigger
    // report(1, "", "Unexpected character.");
}

void Lox::report(int line, const std::string& where, const std::string& message) {
    std::cerr << "[line " << line << "] Error" << where << ": " << message << std::endl;
    hadError = true;
}

// Initialize the static member
bool Lox::hadError = false;
