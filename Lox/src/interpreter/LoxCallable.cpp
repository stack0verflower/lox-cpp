#include "interpreter/LoxCallable.h"
#include "interpreter/Interpreter.h"
#include "interpreter/Return.h"
#include "interpreter/LoxInstance.h"

LoxFunction::LoxFunction(const FuncStmt& declaration, std::shared_ptr<Environment> closure, bool isInitializer) : declaration(declaration), closure(std::move(closure)), isInitializer(isInitializer) {}

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
		// If initializer has return, just return `this`.
		if (isInitializer) return closure->getAt(0, 0);

		// That struct stores return value as a value field.
		return returnValue.value;
	}

	// If no return value for an initializer, return `this`.
	if (isInitializer) return closure->getAt(0, 0);

	return nullptr; // (We’ll add return values later.)
}

std::shared_ptr<LoxFunction> LoxFunction::bind(std::shared_ptr<LoxInstance> instance) {
	auto env = std::make_shared<Environment>(closure);
	env->push_slots(instance);
	return std::make_shared<LoxFunction>(declaration, env, isInitializer);
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

LoxClass::LoxClass(const std::string& name, std::unordered_map<std::string, std::shared_ptr<LoxFunction>> methods) : name(name), methods(std::move(methods)) {}

LiteralValue LoxClass::call(Interpreter& interpreter, std::vector<LiteralValue> arguments) {
	auto instance = std::make_shared<LoxInstance>(std::make_shared<LoxClass>(*this));

	auto initializer = findMethod("init");
	if (initializer) {
		initializer->bind(instance)->call(interpreter, arguments);
	}

	return instance;
}

// This stuff is for constructors.
int LoxClass::arity() {
	auto initializer = findMethod("init");
	if (initializer) {
		return initializer->arity();
	}
	return 0;
}

std::string LoxClass::toString() {
	return name;
}

std::shared_ptr<LoxFunction> LoxClass::findMethod(const std::string& name) {
	if(methods.contains(name)) {
		return methods[name];
	}

	return nullptr;
}
