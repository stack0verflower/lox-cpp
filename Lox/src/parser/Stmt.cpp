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

