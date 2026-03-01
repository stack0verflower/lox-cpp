/*
program        → statement* EOF ;

declaration    → funDecl 
			   | varDecl
			   | statement ;

This is not a precedence rule. We are just checking if a statement begins with what keyword and essigning them that.
statement      → exprStmt
			   | forStmt
			   | ifStmt
               | printStmt
			   | returnStmt
			   | whileStmt
			   | block ;


exprStmt       → expression ";" ;
ifStmt         → "if" "(" expression ")" statement
			   ( "else" statement )? ;
printStmt      → "print" expression ";" ;
whileStmt      → "while" "(" expression ")" statement ;

forStmt        → "for" "(" ( varDecl | exprStmt | ";" )
				 expression? ";"
				 expression? ")" statement ;

This varDecl has this syntax:
varDecl        → "var" IDENTIFIER ( "=" expression )? ";" ;

Block has this syntax.
block          → "{" declaration* "}" ;

funDecl        → "fun" function ;
function       → IDENTIFIER "(" parameters? ")" block ;
parameters     → IDENTIFIER ( "," IDENTIFIER )* ;

returnStmt     → "return" expression? ";" ;
*/

/*
There is no place in the grammar where both an expression and a statement are allowed. 
The operands of, say, + are always expressions, never statements. The body of a while loop is always a statement.
*/

#ifndef STMT_H
#define STMT_H

#include <memory>
#include <vector>
#include "parser/Expr.h"

class Stmt;

class VarDeclStmt;
class ExprStmt;
class IfStmt;
class PrintStmt;
class BlockStmt;
class WhileStmt;
class FuncStmt;
class ReturnStmt;

class StmtVisitor {
public:
	virtual ~StmtVisitor() = default;

	// One visit method for EACH statement type
	virtual void visitVarDeclStmt(const VarDeclStmt& stmt) = 0;

	virtual void visitExprStmt(const ExprStmt& stmt) = 0;
	virtual void visitIfStmt(const IfStmt& stmt) = 0;
	virtual void visitPrintStmt(const PrintStmt& stmt) = 0;

	virtual void visitBlockStmt(const BlockStmt& stmt) = 0;
	virtual void visitWhileStmt(const WhileStmt& stmt) = 0;
	
	virtual void visitFuncStmt(const FuncStmt& stmt) = 0;
	virtual void visitReturnStmt(const ReturnStmt& stmt) = 0;
};

class Stmt {
public:
	virtual ~Stmt() = default;

	virtual void accept(StmtVisitor* visitor) const = 0;
};

// Declaration of statement classes. Now, what do you mean by a declarative statement?
// A declaration is a statement that declares a variable, function, class, etc. It introduces a new name into the program and associates it with some value or behavior. 
// For example, in Lox, we have variable declarations like "var x = 5;", function declarations like "fun add(a, b) { return a + b; }", and class declarations like "class Foo { ... }". These are all examples of declarations because they declare new entities in the program.

// Core Idea, it is a statement, has a StmtVisitor, and it's data members includes a Token (literal value) and a unique pointer to an expression (the initializer). 
// The constructor takes a Token and a unique pointer to an expression, and initializes the data members. The accept method calls the visitVarStmt method of the visitor, passing itself as an argument. This allows the visitor to access the data members of the VarStmt and perform operations on it, such as interpreting it or printing it.
class VarDeclStmt : public Stmt {
public:
	Token name;
	std::unique_ptr<Expr> initializer;
	VarDeclStmt(const Token& name, std::unique_ptr<Expr> initializer);
	void accept(StmtVisitor* visitor) const override;
};


class ExprStmt : public Stmt {
	// Expr statement is a statement that consists of a single expression followed by a semicolon.
	// We built an expression tree for the expression. The root of that tree will be stored as a unique pointer in the ExprStmt class.
	// ExprStmt belongs to main AST tree, and the expression belongs to the subtree of the main AST tree. The expression is owned by the ExprStmt, and it should be automatically destroyed when the ExprStmt is destroyed.
public:
	std::unique_ptr<Expr> expression;
	explicit ExprStmt(std::unique_ptr<Expr> expression);

	void accept(StmtVisitor* visitor) const override;
};

class IfStmt : public Stmt {
public:
	std::unique_ptr<Expr> condition;
	/*
	thenBranch houses what needs to be done. Now, if it is a block of Stmt, already being parsed under BlockStmt, so a pointer to that statement type.
	If we have only single statement, like if(this) print "Hello";, then a pointer to single statement.
	*/

	std::unique_ptr<Stmt> thenBranch;
	std::unique_ptr<Stmt> elseBranch;

	explicit IfStmt(std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> thenBranch, std::unique_ptr<Stmt> elseBranch);
	void accept(StmtVisitor* visitor) const override;
};

class PrintStmt : public Stmt {
public:
	std::unique_ptr<Expr> expression;
	explicit PrintStmt(std::unique_ptr<Expr> expression);

	void accept(StmtVisitor* visitor) const override;
};

class BlockStmt : public Stmt {
private:
	std::vector<std::unique_ptr<Stmt>> statements;

public:
	explicit BlockStmt(std::vector<std::unique_ptr<Stmt>> statements);
	const std::vector<std::unique_ptr<Stmt>>& getStatements() const;

	void accept(StmtVisitor* visitor) const override;
};

class WhileStmt : public Stmt {
public:
	std::unique_ptr<Expr> condition;
	std::unique_ptr<Stmt> body;

	explicit WhileStmt(std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> body);
	void accept(StmtVisitor* visitor) const override;
};

class FuncStmt : public Stmt {
public:
	Token name;
	std::vector<Token> params;
	std::vector<std::unique_ptr<Stmt>> body;	// body is likely a block statement.

	explicit FuncStmt(Token name, std::vector<Token> params, std::vector<std::unique_ptr<Stmt>> body);
	void accept(StmtVisitor* visitor) const override;
};

class ReturnStmt : public Stmt {
public:
	Token keyword;
	std::unique_ptr<Expr> value;

	explicit ReturnStmt(Token keyword, std::unique_ptr<Expr> value);
	void accept(StmtVisitor* visitor) const override;
};

#endif // !STMT_H

