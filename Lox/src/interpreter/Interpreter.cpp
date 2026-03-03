#include "interpreter/Interpreter.h"
#include "interpreter/LoxCallable.h"
#include "core/Error.h"
#include "interpreter/Return.h"
#include <iostream>

Interpreter::Interpreter() {
	// This is classic, injecting into global scope.
	// This is how std libraries are injected.

	struct ClockFn : public LoxCallable {
		int arity() override { return 0; }
		LiteralValue call(Interpreter& interpreter, std::vector<LiteralValue> arguments) override {
			return (double)clock() / CLOCKS_PER_SEC;
		}
		std::string toString() { return "<native fn>"; }
	};

	globalEnv->define("clock", std::make_shared<ClockFn>());
}

void Interpreter::interpret(const std::vector<std::unique_ptr<Stmt>>& statements) {
	// here, & means pass the original object
	// Then, you treat that statements as an object (hence pass its address as *statements), just the difference is that statements is original copy
	for (auto& statement : statements) {
		execute(*statement);
	}
}

void Interpreter::visitVarDeclStmt(const VarDeclStmt& stmt) {
	LiteralValue value = nullptr;
	if (stmt.initializer) {
		value = evaluate(*stmt.initializer);
	}

	if (environment == globalEnv) {
		environment->define(stmt.name.lexeme, value);  // global — name based
	} else {
		environment->push_slots(value);           // local — index based
	}
}

void Interpreter::visitExprStmt(const ExprStmt& stmt) {
	evaluate(*stmt.expression);
}

void Interpreter::visitIfStmt(const IfStmt& stmt) {
	if (isTruthy(evaluate(*stmt.condition))) {
		execute(*stmt.thenBranch);
	} else if (stmt.elseBranch != nullptr) {
		execute(*stmt.elseBranch);
	}
}

void Interpreter::visitPrintStmt(const PrintStmt& stmt) {
	LiteralValue value = evaluate(*stmt.expression);
	std::cout << stringify(value) << std::endl;
}

void Interpreter::visitWhileStmt(const WhileStmt& stmt) {
	while (isTruthy(evaluate(*stmt.condition))) {
		execute(*stmt.body);
	}
}

void Interpreter::visitBlockStmt(const BlockStmt& stmt) {
	auto blockEnv = std::make_shared<Environment>(environment);
	executeBlock(stmt.getStatements(), blockEnv);
}

// This is parser phase. Visit here, create a defination of function in environment and move on.
// visitCallExpr is calltime phase, when function is actually invoked.
void Interpreter::visitFuncStmt(const FuncStmt& stmt) {
	// Wrap into a LoxFunction, and cast that into a shared_ptr, as env constructor takes a literal value, which has a shared_ptr<LoxCallable>
	// stmt is casted into LoxFunction, which is a LoxCallable, then a shared_ptr is made of it.

	auto function = std::make_shared<LoxFunction>(stmt, environment);
	if (environment == globalEnv) {
		environment->define(stmt.name.lexeme, function);
	} else {
		environment->push_slots(function);
	}
}

void Interpreter::visitReturnStmt(const ReturnStmt& stmt) {
	LiteralValue value = nullptr;
	if (stmt.value) value = evaluate(*stmt.value);

	throw ReturnException(value);
}

LiteralValue Interpreter::visitAssignExpr(const Assign& expr) {
	// Now, we have visited an assignment expr. Now, visit it's Expr, evaluate that. Evaluate does nothing but visit it.
	LiteralValue value = evaluate(*expr.value);

	if (locals.count(&expr)) {
		auto& res = locals.at(&expr);
		environment->assignAt(res.depth, res.index, value);
	} else {
		globalEnv->assign(expr.name, value);
	}

	return value;
}

