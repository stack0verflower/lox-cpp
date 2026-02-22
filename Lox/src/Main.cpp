#include <iostream>
#include <string>
#include <vector>
#include "scanner/Lexer.h"
#include "parser/Parser.h"
#include "parser/ASTPrinter.h"
#include "interpreter/Interpreter.h"

int main() {
    std::vector<std::string> tests = {
        "1 + 2 * 3",
        "(1 + 2) * 3",
        "1 - 2 - 3",
        "1 * 2 + 3 * 4",
        "--1",
        "1 < 2 == true",
        "!(1 + 2 * (3 - 4) >= 5 == false)",
        "-((3 + 4) * (5 - 2) == 21) != false"
    };

    for (const auto& source : tests) {
        std::cout << "Input:  " << source << std::endl;

        Lexer lexer(source);
        auto tokens = lexer.scanSource();

        Parser parser(tokens);
        std::unique_ptr<Expr> expression = parser.parse();

        if (expression) {
			Interpreter interpreter;
			interpreter.interpret(*expression);
        }
        else {
            std::cout << "Parse failed." << std::endl;
        }

        std::cout << "-----------------------------------\n";
    }

    return 0;
}

/*

/*
#include "Lox.h"

int main(int argc, char* argv[]) {
    Lox::main(argc, argv);
    return 0;
}
*/