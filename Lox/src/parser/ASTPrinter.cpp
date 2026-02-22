#include "parser/ASTPrinter.h"

std::string ASTPrinter::print(const Expr& expr) {
    return expr.accept(this);
}

std::string ASTPrinter::visitBinaryExpr(const Binary& expr) {
	return parenthesize(expr.op.lexeme, { expr.left.get(), expr.right.get()});
}

std::string ASTPrinter::visitGroupingExpr(const Grouping& expr) {
	return parenthesize("group", { expr.expression.get()});
}

std::string ASTPrinter::visitLiteralExpr(const Literal& expr) {
    // Check if the value is null.
    if (std::holds_alternative<std::nullptr_t>(expr.value)) return "nil";
    if (std::holds_alternative<double>(expr.value)) return std::to_string(std::get<double>(expr.value));
    if (std::holds_alternative<bool>(expr.value)) return std::get<bool>(expr.value) ? "true" : "false";
    return std::get<std::string>(expr.value);
}

std::string ASTPrinter::visitUnaryExpr(const Unary& expr) {
    return parenthesize(expr.op.lexeme, { expr.right.get() });
}

std::string ASTPrinter::parenthesize(const std::string& name, std::initializer_list<const Expr*> exprs) {
    std::string builder = "(" + name;
    for (const Expr* expr : exprs) {
        builder += " ";
        // This triggers the next level of the tree
        builder += expr->accept(this);
    }
    builder += ")";
    return builder;
}
