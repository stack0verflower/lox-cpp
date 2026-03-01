# Function Pipeline — Chapter 10

## Overview

Functions in Lox are **first-class values** — they can be stored in variables, passed as arguments, and returned from other functions. This document traces the complete pipeline from source code to execution using a concrete example.

---

## Example Program

```lox
fun add(a, b) {
    return a + b;
}

var result = add(3, 5);
print result;   // 8
```

---

## Phase 1: Parse Time — Building the Blueprint

### What happens

The parser sees `fun` and calls `funcDeclaration()`. It does NOT execute anything — it just builds an AST node that describes the function.

```
SOURCE: fun add(a, b) { return a + b; }
                │
                ▼
        funcDeclaration("function")
                │
                ├── consume IDENTIFIER → name = Token("add")
                ├── consume LEFT_PAREN
                ├── parse params → [Token("a"), Token("b")]
                ├── consume RIGHT_PAREN
                ├── consume LEFT_BRACE
                ├── parse body → [ReturnStmt(BinaryExpr(a + b))]
                │
                ▼
        FuncStmt {
            name:   Token("add")
            params: [Token("a"), Token("b")]
            body:   [ReturnStmt(Binary(Var("a"), +, Var("b")))]
        }
```

### Key point

`FuncStmt` is just a **blueprint** — a data structure. No code runs here. Think of it like a recipe card sitting on a shelf. It describes what to do but does nothing by itself.

---

## Phase 2: visitFuncStmt — Blueprint Becomes a Value

### What happens

The interpreter visits `FuncStmt` and wraps it into a `LoxFunction` object — a **runtime callable value**. This is then stored in the environment like any other variable.

```
visitFuncStmt(FuncStmt)
        │
        ├── create LoxFunction(declaration=FuncStmt, closure=currentEnvironment)
        │       LoxFunction is a runtime object — inherits LoxCallable
        │       closure captures WHERE the function was defined
        │
        ├── wrap in shared_ptr<LoxCallable>
        │
        ▼
environment->define("add", shared_ptr<LoxFunction>)

Environment now:
┌─────────────────────────────────────┐
│ "add" → shared_ptr<LoxFunction>     │  ← stored as LiteralValue
└─────────────────────────────────────┘
```

### Key point

The function is now a **value in the environment** — same as a number or string. `add` is just a name that maps to a `LoxFunction` object.

---

## Phase 3: Parse Time — The Call Expression

```
SOURCE: add(3, 5)
                │
                ▼
        call()
                │
                ├── primary() → VariableExpr("add")
                ├── sees LEFT_PAREN
                ├── finishCall(VariableExpr("add"))
                │       ├── parse args → [Literal(3), Literal(5)]
                │       ├── consume RIGHT_PAREN
                │       └── return CallExpr
                │
                ▼
        CallExpr {
            callee:    VariableExpr("add")
            paren:     Token(")")   ← for error reporting
            arguments: [Literal(3.0), Literal(5.0)]
        }
```

---

## Phase 4: visitCallExpr — The Call Machinery

```
visitCallExpr(CallExpr)
        │
        ├── Step 1: evaluate callee
        │       evaluate(VariableExpr("add"))
        │       → visitVariableExpr()
        │       → environment lookup "add"
        │       → returns LiteralValue holding shared_ptr<LoxFunction>
        │
        ├── Step 2: check callable
        │       holds_alternative<shared_ptr<LoxCallable>>(callee)?
        │       YES ✅
        │       auto function = std::get<shared_ptr<LoxCallable>>(callee)
        │       (unpack LiteralValue → actual LoxCallable pointer)
        │
        ├── Step 3: evaluate arguments (left to right)
        │       evaluate(Literal(3.0)) → 3.0
        │       evaluate(Literal(5.0)) → 5.0
        │       arguments = [3.0, 5.0]
        │
        ├── Step 4: arity check
        │       function->arity() == 2
        │       arguments.size() == 2  ✅
        │       (mismatch → RuntimeError before call ever happens)
        │
        ▼
        function->call(interpreter, [3.0, 5.0])
```

---

## Phase 5: LoxFunction::call() — Actual Execution