LiteralValue Interpreter::visitBinaryExpr(const Binary& expr) {
	LiteralValue left = evaluate(*expr.left);
	LiteralValue right = evaluate(*expr.right);

	switch (expr.op.type) {
		case TokenType::MINUS:
			checkNumberOperands(expr.op, left, right);
			return std::get<double>(left) - std::get<double>(right);

		case TokenType::STAR:
			checkNumberOperands(expr.op, left, right);
			return std::get<double>(left) * std::get<double>(right);

		case TokenType::SLASH:
			checkNumberOperands(expr.op, left, right);
			return std::get<double>(left) / std::get<double>(right);

		// PLUS does two things: if both operands are numbers, it adds them. If either operand is a string, it concatenates them as strings.
		case TokenType::PLUS:
			if (std::holds_alternative<std::string>(left) && std::holds_alternative<std::string>(right)) {
				return std::get<std::string>(left) + std::get<std::string>(right);
			}

			if (std::holds_alternative<double>(left) && std::holds_alternative<double>(right)) {
				return std::get<double>(left) + std::get<double>(right);
			}

			// TODO: 2 more cases pending, both are same, either one is string. 
			throw RuntimeError(expr.op, ": Operands must be two numbers or two strings.");
		
		// TODO : Support not just number operations
		case TokenType::GREATER:
			checkNumberOperands(expr.op, left, right);
			return LiteralValue(std::get<double>(left) > std::get<double>(right));

		case TokenType::GREATER_EQUAL:
			checkNumberOperands(expr.op, left, right);
			return LiteralValue(std::get<double>(left) >= std::get<double>(right));

		case TokenType::LESS:
			checkNumberOperands(expr.op, left, right);
			return LiteralValue(std::get<double>(left) < std::get<double>(right));

		case TokenType::LESS_EQUAL:
			checkNumberOperands(expr.op, left, right);
			return LiteralValue(std::get<double>(left) <= std::get<double>(right));

		case TokenType::BANG_EQUAL:
			return !isEqual(left, right);

		case TokenType::EQUAL_EQUAL:
			return isEqual(left, right);
	}

	// Unreachable code, but we need to return something to satisfy the compiler
	return nullptr;
}

LiteralValue Interpreter::visitGroupingExpr(const Grouping& expr) {
	return evaluate(*expr.expression);
}

LiteralValue Interpreter::visitLiteralExpr(const Literal& expr) {
	return expr.value;
}

LiteralValue Interpreter::visitLogicalExpr(const Logical& expr) {
	LiteralValue left = evaluate(*expr.left);

	// Short circuit. If left expression evaluates to True, no need to check right expression for logical OR.
	if (expr.op.type == TokenType::OR) {
		// left is a LiteralValue. Our whole interpreter knows LiteralValue as the literal data type. 
		// Hence returning left instead of true makes sense. We are always performing operation on LiteralValue and not boolean themselves, or any other primitive data type as in individual themselves.
		if (isTruthy(left)) return left;	
	} else {
		// For logical AND, if left expression evaluates to False, just return false, no need to evaluate right expression
		if (!isTruthy(left)) return left;
	}

	// At this stage, we need to evaluate right expression too, and whatever it evaluates too, will be returned.
	return evaluate(*expr.right);
}

LiteralValue Interpreter::visitUnaryExpr(const Unary& expr) {
	LiteralValue right = evaluate(*expr.right);

	switch (expr.op.type) {
		case TokenType::BANG:
			return !isTruthy(right);
		case TokenType::MINUS:
			checkNumberOperand(expr.op, right);
			return std::get<double>(right) * -1;
	}

	// Unreachable code, but we need to return something to satisfy the compiler
	// Why unreachable? Because the switch statement should cover all possible cases of TokenType, and if it doesn't, it would be a logic error in the program. 
	// In a well-designed interpreter, we would likely have error handling for unsupported operators, but for the sake of this example, we assume that the input is valid and that the switch statement covers all cases.
	return nullptr;
}

LiteralValue Interpreter::visitVariableExpr(const VariableExpr& expr) {
	return lookUpVariable(expr.name, expr);
}

LiteralValue Interpreter::visitCallExpr(const CallExpr& expr) {
	LiteralValue callee = evaluate(*expr.callee);

	std::vector<LiteralValue> arguments;
	for (const auto& argument : expr.arguments) {
		arguments.push_back(evaluate(*argument));
	}

	// This checks if callee is a LoxCallable shared_ptr. We will spit out errors in cases like "Hello"(). Strings are not callable.
	if (!std::holds_alternative<std::shared_ptr<LoxCallable>>(callee)) {
		throw RuntimeError(expr.paren, "Can only call functions and classes.");
	}

	// std::get extracts a specific type out of a std::variant
	/*
	Say, for a function `add`
	environment lookup "add"
    → returns LiteralValue        (the generic box)
    → holds_alternative check     (peek inside — is it callable?)
    → std::get<shared_ptr<LoxCallable>>  (unpack + "typecast")
    → shared_ptr<LoxCallable>     (now you have the actual function)
    → function->call(...)         (invoke it)
	*/

	auto function = std::get<std::shared_ptr<LoxCallable>>(callee);

	// function is a ptr, use, -> to access members.
	if (arguments.size() != function->arity()) {
		throw RuntimeError(expr.paren, "Expected " + std::to_string(function->arity()) + " arguments but got " + std::to_string(arguments.size()) + ".");
	}

	return function->call(*this, arguments);
}

LiteralValue Interpreter::visitLambdaExpr(const LambdaExpr& expr) {
	return std::make_shared<LoxLambda>(expr, environment);
}


