#include <iostream>
#include <string>
#include "Lexer.h"   // adjust name if needed

int main() {
    std::string source = R"(

// Sample Lox program
var x = 42;
var y = 3.14;
var name = "Lox";

if (x >= 10) {
    print name + " value:";
    print x + y;
}

)";

    Lexer lexer(source);
    auto tokens = lexer.scanSource();   // or scanTokens() depending on your name

    for (const auto& token : tokens) {
        std::cout << token.toString() << std::endl;
    }

    return 0;
}


/*
#include "Lox.h"

int main(int argc, char* argv[]) {
    Lox::main(argc, argv);
    return 0;
}
*/