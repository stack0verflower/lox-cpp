#include "interpreter/Interpreter.h"
#include "core/Error.h"

#include <iostream>

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

	environment->define(stmt.name.lexeme, value);
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
	executeBlock(stmt.getStatements(), new Environment(environment));
}

LiteralValue Interpreter::visitAssignExpr(const Assign& expr) {
	// Now, we have visited an assignment expr. Now, visit it's Expr, evaluate that. Evaluate does nothing but visit it.
	LiteralValue value = evaluate(*expr.value);
	environment->assign(expr.name, value);
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
	return environment->get(expr.name);
}


// Private helper methods

void Interpreter::execute(const Stmt& statement) {
	statement.accept(this);
}

void Interpreter::executeBlock(const std::vector<std::unique_ptr<Stmt>>& statements, Environment* currEnv) {
	Environment* previous = environment;  // Save the current environment
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
	// Delete the currEnv, heap memory
	delete currEnv;
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