// Private helper methods

void Interpreter::execute(const Stmt& statement) {
	statement.accept(this);
}

void Interpreter::executeBlock(const std::vector<std::unique_ptr<Stmt>>& statements, std::shared_ptr<Environment> currEnv) {
	std::shared_ptr<Environment> previous = environment;  // Save the current environment
	environment = currEnv;  // Switch to the new environment
	try {
		for (const auto& statement : statements) {
			execute(*statement);
		}
	} catch (...) {
		/*
		 * catch (...) — The Cleanup Catch
		 *
		 * This catch block is NOT here to handle any exception.
		 * It is here for ONE reason only — guaranteed cleanup.
		 *
		 * catch (...) means "catch absolutely anything":
		 *   - RuntimeError    (type mismatch, undefined variable)
		 *   - ParseError      (shouldn't reach here, but just in case)
		 *   - std::exception  (any standard exception)
		 *   - anything else   (unknown throws)
		 *
		 * We don't know what was thrown, we don't care.
		 * We just need to restore the environment before it propagates.
		 *
		 * environment = previous;
		 *   Restore the environment to what it was before this block.
		 *   Without this, a RuntimeError mid-block would leave the
		 *   interpreter stuck in the wrong scope forever.
		 *
		 * throw;
		 *   Re-throw the EXACT same exception that was caught.
		 *   No new exception, no modification, no swallowing.
		 *   Just "put it back on the stack and let it go."
		 *   Whoever called executeBlock() will still see the original
		 *   exception as if this catch never existed — just with a
		 *   clean environment now.
		 *
		 * Think of it like:
		 *   "I don't know what went wrong, and it's not my job to fix it.
		 *    But before I pass the problem up, let me clean up my mess."
		 * 
		 * Block4 throws
			→ Block3 restores → rethrows
				→ Block2 restores → rethrows
					→ Block1 restores → rethrows
						→ Lox.cpp catches → reports error

		 * Each layer cleans up its own mess before passing the problem up. 
		 * By the time it reaches Lox.cpp the entire environment chain is clean regardless of how deeply nested the error occurred.
		 */

		environment = previous;  // Restore the previous environment on error
		throw;  // Re-throw the exception
	}
	environment = previous;  // Restore the previous environment after block execution
}

void Interpreter::resolve(const Expr& expr, int depth, int index) {
	locals[&expr] = { depth, index };
}

LiteralValue Interpreter::lookUpVariable(const Token& name, const Expr& expr) {
	if (locals.count(&expr)) {
		// Get that struct.
		Resolution res = locals[&expr];
		return environment->getAt(res.depth, res.index);
	} else {
		return globalEnv->get(name);
	}
}

LiteralValue Interpreter::evaluate(const Expr& expr) {
	return expr.accept(this);
}

void Interpreter::checkNumberOperand(const Token& op, const LiteralValue& operand) {
	if (!std::holds_alternative<double>(operand)) {
		throw RuntimeError(op, ": Unary Operand must be a number.");
	}
}

void Interpreter::checkNumberOperands(const Token& op, const LiteralValue& left, const LiteralValue& right) {
	if (!std::holds_alternative<double>(left) || !std::holds_alternative<double>(right)) {
		throw RuntimeError(op, ": Operands must be numbers.");
	}
}

std::string Interpreter::stringify(LiteralValue value) {
	if (std::holds_alternative<std::nullptr_t>(value)) return "nil";
	if (std::holds_alternative<double>(value)) {
		std::string text = std::to_string(std::get<double>(value));

		// C++ std::to_string often adds many trailing zeros (e.g., 5.000000)
		// This logic trims them and the decimal point if it's a whole number
		if (text.find('.') != std::string::npos) {
			text.erase(text.find_last_not_of('0') + 1, std::string::npos);
			if (text.back() == '.') text.pop_back();
		}
		return text;
	}
	if (std::holds_alternative<bool>(value)) return std::get<bool>(value) ? "true" : "false";
	return std::get<std::string>(value);
}

bool Interpreter::isTruthy(LiteralValue value) {
	if (std::holds_alternative<std::nullptr_t>(value)) return false;
	if (std::holds_alternative<bool>(value)) return std::get<bool>(value);
	return true;
}

bool Interpreter::isEqual(LiteralValue a, LiteralValue b) {
	if (std::holds_alternative<std::nullptr_t>(a) && std::holds_alternative<std::nullptr_t>(b)) return true;
	if (std::holds_alternative<std::nullptr_t>(a) || std::holds_alternative<std::nullptr_t>(b)) return false;
	return std::equal_to<LiteralValue>()(a, b);
}
