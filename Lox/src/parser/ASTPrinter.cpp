#include "parser/ASTPrinter.h"

std::string ASTPrinter::literalValueToString(const LiteralValue& value) {
    if (std::holds_alternative<std::nullptr_t>(value)) {
        return "nil";
    }
    if (std::holds_alternative<double>(value)) {
        return std::to_string(std::get<double>(value));
    }
    if (std::holds_alternative<bool>(value)) {
        return std::get<bool>(value) ? "true" : "false";
    }
    if (std::holds_alternative<std::string>(value)) {
        return std::get<std::string>(value);
    }
    return "unknown";
}

LiteralValue ASTPrinter::print(const Expr& expr) {
    return expr.accept(this);
}

LiteralValue ASTPrinter::visitBinaryExpr(const Binary& expr) {
	return parenthesize(expr.op.lexeme, { expr.left.get(), expr.right.get()});
}

LiteralValue ASTPrinter::visitGroupingExpr(const Grouping& expr) {
	return parenthesize("group", { expr.expression.get()});
}

LiteralValue ASTPrinter::visitLiteralExpr(const Literal& expr) {
	return literalValueToString(expr.value);
}

LiteralValue ASTPrinter::visitUnaryExpr(const Unary& expr) {
    return parenthesize(expr.op.lexeme, { expr.right.get() });
}

std::string ASTPrinter::parenthesize(const std::string& name, std::initializer_list<const Expr*> exprs) {
    std::string builder = "(" + name;
    for (const Expr* expr : exprs) {
        builder += " ";
        // This triggers the next level of the tree
        builder += literalValueToString(expr->accept(this));
    }
    builder += ")";
    return builder;
}
