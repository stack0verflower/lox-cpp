#include "interpreter/Environment.h"
#include "core/Error.h"

Environment::Environment() {}

Environment::Environment(std::shared_ptr<Environment> enclosing) : enclosing(std::move(enclosing)) {}  // for local scope

void Environment::define(const std::string& name, const LiteralValue& value) {
	/*
	We have made one interesting semantic choice. When we add the key to the map, we don’t check to see if it’s already present. 
	That means that this program works:

	var a = "before";
	print a; // "before".
	var a = "after";
	print a; // "after".
	*/

	// No need for environment switch. New variables are always defined in the current environment, never in the enclosing environment. 
	// So, we just add it to the current environment's map.
	values[name] = value;
}

LiteralValue Environment::get(const Token& name) {
	// Lexemme stores the variable name, i.e, actual text of the token as it appears in the source code.
	if (values.count(name.lexeme)) {
		return values[name.lexeme];
	}
	
	// If you don't find the variable in the current environment, look in the enclosing environment. This is how we implement lexical scoping.
	if (enclosing != nullptr) return enclosing->get(name);

	// At this point, that variable was used without declaring
	throw RuntimeError(name, "Undefined variable '" + name.lexeme + "'.");
}

void Environment::assign(const Token& name, const LiteralValue& value) {
	if (values.count(name.lexeme)) {
		values[name.lexeme] = value;
		return;
	}

	// Same concept as get(). No such variable in current environment, go to enclosing environment.
	if (enclosing != nullptr) {
		enclosing->assign(name, value);
		return;
	}

	throw RuntimeError(name, "Undefined variable '" + name.lexeme + "'.");
}


