/*
program        → statement* EOF ;

statement      → exprStmt
               | printStmt ;

exprStmt       → expression ";" ;
printStmt      → "print" expression ";" ;
*/

/*
There is no place in the grammar where both an expression and a statement are allowed. 
The operands of, say, + are always expressions, never statements. The body of a while loop is always a statement.
*/

#ifndef STMT_H
#define STMT_H

#include <memory>
#include "parser/Expr.h"

class Stmt;
class ExprStmt;
class PrintStmt;

class Stmt {
public:
	virtual ~Stmt() = default;
};

class ExprStmt : public Stmt {
	// Expr statement is a statement that consists of a single expression followed by a semicolon.
	// We built an expression tree for the expression. The root of that tree will be stored as a unique pointer in the ExprStmt class.
	// ExprStmt belongs to main AST tree, and the expression belongs to the subtree of the main AST tree. The expression is owned by the ExprStmt, and it should be automatically destroyed when the ExprStmt is destroyed.
public:
	std::unique_ptr<Expr> expression;
	explicit ExprStmt(std::unique_ptr<Expr> expression);
};

class PrintStmt : public Stmt {
public:
	std::unique_ptr<Expr> expression;
	explicit PrintStmt(std::unique_ptr<Expr> expression);
};

#endif // !STMT_H

