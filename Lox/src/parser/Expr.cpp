#include "parser/Expr.h"

Binary::Binary(std::unique_ptr<Expr> left, const Token& op, std::unique_ptr<Expr> right)
	: left(std::move(left)), op(op), right(std::move(right)) {
}

LiteralValue Binary::accept(ExprVisitor* visitor) const {
	return visitor->visitBinaryExpr(*this);
}

Grouping::Grouping(std::unique_ptr<Expr> expression) : expression(std::move(expression)) {}

LiteralValue Grouping::accept(ExprVisitor* visitor) const {
	return visitor->visitGroupingExpr(*this);
}

Literal::Literal(const LiteralValue& value) : value(value) {}

LiteralValue Literal::accept(ExprVisitor* visitor) const {
	return visitor->visitLiteralExpr(*this);
}

Unary::Unary(const Token& op, std::unique_ptr<Expr> right) : op(op), right(std::move(right)) {}

LiteralValue Unary::accept(ExprVisitor* visitor) const {
	return visitor->visitUnaryExpr(*this);
}

VariableExpr::VariableExpr(const Token& name) : name(name) {}

LiteralValue VariableExpr::accept(ExprVisitor* visitor) const {
	return visitor->visitVariableExpr(*this);
}

Assign::Assign(const Token& name, std::unique_ptr<Expr> value) : name(name), value(std::move(value)) {}

LiteralValue Assign::accept(ExprVisitor* visitor) const {
	return visitor->visitAssignExpr(*this);
}
