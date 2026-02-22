#include "parser/Expr.h"

Binary::Binary(std::unique_ptr<Expr> left, const Token& op, std::unique_ptr<Expr> right)
	: left(std::move(left)), op(op), right(std::move(right)) {
}

std::string Binary::accept(ExprVisitor* visitor) const {
	return visitor->visitBinaryExpr(*this);
}

Grouping::Grouping(std::unique_ptr<Expr> expression) : expression(std::move(expression)) {}

std::string Grouping::accept(ExprVisitor* visitor) const {
	return visitor->visitGroupingExpr(*this);
}

Literal::Literal(const LiteralValue& value) : value(value) {}

std::string Literal::accept(ExprVisitor* visitor) const {
	return visitor->visitLiteralExpr(*this);
}

Unary::Unary(const Token& op, std::unique_ptr<Expr> right) : op(op), right(std::move(right)) {}

std::string Unary::accept(ExprVisitor* visitor) const {
	return visitor->visitUnaryExpr(*this);
}
