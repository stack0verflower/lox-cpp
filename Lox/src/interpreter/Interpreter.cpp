#include "interpreter/Interpreter.h"
#include "core/Error.h"

LiteralValue Interpreter::interpret(const Expr& expression) {
	return evaluate(expression);
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


// Private helper methods

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
