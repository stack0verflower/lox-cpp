# Assignment Pipeline — Deep Dive

## Input
```
a.b.h = 312 / 3 - 4;
```

> Note: `.` (property access) is not yet implemented in Chapter 8 — it comes in Chapter 12 (Classes).
> But this example is used to illustrate **why the lvalue trick is needed**.
> For now, Lox only supports simple variable assignment like `a = 312 / 3 - 4;`
> The pipeline below shows both cases.

---

## Case 1 — Simple: `a = 312 / 3 - 4;`

### Token Stream
```
[ IDENTIFIER("a"), EQUAL, NUMBER(312), SLASH, NUMBER(3), MINUS, NUMBER(4), SEMICOLON ]
```

### Parse Walkthrough
```
declaration()
└── statement()
    └── expressionStatement()
        ├── parseExpression()
        │   └── parseAssignment()
        │       ├── parseEquality()        ← parses left side first, no idea about "=" yet
        │       │   └── ... → parsePrimary()
        │       │               └── sees IDENTIFIER "a"
        │       │               └── returns VariableExpr("a")
        │       │
        │       ├── sees "=" → match!
        │       ├── dynamic_cast<VariableExpr*> → SUCCESS ✅
        │       ├── grabs Token name = "a"
        │       │
        │       └── parseAssignment()      ← right side, recursive call
        │           └── parseEquality()
        │               └── parseComparison()
        │                   └── parseTerm()
        │                       ├── parseFactor()
        │                       │   ├── parseUnary() → parsePrimary() → Literal(312)
        │                       │   ├── sees SLASH
        │                       │   ├── parseUnary() → parsePrimary() → Literal(3)
        │                       │   └── returns Binary(/, 312, 3)
        │                       │
        │                       ├── sees MINUS
        │                       ├── parseFactor() → Literal(4)
        │                       └── returns Binary(-, Binary(/, 312, 3), 4)
        │
        └── consume(SEMICOLON) ✅
```

### Final AST
```
ExprStmt
└── Assign
    ├── name: Token("a")
    └── value: Binary(-)
                ├── left:  Binary(/)
                │           ├── left:  Literal(312)
                │           └── right: Literal(3)
                └── right: Literal(4)
```

### Interpreter Evaluation
```
visitExprStmt()
└── evaluate(Assign)
    └── visitAssign()
        ├── evaluate(value)
        │   └── visitBinaryExpr(-)
        │       ├── evaluate(Binary(/))
        │       │   └── visitBinaryExpr(/)
        │       │       ├── evaluate(Literal(312)) → 312.0
        │       │       └── evaluate(Literal(3))   → 3.0
        │       │       └── returns 104.0
        │       └── evaluate(Literal(4)) → 4.0
        │       └── returns 100.0
        │
        └── environment.assign("a", 100.0)  ✅
```

---

## Case 2 — Complex: `a.b.h = 312 / 3 - 4;` (Chapter 12, not yet)

### Why the lvalue trick is needed here

```
Token stream:
[ IDENTIFIER("a"), DOT, IDENTIFIER("b"), DOT, IDENTIFIER("h"), EQUAL, NUMBER(312), ... ]
```

The parser has **only 1 token of lookahead**. When it sees `IDENTIFIER("a")`, it has no idea
that 5 tokens later there will be an `=`. So it just parses normally:

```
parseAssignment()
└── parseEquality()
    └── ... → parsePrimary()
        └── sees "a" → VariableExpr("a")
            └── sees "." → GetExpr("a", "b")        ← property access
                └── sees "." → GetExpr(GetExpr, "h") ← chained property access
                    └── returns GetExpr(GetExpr("a","b"), "h")
```

Now back in `parseAssignment()`:
```
← left = GetExpr(GetExpr("a","b"), "h")
← sees "=" → match!
← dynamic_cast<VariableExpr*> → FAILS ❌  (it's a GetExpr, not VariableExpr)
← BUT — Bob adds a second check:
   dynamic_cast<GetExpr*> → SUCCESS ✅
   → converts to SetExpr(object, name, value)   ← assignment to property
```

### The Key Insight
```
┌─────────────────────────────────────────────────────────┐
│  lvalue check happens AFTER parsing the left side       │
│                                                         │
│  Parse first → check type → decide assignment or error  │
│                                                         │
│  VariableExpr  → Assign node   (variable assignment)    │
│  GetExpr       → Set node      (property assignment)    │
│  anything else → ParseError    (invalid target)         │
└─────────────────────────────────────────────────────────┘
```

---

## Why Token and not Expr for lvalue?

```
Assign
├── name:  Token("a")    ← just a name, NOT evaluated
└── value: Expr          ← this IS evaluated

vs

VariableExpr
└── name: Token("a")     ← looked up in environment (evaluation)
```

When you see `a` on the LEFT of `=`:
- You don't want its VALUE (that's what VariableExpr does)
- You want its LOCATION — "where in the environment do I store this?"
- A Token (just the name string) is enough for that

```
a = 20
│   │
│   └── rvalue — evaluate this → 20.0
└────── lvalue — store into this → environment["a"] = 20.0
```

---

*Part of the Lox C++ interpreter — [Crafting Interpreters](https://craftinginterpreters.com/) by Bob Nystrom*
