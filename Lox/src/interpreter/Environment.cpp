#include "interpreter/Environment.h"
#include "core/Error.h"
#include <cassert>

Environment::Environment() {}

Environment::Environment(std::shared_ptr<Environment> enclosing) : enclosing(std::move(enclosing)) {}  // for local scope

// global define — name based, stores in values map
void Environment::define(const std::string& name, LiteralValue value) {
	values[name] = value;
}

// local define — index based, stores in slots vector. This is a direct map from Resolver's "sticky notes" to Environment's slots. So, we just add it to the current environment's map.
// Why maps? Index based operations are O(1), while name based operations are O(n) in worst case. So, for local variables, we want to use index based lookup for better performance. For global variables, we don't care much about performance, as they are not accessed as frequently as local variables, and we want the convenience of name based lookup.
void Environment::define(int index, const LiteralValue& value) {
	// Switch to index based defination for local variables. This is a direct map from Resolver's "sticky notes" to Environment's slots.
	// So, we just add it to the current environment's map.
	if (index >= (int)slots.size()) slots.resize(index + 1);
	slots[index] = value;
}


// Global variables are stored in the values map, which maps variable names to their values. 
// Local variables are stored in the slots vector, which is indexed by the "sticky notes" created by the Resolver. 
// This method is strictly for global variables, as local variables are accessed by getAt() method, which is indexed by Resolver's "sticky notes".
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

LiteralValue Environment::getAt(int depth, int index) {
	Environment* env = this;
	for (int i = 0; i < depth; i++) {
		env = env->enclosing.get();  // hop up
	}
	assert(env != nullptr);
	assert(index < (int)env->slots.size());
	return env->slots[index];  // guaranteed to be a local scope
}



// This is strictly for global variables, as local variables are assigned by assignAt() method, which is indexed by Resolver's "sticky notes". This method looks up by variable name, so it is for global variables.
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

// Same here, this is strictly for local variables, as global variables are assigned by assign() method, which looks up by variable name. This method is indexed by Resolver's "sticky notes", so it is for local variables.
void Environment::assignAt(int depth, int index, LiteralValue value) {
	Environment* env = this;
	for (int i = 0; i < depth; i++) {
		// That .get() is just to get the raw pointer from the shared_ptr, as we are not owning it, just observing it. We are guaranteed that it will outlive us, so we don't have to worry about dangling pointers.
		env = env->enclosing.get();
	}
	assert(env != nullptr);
	assert(index < (int)env->slots.size());
	env->slots[index] = value;
}

void Environment::push_slots(LiteralValue value) {
	slots.push_back(value);
}

