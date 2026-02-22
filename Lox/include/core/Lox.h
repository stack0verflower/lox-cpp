#ifndef LOX_H
#define LOX_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include "interpreter/Interpreter.h"

class Lox {
public:
    static bool hadError;

    static void main(int argc, char* argv[]);

    static void error(int line, const std::string& message);

private:
    static void runFile(const std::string& path);

    static void runPrompt();

    static void run(const std::string& source);

    static void report(int line, const std::string& where, const std::string& message);
};

#endif // !LOX_H

