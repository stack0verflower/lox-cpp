#ifndef EXPR_H
#define EXPR_H

#include <memory>
#include "core/Token.h"
#include "core/Common.h"

class ExprVisitor;
class Expr;
class Assign;
class Binary;
class Grouping;
class Literal;
class Unary;
class VariableExpr;

class ExprVisitor {
public:
	virtual ~ExprVisitor() = default;

	// One visit method for EACH expression type
	virtual LiteralValue visitAssignExpr(const Assign& expr) = 0;
	virtual LiteralValue visitBinaryExpr(const Binary& expr) = 0;
	virtual LiteralValue visitGroupingExpr(const Grouping& expr) = 0;
	virtual LiteralValue visitLiteralExpr(const Literal& expr) = 0;
	virtual LiteralValue visitUnaryExpr(const Unary& expr) = 0;
	virtual LiteralValue visitVariableExpr(const VariableExpr& expr) = 0;
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

	virtual LiteralValue accept(ExprVisitor* visitor) const = 0;
};


class Assign : public Expr {
public:
	const Token name;
	std::unique_ptr<Expr> value;

	Assign(const Token& name, std::unique_ptr<Expr> value);

	/*
	We return the value because assignment in Lox is an expression, not just a statement. That means it can be used in places like:
	loxvar a = 0;
	var b = 0;
	a = b = 10;  // chained assignment
	print a;     // 10, The assignment itself produces a value that can be passed to print.
	*/

	LiteralValue accept(ExprVisitor* visitor) const override;
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

	LiteralValue accept(ExprVisitor* visitor) const override;
};

class Grouping : public Expr {
public:
	std::unique_ptr<Expr> expression;
	Grouping(std::unique_ptr<Expr> expression);

	LiteralValue accept(ExprVisitor* visitor) const override;
};

class Literal : public Expr {
public:
	LiteralValue value;
	Literal(const LiteralValue& value);

	LiteralValue accept(ExprVisitor* visitor) const override;
};

class Unary : public Expr {
public:
	Token op;
	std::unique_ptr<Expr> right;
	Unary(const Token& op, std::unique_ptr<Expr> right);

	LiteralValue accept(ExprVisitor* visitor) const override;
};

/*
In declarative statements, say for variable declaration, we have
var a = 5 + 3;
So, later, when we are evaluating something like 4 + 5 - a -> This a is the new node type VariableExpr, which is a leaf node, referencing to the value of a
*/
class VariableExpr : public Expr {
public:
	Token name;
	VariableExpr(const Token& name);

	LiteralValue accept(ExprVisitor* visitor) const override;
};

#endif