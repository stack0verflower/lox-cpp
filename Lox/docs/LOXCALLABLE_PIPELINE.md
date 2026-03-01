# LoxCallable — Function Call Pipeline

## Why LoxCallable?

Right now, `LiteralValue` holds:
```cpp
std::variant<std::nullptr_t, double, bool, std::string>
```

Functions need to be **values** in Lox — storable in variables, passable as arguments, returnable from other functions:

```lox
fun add(a, b) { return a + b; }
var f = add;       // function stored in a variable
f(1, 2);           // called through the variable
```

So the environment must be able to store functions alongside numbers and strings. That means adding a callable type to the variant:

```cpp
std::variant<std::nullptr_t, double, bool, std::string, std::shared_ptr<LoxCallable>>
```

`LoxCallable` is the **abstract base class** (Java: interface) that every callable thing — functions, native functions, classes — implements. It defines one contract:

```cpp
class LoxCallable {
public:
    virtual LiteralValue call(Interpreter& interpreter, std::vector<LiteralValue> arguments) = 0;
    virtual int arity() = 0;   // how many arguments expected
    virtual ~LoxCallable() = default;
};
```

---

## Why `shared_ptr` and not `unique_ptr`?

```lox
fun greet() { print "hello"; }
var a = greet;
var b = greet;   // two variables, same function object
```

`unique_ptr` means one owner — moving it would leave `a` empty. `shared_ptr` allows multiple variables to point to the same function object safely. The function lives as long as at least one variable holds it.

---

## The Full Call Pipeline

### Example Program
```lox
fun add(a, b) {
    return a + b;
}
print add(3, 5);   // 8
```

---

### Flowchart

```
SOURCE CODE: add(3, 5)
        │
        ▼
┌─────────────────┐
│     LEXER       │
│  Tokenizes into │
│  IDENTIFIER "add"│
│  LEFT_PAREN     │
│  NUMBER 3       │
│  COMMA          │
│  NUMBER 5       │
│  RIGHT_PAREN    │
└────────┬────────┘
         │
         ▼
┌─────────────────────────────────────┐
│              PARSER                 │
│                                     │
│  call()                             │
│    → primary() → VariableExpr("add")│
│    → sees (                         │
│    → finishCall(VariableExpr("add"))│
│        → parse args: [Literal(3),   │
│                       Literal(5)]   │
│        → consume )                  │
│        → return CallExpr(           │
│            callee: VariableExpr("add")│
│            args:   [Literal(3),     │
│                     Literal(5)]     │
│          )                          │
└────────────────┬────────────────────┘
                 │
                 ▼
┌─────────────────────────────────────────────┐
│           INTERPRETER                       │
│                                             │
│  visitCallExpr(CallExpr)                    │
│                                             │
│  Step 1: evaluate callee                    │
│    → visitVariableExpr("add")               │
│    → environment lookup "add"               │
│    → returns shared_ptr<LoxCallable>        │
│      (the LoxFunction object)               │
│                                             │
│  Step 2: evaluate arguments                 │
│    → evaluate(Literal(3)) → 3.0             │
│    → evaluate(Literal(5)) → 5.0             │
│    → arguments = [3.0, 5.0]                 │
│                                             │
│  Step 3: arity check                        │
│    → function->arity() == 2                 │
│    → arguments.size() == 2  ✅              │
│                                             │
│  Step 4: check callable                     │
│    → holds_alternative<LoxCallable>? YES ✅ │
│                                             │
│  Step 5: call                               │
│    → function->call(interpreter, [3.0, 5.0])│
└──────────────────┬──────────────────────────┘
                   │
                   ▼
┌─────────────────────────────────────────────┐
│           LoxFunction::call()               │
│                                             │
│  Step 1: create new Environment             │
│    → new Environment(closure)               │
│    → this is the function's local scope     │
│                                             │
│  Step 2: bind parameters to arguments       │
│    → environment->define("a", 3.0)          │
│    → environment->define("b", 5.0)          │
│                                             │
│  Step 3: executeBlock(body, environment)    │
│    → visits ReturnStmt                      │
│    → evaluates a + b → 8.0                  │
│    → throws ReturnException(8.0)            │
│      (return is implemented as exception    │
│       to unwind the call stack cleanly)     │
└──────────────────┬──────────────────────────┘
                   │
                   ▼
┌─────────────────────────────────────────────┐
│  visitCallExpr catches ReturnException      │
│    → extracts value 8.0                     │
│    → returns 8.0 as LiteralValue            │
└──────────────────┬──────────────────────────┘
                   │
                   ▼
┌─────────────────────────────────────────────┐
│  visitPrintStmt                             │
│    → stringify(8.0) → "8"                   │
│    → std::cout << "8"                       │
└─────────────────────────────────────────────┘

OUTPUT: 8
```

---

## Class Hierarchy

```
LoxCallable  (abstract base — the interface)
    │
    ├── LoxFunction      (user-defined functions: fun add(a,b) { ... })
    │
    └── LoxNativeFunction  (built-in C++ functions: clock(), etc.)
```

Each subclass implements:
- `call()` — the actual execution logic
- `arity()` — how many parameters it expects

---

## Why Return is an Exception

`return` can happen anywhere inside nested blocks, loops, if statements:

```lox
fun find(list, target) {
    for (var i = 0; i < len; i = i + 1) {
        if (list[i] == target) {
            return i;   // deep inside nested scopes
        }
    }
    return -1;
}
```

Using a C++ exception to carry the return value **unwinds the entire call stack** back to `visitCallExpr` in one shot — no need to thread a return flag through every loop, block, and if statement. Clean and simple.

```cpp
// Thrown by visitReturnStmt
struct ReturnException {
    LiteralValue value;
};

// Caught by LoxFunction::call()
try {
    interpreter.executeBlock(body, environment);
} catch (ReturnException& returnValue) {
    return returnValue.value;
}
```

---

## Error Cases

```lox
var x = "hello";
x(1, 2);           // runtime error: x is not callable

fun add(a, b) { return a + b; }
add(1);            // runtime error: expected 2 arguments, got 1
add(1, 2, 3);      // runtime error: expected 2 arguments, got 3
```

Both errors are caught in `visitCallExpr` before `call()` is ever invoked.

---

*Reference: Crafting Interpreters, Chapter 10 — Functions*
