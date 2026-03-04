#include "interpreter/LoxInstance.h"
#include "core/Error.h"

LoxInstance::LoxInstance(std::shared_ptr<LoxClass> klass) : klass(std::move(klass)) {}

LiteralValue LoxInstance::get(Token name) {
	// Ananlogy, first search fields, then methods. So, if there is a field with the same name as a method, the field will shadow the method.
	if(fields.contains(name.lexeme)) {
		return fields[name.lexeme];
	}

	// From klass, we get method.
	// Methods are bound to class, unlike data members bound to fields, subject to each instance. Methods are unified across all instances.
	auto method = klass->findMethod(name.lexeme);
	if (method) {
		return method->bind(shared_from_this());
	}

	throw RuntimeError(name, "Undefined property '" + name.lexeme + "'.");
}

void LoxInstance::set(Token name, LiteralValue value) {
	fields[name.lexeme] = value;
}

std::string LoxInstance::toString() {
	return klass->toString() + " instance";
}
