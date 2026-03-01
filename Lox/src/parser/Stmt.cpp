#include "parser/Stmt.h"

ExprStmt::ExprStmt(std::unique_ptr<Expr> expression) : expression(std::move(expression)) {}

void ExprStmt::accept(StmtVisitor* visitor) const {
	visitor->visitExprStmt(*this);
}

PrintStmt::PrintStmt(std::unique_ptr<Expr> expression) : expression(std::move(expression)) {}

void PrintStmt::accept(StmtVisitor* visitor) const {
	visitor->visitPrintStmt(*this);
}

VarDeclStmt::VarDeclStmt(const Token& name, std::unique_ptr<Expr> initializer) : name(name), initializer(std::move(initializer)){}

void VarDeclStmt::accept(StmtVisitor* visitor) const {
	visitor->visitVarDeclStmt(*this); 
}

BlockStmt::BlockStmt(std::vector<std::unique_ptr<Stmt>> statements) : statements(std::move(statements)) {}

const std::vector<std::unique_ptr<Stmt>>& BlockStmt::getStatements() const {
	return statements;
}

void BlockStmt::accept(StmtVisitor* visitor) const {
	visitor->visitBlockStmt(*this);
}

IfStmt::IfStmt(std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> thenBranch, std::unique_ptr<Stmt> elseBranch) 
	: condition(std::move(condition)), thenBranch(std::move(thenBranch)), elseBranch(std::move(elseBranch)) {}

void IfStmt::accept(StmtVisitor* visitor) const {
	visitor->visitIfStmt(*this);
}

WhileStmt::WhileStmt(std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> body) 
	: condition(std::move(condition)), body(std::move(body)) {}

void WhileStmt::accept(StmtVisitor* visitor) const {
	visitor->visitWhileStmt(*this);
}

FuncStmt::FuncStmt(Token name, std::vector<Token> params, std::vector<std::unique_ptr<Stmt>> body) 
	: name(name), params(std::move(params)), body(std::move(body)) {}

void FuncStmt::accept(StmtVisitor* visitor) const {
	visitor->visitFuncStmt(*this);
}

ReturnStmt::ReturnStmt(Token keyword, std::unique_ptr<Expr> value) : keyword(keyword), value(std::move(value)) {}

void ReturnStmt::accept(StmtVisitor* visitor) const {
	visitor->visitReturnStmt(*this);
}
