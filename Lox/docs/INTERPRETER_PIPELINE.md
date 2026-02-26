# Lox Interpreter Pipeline

> A walkthrough of how source code flows through the Lox interpreter, from raw text to execution.

---

## Overview

```
Source Code (string)
      │
      ▼
   Lexer          → Tokens
      │
      ▼
   Parser         → AST (Statements + Expressions)
      │
      ▼
  Interpreter     → Result / Side Effects
```

Each phase is **blind to the ones above it**. The Lexer doesn't know about the Parser. The Parser doesn't know about the Interpreter. This is intentional — it keeps each layer independently testable and maintainable.

---

## Phase 1 — Lexing (Lexer.cpp)

**Input:** Raw source string
**Output:** `std::vector<Token>`

The Lexer scans the source character by character and groups them into **tokens** — the smallest meaningful units of the language.

```
"var a = 10;"
      │
      ▼
[ VAR, IDENTIFIER("a"), EQUAL, NUMBER(10), SEMICOLON, EOF ]
```

Each `Token` carries:
- `type` — what kind of token it is (`VAR`, `IDENTIFIER`, `NUMBER`, etc.)
- `lexeme` — the raw string from source (`"var"`, `"a"`, `"10"`)
- `literal` — the parsed value for literals (`10.0` for numbers, `"hello"` for strings)
- `line` — line number for error reporting

**Errors thrown:** `LexError` — unterminated strings, unexpected characters, etc.

---

## Phase 2 — Parsing (Parser.cpp)

**Input:** `std::vector<Token>`
**Output:** `std::vector<std::unique_ptr<Stmt>>`

The Parser consumes tokens and builds an **Abstract Syntax Tree (AST)**. The tree has two node hierarchies:

### Statement Nodes (Stmt.h)
Statements are things that *do* something — they don't return a value.

```
Stmt
├── ExprStmt       — an expression used as a statement (e.g. "1 + 2;")
├── PrintStmt      — print an expression ("print 1 + 2;")
└── VarDeclStmt    — declare a variable ("var a = 10;")
```

### Expression Nodes (Expr.h)
Expressions are things that *produce* a value.

```
Expr
├── Literal        — a raw value (10, "hello", true, nil)
├── Binary         — two operands and an operator (1 + 2, a == b)
├── Unary          — one operand and an operator (-1, !true)
├── Grouping       — parenthesized expression ((1 + 2))
└── VariableExpr   — a variable reference (a, name)
```

### Parse Hierarchy (Operator Precedence)
The parser uses **recursive descent** — each level of precedence is a separate function, calling the next level down:

```
declaration()        ← var declarations, falls through to statement()
    statement()      ← print, falls through to expressionStatement()
        expression()
            equality()        != ==
                comparison()  > >= < <=
                    term()    + -
                        factor()  * /
                            unary()   ! -
                                primary()  literals, grouping, variables
```

Lower in the call chain = higher precedence. `primary()` binds tightest.

### Token Consumption
The parser moves through tokens using:
- `peek()` — look at current token without consuming
- `advance()` — consume and return current token
- `match()` — consume if token matches, return bool
- `consume()` — consume expected token or throw `ParseError`

**Critical rule:** Every token that is part of a construct must be explicitly consumed. A missing `consume(SEMICOLON)` means the `;` is left in the stream and causes the next parse cycle to fail.

**Errors thrown:** `ParseError` — unexpected tokens, missing semicolons, etc.

### Error Recovery — `synchronize()`
When a `ParseError` is caught in `parse()`, `synchronize()` is called to skip tokens until a **statement boundary** is found (a `;` or a keyword like `if`, `while`, `var`). This prevents **cascading errors** — false errors caused by the parser being in a confused state after the first real error.

---

## Phase 3 — Interpretation (Interpreter.cpp)

**Input:** `std::vector<std::unique_ptr<Stmt>>`
**Output:** Side effects (print output, variable storage) and evaluated values

The Interpreter walks the AST using the **Visitor pattern** and evaluates each node.

### Visitor Pattern
Each AST node has an `accept(visitor)` method that calls back into the visitor:

```cpp
// Node just dispatches — no logic here
LiteralValue Binary::accept(ExprVisitor* visitor) {
    return visitor->visitBinaryExpr(*this);
}

// All logic lives in the Interpreter
LiteralValue Interpreter::visitBinaryExpr(const Binary& expr) {
    LiteralValue left = evaluate(*expr.left);
    LiteralValue right = evaluate(*expr.right);
    // ... perform operation
}
```

This keeps AST nodes dumb — they only know their structure, not what to do with it.

### Statement Execution
```
interpret(statements)
    → execute(stmt)          for each statement
        → stmt.accept(this)
            → visitExprStmt / visitPrintStmt / visitVarDeclStmt
```

### Expression Evaluation
```
evaluate(expr)
    → expr.accept(this)
        → visitBinaryExpr / visitLiteralExpr / visitUnaryExpr / visitVariableExpr / visitGroupingExpr
```

### Environment
The `Environment` class is a wrapper around `std::unordered_map<std::string, LiteralValue>`. It stores variable name → value mappings.

```
var a = 10;
    → visitVarDeclStmt()
        → evaluate(initializer)     // 10.0
        → environment.define("a", 10.0)

print a;
    → visitPrintStmt()
        → evaluate(VariableExpr)
            → visitVariableExpr()
                → environment.get("a")  // returns 10.0
        → stringify(10.0) → "10"
        → std::cout << "10"
```

**Errors thrown:** `RuntimeError` — undefined variables, type mismatches, divide by zero, etc.

---

## Error Hierarchy

```
LoxError (base)
├── LexError      — tokenization phase
├── ParseError    — parsing phase
└── RuntimeError  — interpretation phase
```

Each error type is caught separately in `Lox.cpp::run()`, allowing distinct error messages for compiler vs runtime errors — mirroring how real compilers like GCC and Clang categorize errors.

---

## Layer Isolation Rule

```
Lox.cpp          ← knows everything, orchestrates the pipeline
Interpreter      ← knows Expr, Stmt, Token, Environment, Common
Parser           ← knows Expr, Stmt, Token, Common
Lexer            ← knows Token, Common
Common.h         ← knows nothing, base of everything
```

**Lower layers must never include higher layers.** `Common.h` must not include `Interpreter.h`. `Parser` must be blind to `Interpreter`. Dependencies flow downward only.

---

*Part of the Lox C++ interpreter — a personal implementation of [Crafting Interpreters](https://craftinginterpreters.com/) by Bob Nystrom.*
