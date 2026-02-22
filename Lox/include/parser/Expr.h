#ifndef EXPR_H
#define EXPR_H

#include <memory>
#include <core/Token.h>
#include <variant>

using LiteralValue = std::variant<std::string, double, bool, std::nullptr_t>;

class ExprVisitor;
class Expr;
class Binary;
class Grouping;
class Literal;
class Unary;

class ExprVisitor {
public:
	virtual ~ExprVisitor() = default;

	// One visit method for EACH expression type
	virtual std::string visitBinaryExpr(const Binary& expr) = 0;
	virtual std::string visitGroupingExpr(const Grouping& expr) = 0;
	virtual std::string visitLiteralExpr(const Literal& expr) = 0;
	virtual std::string visitUnaryExpr(const Unary& expr) = 0;
};

class Expr {
public:
	/*
	What does virtual ~Expr() = default; mean in C++?
	This means, call the destructor of the derived class when deleting an object through a pointer to the base class.
	It ensures proper cleanup of resources allocated by derived classes, preventing memory leaks and undefined behavior.
	If not defined virtual, this destructor would not be called when deleting an object through a pointer to the base class, 
	leading to potential resource leaks and undefined behavior.
	*/
	virtual ~Expr() = default;

	virtual std::string accept(ExprVisitor* visitor) const = 0;
};

/*
Why unique_ptr for left and right in Binary expression?
Because each of those expressions is owned by the Binary expression, and they should be automatically destroyed when the Binary expression is destroyed.
In other words, a node is owned by its parent node, and when the parent node is destroyed, all of its child nodes should be destroyed as well.
It does not make sense for multiple nodes to share ownership of the same child node, so we use unique_ptr to enforce this ownership model.

And, we use std::move when initializing left and right in the Binary constructor to transfer ownership of the expressions to the Binary object.
Why? Because unique_ptr cannot be copied, but it can be moved.
Using generic left(left) and right(right) would attempt to copy the unique_ptr, which is not allowed and would result in a compilation error or in deletion of the pointer itself.
*/

class Binary : public Expr {
public:
	std::unique_ptr<Expr> left;
	const Token op;
	std::unique_ptr<Expr> right;

	Binary(std::unique_ptr<Expr> left, const Token& op, std::unique_ptr<Expr> right);

	std::string accept(ExprVisitor* visitor) const override;
};

class Grouping : public Expr {
public:
	std::unique_ptr<Expr> expression;
	Grouping(std::unique_ptr<Expr> expression);

	std::string accept(ExprVisitor* visitor) const override;
};

class Literal : public Expr {
public:
	LiteralValue value;
	Literal(const LiteralValue& value);

	std::string accept(ExprVisitor* visitor) const override;
};

class Unary : public Expr {
public:
	Token op;
	std::unique_ptr<Expr> right;
	Unary(const Token& op, std::unique_ptr<Expr> right);

	std::string accept(ExprVisitor* visitor) const override;
};

#endif