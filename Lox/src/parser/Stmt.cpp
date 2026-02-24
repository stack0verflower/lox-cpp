#include "parser/Stmt.h"

ExprStmt::ExprStmt(std::unique_ptr<Expr> expression) : expression(std::move(expression)) {}

PrintStmt::PrintStmt(std::unique_ptr<Expr> expression) : expression(std::move(expression)) {}
