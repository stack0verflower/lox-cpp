#include "interpreter/LoxCallable.h"
#include "interpreter/Interpreter.h"
#include "interpreter/Return.h"

LoxFunction::LoxFunction(const FuncStmt& declaration, std::shared_ptr<Environment> closure) : declaration(declaration), closure(std::move(closure)) {}

LiteralValue LoxFunction::call(Interpreter& interpreter, std::vector<LiteralValue> arguments) {
	// If a function is nested? Then it's enclosing scope is not globalEnv, but the `closure` or current scope, which is passed down.
	// This syntax creates a new shared_ptr that shares ownership of the same Environment object that closure points to.
	auto environment = std::make_shared<Environment>(closure);

	for (int i = 0; i < declaration.params.size(); i++) {
		// You have params passed by user, in arguments. To what values? Those are included in declaration->params, which was created while defining function.
		/*
			fun add(a, b) => declaration->params has a, b
			add(1, 2)     => argument list has 1, 2. You are defining a=1, b=2 in environment.
		*/

		// These go in function slots.
		environment->push_slots(arguments[i]);
	}

	// Execute this declaraction.body(); but if a ReturnException is encountered, return this call.
	try {
		interpreter.executeBlock(declaration.body, environment);
	} catch (ReturnException& returnValue) {
		// That struct stores return value as a value field.
		return returnValue.value;
	}

	return nullptr; // (We’ll add return values later.)
}

int LoxFunction::arity() {
	return declaration.params.size();
}

std::string LoxFunction::toString() {
	return "<fn " + declaration.name.lexeme + ">";
}

LoxLambda::LoxLambda(const LambdaExpr& declaration, std::shared_ptr<Environment> closure) : declaration(declaration), closure(std::move(closure)) {}

LiteralValue LoxLambda::call(Interpreter& interpreter, std::vector<LiteralValue> arguments) {
	auto environment = std::make_shared<Environment>(closure);

	for (int i = 0; i < declaration.params.size(); i++) {
		environment->push_slots(arguments[i]);
	}

	try {
		interpreter.executeBlock(declaration.body, environment);
	}
	catch (ReturnException& returnValue) {
		return returnValue.value;
	}

	return nullptr;
}

int LoxLambda::arity() {
	return declaration.params.size();
}

std::string LoxLambda::toString() {
	return "<lambda fn>";
}

