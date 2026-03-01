#ifndef LOXCALLABLE_H
#define LOXCALLABLE_H

#include "core/Common.h"
#include <vector>
#include "parser/Stmt.h"
#include "interpreter/Environment.h"
#include "parser/Expr.h"


// Why this? Because call requires an interpreter object. 
// We need this file in Interpreter.h . If we include Interpreter.h here, circular dependency. Forward declare
class Interpreter;
class LoxCallable;
class LoxLambda;

class LoxCallable {
public:
    virtual LiteralValue call(Interpreter& interpreter, std::vector<LiteralValue> arguments) = 0;
    virtual int arity() = 0;
    virtual std::string toString() = 0;
    virtual ~LoxCallable() = default;
};

class LoxFunction : public LoxCallable {
public:
    LoxFunction(const FuncStmt& declaration, Environment* closure);
    LiteralValue call(Interpreter& interpreter, std::vector<LiteralValue> arguments) override;
    int arity() override;
    std::string toString() override;

private:
    const FuncStmt& declaration;
    Environment* closure;
};

class LoxLambda : public LoxCallable {
public:
    LoxLambda(const LambdaExpr& declaration, Environment* closure);
    LiteralValue call(Interpreter& interpreter, std::vector<LiteralValue> arguments) override;
    int arity() override;
    std::string toString() override;

private:
    const LambdaExpr& declaration;
    Environment* closure;
};

#endif // !LOXCALLABLE_H
