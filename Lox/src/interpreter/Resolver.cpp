#include "interpreter/Resolver.h"
#include "interpreter/Interpreter.h"
#include "core/Error.h"

// Helper methods
void Resolver::beginScope() {
	// Create a new `notebook`
	scopes.push_back({});
}

void Resolver::endScope() {
	// Throw away the top/current `notebook`. It's use is over, no need to keep in env chain.
	scopes.pop_back();
}

void Resolver::resolve(const Stmt& stmt) {
	// Note, this is an instance of Resolver class. 
	// The statements undergo method call as, say, visitor->visitBlockStmt(const BlockStmt& stmt) override; 
	// Here, visitor is an instance of Resolver class, and visitBlockStmt is a overriden method of Resolver class, not Interpreter.
	stmt.accept(this);
}

void Resolver::resolve(const Expr& expr) {
	expr.accept(this);
}

void Resolver::resolve(const std::vector<std::unique_ptr<Stmt>>& statements) {
	for (const auto& stmt : statements) {
		resolve(*stmt);
	}
}

void Resolver::declare(const Token& name) {
	// global — push back in globalVariables, why? So we can spit warning that variable already declared in global scope.
	if (scopes.empty()) { 
		if (globalVariables.count(name.lexeme)) {
			throw ResolverError(name, "Variable '" + name.lexeme + "' already declared in global scope.");
		}
		globalVariables.insert(name.lexeme); 
		return;  
	}  

	// Now, before declaring a new variable, check if it already exists, in outer scopes too.
	// Why check exists first? Because we want to allow shadowing, but not redeclaration. 
	// So, if the variable already exists in any outer scope, we should throw an error. 
	// If we check after declaring, then we would have already declared the variable in the current scope, and we would not be able to detect redeclaration.
	
	// same scope redeclaration — hard error
	if (scopes.back().count(name.lexeme)) {
		throw ResolverError(name, "Variable '" + name.lexeme + "' already declared in this scope.");
	}

	// If not, then throw a warning for preeceding scopes.
	for (int i = scopes.size() - 1; i >= 0; i--) {
		if (scopes[i].count(name.lexeme)) {
			warnings.push_back(ResolverWarning(name, "Variable '" + name.lexeme + "' shadows local at line " + std::to_string(name.line)));
			break;
		}
	}

	// If we passed that, it means that variable name is not declared in any outer scope, so we can safely declare it in the current scope.
	// What about global scope? scopes store all but global scope, hence we have that globalVariables vector to track global variables. 
	if (globalVariables.count(name.lexeme)) {
		warnings.push_back(ResolverWarning(name, "Variable '" + name.lexeme + "' shadows global variable."));
	}


	int index = scopes.back().size();
	scopes.back()[name.lexeme] = -1;	// declared, not initialized
}

void Resolver::define(const Token& name) {
	if (scopes.empty()) return;  // global — ignore
	int index = scopes.back().size() - 1;
	scopes.back()[name.lexeme] = index;	// now initialized
}

void Resolver::resolveLocal(const Expr& expr, Token name) {
	// Create those sticky notes.
	for (int i = scopes.size() - 1; i >= 0; i--) {
		if (scopes[i].count(name.lexeme)) {
			interpreter.resolve(expr, scopes.size() - 1 - i, scopes[i][name.lexeme]);
			return;
		}
	}
}

void Resolver::resolveFunction(const std::vector<Token>& params, const std::vector<std::unique_ptr<Stmt>>& body, FunctionType type) {
	FunctionType enclosing = currentFunction;  // save previous
	currentFunction = type;  // now inside function

	// Inside function, create a new scope for function body
	beginScope();

	// Push params into that scope.
	for (Token param : params) {
		declare(param);
		define(param);
	}

	// Resolve the function body in that scope. If there are nested functions, they will create their own scopes when we resolve them.
	resolve(body);
	endScope();
	currentFunction = enclosing;  // restore previous
}

// Stmt Visitors

void Resolver::visitBlockStmt(const BlockStmt& stmt) {
	beginScope();
	resolve(stmt.getStatements());
	endScope();
}

void Resolver::visitVarDeclStmt(const VarDeclStmt& stmt) {
	declare(stmt.name);
	if (stmt.initializer) {
		resolve(*stmt.initializer);
	}
	define(stmt.name);
}

void Resolver::visitFuncStmt(const FuncStmt& stmt) {
	// Why declare and define first? 
	// To allow recursion. Suppose function body contains function itself? We need to declare and define it first, so that when we resolve the body, we can find the function name in the current scope.
	declare(stmt.name);
	define(stmt.name);
	
	resolveFunction(stmt.params, stmt.body, FunctionType::FUNCTION);
}

void Resolver::visitExprStmt(const ExprStmt& stmt) {
	// Resolve the expression it contains. This is important because the expression may contain variable references that need to be resolved to their scopes.
	resolve(*stmt.expression);
}

void Resolver::visitIfStmt(const IfStmt& stmt) {
	// Resolve the condition expression first, so that any variable references in the condition are resolved to their scopes.
	resolve(*stmt.condition);
	// Resolve the then branch. This will create a new scope for the then branch, and any variable declarations in the then branch will be resolved to that scope.
	resolve(*stmt.thenBranch);
	// Resolve the else branch, if any
	if(stmt.elseBranch) resolve(*stmt.elseBranch);
}

