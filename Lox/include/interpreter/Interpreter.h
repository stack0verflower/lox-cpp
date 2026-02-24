#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "parser/Expr.h"
#include <stdexcept>

class Interpreter : public ExprVisitor {
public:
	LiteralValue interpret(const Expr& expression);

	LiteralValue visitBinaryExpr(const Binary& expr) override;
	LiteralValue visitGroupingExpr(const Grouping& expr) override;
	LiteralValue visitLiteralExpr(const Literal& expr) override;
	LiteralValue visitUnaryExpr(const Unary& expr) override;

	// TODO: privatise this later
	std::string stringify(LiteralValue value);

private:
	LiteralValue evaluate(const Expr& expr);

	void checkNumberOperand(const Token& op, const LiteralValue& operand);
	void checkNumberOperands(const Token& op, const LiteralValue& left, const LiteralValue& right);


	bool isTruthy(LiteralValue value);
	bool isEqual(LiteralValue a, LiteralValue b);
};

#endif // !INTERPRETER_H
