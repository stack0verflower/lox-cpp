#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "parser/Expr.h"
#include <stdexcept>

// TODO: Remove this afterwards
#include <iostream>
#include "core/Lox.h"

class RuntimeError;
class Interpreter;

class RuntimeError : public std::runtime_error {
public:
	const Token token;
	RuntimeError(const Token& token, const std::string& message);
};

class Interpreter : public ExprVisitor {
public:
	void interpret(const Expr& expression);

	LiteralValue visitBinaryExpr(const Binary& expr) override;
	LiteralValue visitGroupingExpr(const Grouping& expr) override;
	LiteralValue visitLiteralExpr(const Literal& expr) override;
	LiteralValue visitUnaryExpr(const Unary& expr) override;

private:
	LiteralValue evaluate(const Expr& expr);

	void checkNumberOperand(const Token& op, const LiteralValue& operand);
	void checkNumberOperands(const Token& op, const LiteralValue& left, const LiteralValue& right);

	std::string stringify(LiteralValue value);

	bool isTruthy(LiteralValue value);
	bool isEqual(LiteralValue a, LiteralValue b);
};

#endif // !INTERPRETER_H