void Resolver::visitPrintStmt(const PrintStmt& stmt) {
	resolve(*stmt.expression);
}

void Resolver::visitReturnStmt(const ReturnStmt& stmt) {
	if (currentFunction == FunctionType::NONE) {
		throw ResolverError(stmt.keyword, "Cannot return from top-level code.");
	}

	if (stmt.value) {
		if (currentFunction == FunctionType::INITIALIZER) {
			throw ResolverError(stmt.keyword, "Can't return a value from an initializer.");
		}

		resolve(*stmt.value);
	}
}

void Resolver::visitWhileStmt(const WhileStmt& stmt) {
	// Resolve condition first, so that any variable references in the condition are resolved to their scopes.
	resolve(*stmt.condition);
	// Resolve body next, so that any variable declarations in the body are resolved to the body scope.
	resolve(*stmt.body);
}

void Resolver::visitClassStmt(const ClassStmt& stmt) {
	ClassType enclosingClass = currentClass;
	currentClass = ClassType::CLASS;

	declare(stmt.name);
	define(stmt.name);

	beginScope();
	scopes.back()["this"] = 0;  // 'this' is always at index

	for (auto& method : stmt.methods) {
		FunctionType declaration = FunctionType::METHOD;

		if (method->name.lexeme == "init") {
			declaration = FunctionType::INITIALIZER;
		}

		resolveFunction(method->params, method->body, declaration);
	}

	endScope();

	currentClass = enclosingClass;
}



// Expr Visitors

LiteralValue Resolver::visitVariableExpr(const VariableExpr& expr) {
	// In var a = a;, That RHS of `=` is an Expr, it boils down to a varExpr, as it is a variable reference. 
	// So, we are trying to resolve that variable reference, which is the same as the variable we are declaring. 
	// We want to detect this error, because it is a common mistake, and it does not make any sense to read a variable in its own initializer. 
	// So, we check if the variable exists in the current scope, and if it does, we check if it is initialized. If it is not initialized, it means that we are trying to read a variable in its own initializer, so we throw an error.
	if (!scopes.empty()
		&& scopes.back().count(expr.name.lexeme)    //  check exists first, var a = a; => first part, a is declared, but not defined, so it exists in the current scope with value -1. If we check after that, we would have already defined it, and we would not be able to detect this error.
		&& scopes.back()[expr.name.lexeme] == -1) { //  then read, that -1 tells us it was declared but not defined, so we throw an error. We are attempting to do var a = a; does this make any sense?
		throw ResolverError(expr.name, "Cannot read local variable in its own initializer.");
	}

	resolveLocal(expr, expr.name);
	return nullptr;
}

LiteralValue Resolver::visitAssignExpr(const Assign& expr) {
	resolve(*expr.value);
	resolveLocal(expr, expr.name);
	return nullptr;
}

LiteralValue Resolver::visitBinaryExpr(const Binary& expr) {
	// Binary expression itself does not introduce any new variable scopes, so we just need to resolve
	resolve(*expr.left);
	resolve(*expr.right);
	return nullptr;
}

LiteralValue Resolver::visitCallExpr(const CallExpr& expr) {
	resolve(*expr.callee);

	for (const auto& argument : expr.arguments) {
		resolve(*argument);
	}

	return nullptr;
}

LiteralValue Resolver::visitGroupingExpr(const Grouping& expr) {
	resolve(*expr.expression);
	return nullptr;
}

LiteralValue Resolver::visitLiteralExpr(const Literal& expr) {
	// Literals do not have any variable references, so there is nothing to resolve.
	return nullptr;
}

LiteralValue Resolver::visitLogicalExpr(const Logical& expr) {
	// These guys are just like binary expressions, but with short-circuiting. We still need to resolve both sides, because they may contain variable references that need to be resolved to their scopes.
	resolve(*expr.left);
	resolve(*expr.right);
	return nullptr;
}

LiteralValue Resolver::visitUnaryExpr(const Unary& expr) {
	// Unary expression itself does not introduce any new variable scopes, so we just need to resolve the operand.
	resolve(*expr.right);
	return nullptr;
}

LiteralValue Resolver::visitLambdaExpr(const LambdaExpr& expr) {
	// Lambda expressions are like anonymous functions. They introduce a new scope for their body, and they can have parameters that need to be resolved to that scope. So we create a new scope for the lambda body, declare and define the parameters in that scope, and then resolve the body in that scope.
	resolveFunction(expr.params, expr.body, FunctionType::LAMBDA);
	return nullptr;
}

LiteralValue Resolver::visitGetExpr(const GetExpr& expr) {
	/*
	"Yeah, that is what I say, Bacon is global scope, and a LiteralValue list returns a pointer to Bacon class itself. 
	Automatically you get eat, fields everything, hence we do not need to resolve .eat and all."
	*/
	resolve(*expr.object);
	return nullptr;
}

LiteralValue Resolver::visitSetExpr(const SetExpr& expr) {
	resolve(*expr.value);
	resolve(*expr.object);
	return nullptr;
}

LiteralValue Resolver::visitThisExpr(const ThisExpr& expr) {
	if (currentClass == ClassType::NONE) {
		throw ResolverError(expr.keyword, "Can't use 'this' outside of a class.");
	}

	resolveLocal(expr, expr.keyword);
	return nullptr;
}
