#ifndef RESOLVER_H
#define RESOLVER_H

#include "parser/Expr.h"
#include "parser/Stmt.h"
#include "core/Error.h"
#include <unordered_set>
#include <unordered_map>

class Interpreter;
class Resolver;

enum class FunctionType {
	NONE,
	FUNCTION,
	LAMBDA,
};

class Resolver : public ExprVisitor, public StmtVisitor {
private:
	Interpreter& interpreter;
	std::vector<std::unordered_map<std::string, int>> scopes;
	std::unordered_set<std::string> globalVariables;
	std::vector<ResolverWarning> warnings;

	void beginScope();
	void endScope();
	void declare(const Token& name);
	void resolve(const Stmt& stmt);
	void resolve(const Expr& expr);
	void define(const Token& name);
	void resolveLocal(const Expr& expr, Token name);
	void resolveFunction(const std::vector<Token>& params, const std::vector<std::unique_ptr<Stmt>>& body, FunctionType type);

	FunctionType currentFunction = FunctionType::NONE;

public:
	Resolver(Interpreter& interpreter) : interpreter(interpreter) {}

	// StmtVisitor overrides
	void visitBlockStmt(const BlockStmt& stmt) override;
	void visitVarDeclStmt(const VarDeclStmt& stmt) override;
	void visitFuncStmt(const FuncStmt& stmt) override;
	void visitExprStmt(const ExprStmt& stmt) override;
	void visitIfStmt(const IfStmt& stmt) override;
	void visitPrintStmt(const PrintStmt& stmt) override;
	void visitReturnStmt(const ReturnStmt& stmt) override;
	void visitWhileStmt(const WhileStmt& stmt) override;

	// ExprVisitor overrides
	LiteralValue visitVariableExpr(const VariableExpr& expr) override;
	LiteralValue visitAssignExpr(const Assign& expr) override;
	LiteralValue visitBinaryExpr(const Binary& expr) override;
	LiteralValue visitCallExpr(const CallExpr& expr) override;
	LiteralValue visitGroupingExpr(const Grouping& expr) override;
	LiteralValue visitLiteralExpr(const Literal& expr) override;
	LiteralValue visitLogicalExpr(const Logical& expr) override;
	LiteralValue visitUnaryExpr(const Unary& expr) override;
	LiteralValue visitLambdaExpr(const LambdaExpr& expr) override;

	void resolve(const std::vector<std::unique_ptr<Stmt>>& statements);
	const std::vector<ResolverWarning> getWarnings() const { return warnings; }
};

#endif // !RESOLVER_H