```
LoxFunction::call(interpreter, [3.0, 5.0])
        │
        ├── Step 1: create new Environment
        │       Environment env(closure)
        │       closure = environment where "add" was defined (global here)
        │       env is a fresh local scope, child of closure
        │
        │       Scope chain:
        │       global { add: LoxFunction }
        │           └── env { } ← function's local scope
        │
        ├── Step 2: bind parameters to arguments
        │       env.define("a", 3.0)
        │       env.define("b", 5.0)
        │
        │       Scope chain now:
        │       global { add: LoxFunction }
        │           └── env { a: 3.0, b: 5.0 }
        │
        ├── Step 3: executeBlock(body, &env)
        │       interpreter.environment = &env  ← switch to function scope
        │       execute each statement in body...
        │
        │           execute ReturnStmt(Binary(Var("a"), +, Var("b")))
        │               │
        │               ▼
        │           visitReturnStmt()
        │               │
        │               ├── evaluate(Binary(Var("a"), +, Var("b")))
        │               │       → lookup "a" → 3.0
        │               │       → lookup "b" → 5.0
        │               │       → 3.0 + 5.0 → 8.0
        │               │
        │               └── throw ReturnException(8.0)  ← not a real error!
        │                       just control flow to unwind call stack
        │
        ├── Step 4: catch ReturnException
        │       catch(ReturnException& ret)
        │           return ret.value  → 8.0
        │
        │       (interpreter.environment restored to previous scope)
        │
        ▼
        returns LiteralValue(8.0)
```

---

## Phase 6: Back in visitCallExpr

```
function->call() returned 8.0
        │
        ▼
visitCallExpr returns 8.0
        │
        ▼
var result = 8.0
environment->define("result", 8.0)
        │
        ▼
visitPrintStmt
stringify(8.0) → "8"
std::cout << "8"

OUTPUT: 8
```

---

## Complete Class Hierarchy

```
LoxCallable  (abstract base — the contract)
    │   virtual call() = 0
    │   virtual arity() = 0
    │
    ├── LoxFunction      (user-defined: fun add(a,b) { ... })
    │       stores: const FuncStmt& declaration
    │       stores: Environment* closure
    │
    └── LoxNativeFunction  (built-in C++ functions)
            example: clock() → returns system time
            defined directly in Interpreter constructor
            injected into global environment at startup
```

---

## Why Return Uses Exceptions

`return` can happen anywhere inside deeply nested blocks and loops:

```lox
fun search(n) {
    for (var i = 0; i < n; i = i + 1) {
        if (i == 5) {
            return i;   // deep inside for + if
        }
    }
    return -1;
}
```

Using a C++ exception to carry the return value **unwinds the entire call stack** in one shot — back to `LoxFunction::call()` — without threading a return flag through every loop, block, and if statement manually.

```
throw ReturnException(5)
    ← unwinds visitIfStmt
    ← unwinds visitForStmt (desugared WhileStmt)
    ← unwinds executeBlock
    ← caught by LoxFunction::call()
    → return 5
```

`ReturnException` is not an error — it's **intentional control flow** using the exception machinery as an escape hatch.

---

## Error Cases

```lox
var x = "hello";
x();              // Can only call functions and classes.

fun add(a, b) { return a + b; }
add(1);           // Expected 2 arguments but got 1.
add(1, 2, 3);     // Expected 2 arguments but got 3.
```

Both errors are caught in `visitCallExpr` **before** `call()` is ever invoked — type check first, arity check second.

---

## Known Limitation (Fixed in Chapter 11)

Closures that capture a **local scope** will segfault because the captured environment lives on the stack and is destroyed when the outer function returns:

```lox
fun makeCounter() {
    var i = 0;
    fun count() { i = i + 1; print i; }
    return count;   // count's closure points to makeCounter's stack env
}                   // makeCounter returns → stack env DESTROYED

var counter = makeCounter();
counter();  // closure points to dead memory → segfault
```

**Chapter 11 fix**: Environments become `shared_ptr` — heap allocated, reference counted. They live as long as any closure holds a reference to them, regardless of call stack lifetime.

---

*Reference: Crafting Interpreters, Chapter 10 — Functions*
