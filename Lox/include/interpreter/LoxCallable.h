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
class LoxClass;

class LoxCallable {
public:
    virtual LiteralValue call(Interpreter& interpreter, std::vector<LiteralValue> arguments) = 0;
    virtual int arity() = 0;
    virtual std::string toString() = 0;
    virtual ~LoxCallable() = default;
};

class LoxFunction : public LoxCallable {
public:
    LoxFunction(const FuncStmt& declaration, std::shared_ptr<Environment> closure, bool isInitializer);
    LiteralValue call(Interpreter& interpreter, std::vector<LiteralValue> arguments) override;
	std::shared_ptr<LoxFunction> bind(std::shared_ptr<LoxInstance> instance);
    int arity() override;
    std::string toString() override;

private:
    const FuncStmt& declaration;
    std::shared_ptr<Environment> closure;
    bool isInitializer;
};

class LoxLambda : public LoxCallable {
public:
    LoxLambda(const LambdaExpr& declaration, std::shared_ptr<Environment> closure);
    LiteralValue call(Interpreter& interpreter, std::vector<LiteralValue> arguments) override;
    int arity() override;
    std::string toString() override;

private:
    const LambdaExpr& declaration;
    std::shared_ptr<Environment> closure;
};

class LoxClass : public LoxCallable {
public:
    LoxClass(const std::string& name, std::unordered_map<std::string, std::shared_ptr<LoxFunction>> methods);
    LiteralValue call(Interpreter& interpreter, std::vector<LiteralValue> arguments) override;
    int arity() override;
	std::string toString() override;

    std::shared_ptr<LoxFunction> findMethod(const std::string& name);

private:
	std::string name;
    std::unordered_map<std::string, std::shared_ptr<LoxFunction>> methods;
};

#endif // !LOXCALLABLE_H
