#ifndef ASTPRINTER_H
#define ASTPRINTER_H

#include "parser/Expr.h"

class ASTPrinter : public ExprVisitor {
public:
    LiteralValue print(const Expr& expr);
    std::string literalValueToString(const LiteralValue& value);

    LiteralValue visitBinaryExpr(const Binary& expr) override;
    LiteralValue visitGroupingExpr(const Grouping& expr) override;
    LiteralValue visitLiteralExpr(const Literal& expr) override;
    LiteralValue visitUnaryExpr(const Unary& expr) override;

private:
    std::string parenthesize(const std::string& name, std::initializer_list<const Expr*> exprs);
};

#endif // !ASTPRINTER_H
