#ifndef LOX_H
#define LOX_H

#include <string>

class Lox;
class RuntimeError;

class Lox {
public:
    static bool hadError;
	static bool hadRuntimeError;

    // Entry point
    static void main(int argc, char* argv[]);

	// This handles compile time errors, which are errors that occur during the scanning and parsing phases.
    static void error(int line, const std::string& message);
	// This handles runtime errors, which are errors that occur during the interpretation phase.
	static void runtimeError(const RuntimeError& error);

private:
    static void runFile(const std::string& path);

    static void runPrompt();

    static void run(const std::string& source);

    static void report(int line, const std::string& where, const std::string& message);
};

#endif // !LOX_H

