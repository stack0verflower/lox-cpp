#include "parser/Expr.h"
#include "parser/Stmt.h"

Binary::Binary(std::unique_ptr<Expr> left, const Token& op, std::unique_ptr<Expr> right)
	: left(std::move(left)), op(op), right(std::move(right)) {}

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

Logical::Logical(std::unique_ptr<Expr> left, Token op, std::unique_ptr<Expr> right) 
	: left(std::move(left)), op(op), right(std::move(right)) {}

LiteralValue Logical::accept(ExprVisitor* visitor) const {
	return visitor->visitLogicalExpr(*this);
}

CallExpr::CallExpr(std::unique_ptr<Expr> callee, Token paren, std::vector<std::unique_ptr<Expr>> arguments) 
	: callee(std::move(callee)), paren(paren), arguments(std::move(arguments)) {}

LiteralValue CallExpr::accept(ExprVisitor* visitor) const {
	return visitor->visitCallExpr(*this);
}

LambdaExpr::LambdaExpr(std::vector<Token> params, std::vector<std::unique_ptr<Stmt>> body) : params(std::move(params)), body(std::move(body)) {}

LiteralValue LambdaExpr::accept(ExprVisitor* visitor) const {
	return visitor->visitLambdaExpr(*this);
}

GetExpr::GetExpr(std::unique_ptr<Expr> object, Token name) : object(std::move(object)), name(name) {}

LiteralValue GetExpr::accept(ExprVisitor* visitor) const {
	return visitor->visitGetExpr(*this);
}

SetExpr::SetExpr(std::unique_ptr<Expr> object, Token name, std::unique_ptr<Expr> value) : object(std::move(object)), name(name), value(std::move(value)) {}

LiteralValue SetExpr::accept(ExprVisitor* visitor) const {
	return visitor->visitSetExpr(*this);
}

ThisExpr::ThisExpr(Token keyword) : keyword(keyword) {}

LiteralValue ThisExpr::accept(ExprVisitor* visitor) const {
	return visitor->visitThisExpr(*this);
}
