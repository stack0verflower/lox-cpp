#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "parser/Expr.h"
#include "parser/Stmt.h"
#include "interpreter/Environment.h"
#include <stdexcept>
#include <vector>
#include <memory>

class Interpreter;

class Interpreter : public StmtVisitor, public ExprVisitor {
public:
	Interpreter();
	void interpret(const std::vector<std::unique_ptr<Stmt>>& statements);

	void visitVarDeclStmt(const VarDeclStmt& stmt) override;
	void visitExprStmt(const ExprStmt& stmt) override;
	void visitIfStmt(const IfStmt& stmt) override;
	void visitPrintStmt(const PrintStmt& stmt) override;
	void visitWhileStmt(const WhileStmt& stmt) override;
	void visitBlockStmt(const BlockStmt& stmt) override;
	void visitFuncStmt(const FuncStmt& stmt) override;
	void visitReturnStmt(const ReturnStmt& stmt) override;

	LiteralValue visitAssignExpr(const Assign& expr) override;
	LiteralValue visitBinaryExpr(const Binary& expr) override;
	LiteralValue visitGroupingExpr(const Grouping& expr) override;
	LiteralValue visitLiteralExpr(const Literal& expr) override;
	LiteralValue visitLogicalExpr(const Logical& expr) override;
	LiteralValue visitUnaryExpr(const Unary& expr) override;
	LiteralValue visitVariableExpr(const VariableExpr& expr) override;
	LiteralValue visitCallExpr(const CallExpr& expr) override;
	LiteralValue visitLambdaExpr(const LambdaExpr& expr) override;

	// TODO: privatise this later
	std::string stringify(LiteralValue value);

	Environment* getGlobalEnv() { return &globalEnv; }

private:
	// Create a Environment object. Then create a pointer to it.
	Environment globalEnv;                    // lives with the Interpreter
	Environment* environment = &globalEnv;    // points to current scope, no new/delete

	void execute(const Stmt& statement);

public:
	void executeBlock(const std::vector<std::unique_ptr<Stmt>>& statements, Environment* currEnv);

private:
	LiteralValue evaluate(const Expr& expr);
	void checkNumberOperand(const Token& op, const LiteralValue& operand);
	void checkNumberOperands(const Token& op, const LiteralValue& left, const LiteralValue& right);


	bool isTruthy(LiteralValue value);
	bool isEqual(LiteralValue a, LiteralValue b);
};

#endif // !INTERPRETER_H
