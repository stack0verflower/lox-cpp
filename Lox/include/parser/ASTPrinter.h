#ifndef ASTPRINTER_H
#define ASTPRINTER_H

#include "parser/Expr.h"

class ASTPrinter : public ExprVisitor {
public:
    std::string print(const Expr& expr);

    std::string visitBinaryExpr(const Binary& expr) override;
    std::string visitGroupingExpr(const Grouping& expr) override;
    std::string visitLiteralExpr(const Literal& expr) override;
    std::string visitUnaryExpr(const Unary& expr) override;

private:
    std::string parenthesize(const std::string& name, std::initializer_list<const Expr*> exprs);
};

#endif // !ASTPRINTER_H
