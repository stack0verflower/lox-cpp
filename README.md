# Lox Interpreter (C++)

A C++ implementation of the Lox programming language from [*Crafting Interpreters*](https://craftinginterpreters.com/) by Bob Nystrom.

## 📚 About

This project is my journey through *Crafting Interpreters*, translating the Java implementation (jlox) to modern C++. It's a learning project focused on understanding interpreter design, lexical analysis, parsing, and language implementation.

## 🚧 Current Status

**Completed:**
- ✅ **Chapter 4: Scanning** — Lexical analysis and tokenization
- ✅ **Chapter 5: Representing Code** — AST node definitions
- ✅ **Chapter 6: Parsing Expressions** — Recursive descent parser with full expression support
- ✅ **Chapter 7: Evaluating Expressions** — Tree-walk interpreter with runtime error handling
- ✅ **Chapter 8: Statements and State** — Variables, assignment, block scoping, lexical environment chain
- ✅ **Chapter 9: Control Flow** — `if`/`else`, `while`, `for` (desugared), logical `and`/`or` — Lox is now Turing complete
- ✅ **Chapter 10: Functions** — first-class functions, closures, return statements, native functions (`clock()`), `LoxCallable` interface, `LoxFunction` runtime object
- ✅ **Lambda Functions** *(Challenge Extension)* — anonymous function expressions, `LoxLambda` class, immediately invokable, passable as arguments

**Coming Next:**
- ⏳ Chapter 11: Resolving and Binding — fix closures with `shared_ptr` environments (fixes both `LoxFunction` and `LoxLambda`)
- ⏳ Chapter 12: Classes
- ⏳ Chapter 13: Inheritance

## 📁 Project Structure

```
Lox/
├── include/
│   ├── core/
│   │   ├── Common.h
│   │   ├── Error.h
│   │   ├── Lox.h
│   │   └── Token.h
│   ├── scanner/
│   │   └── Lexer.h
│   ├── parser/
│   │   ├── ASTPrinter.h
│   │   ├── Expr.h
│   │   ├── Parser.h
│   │   └── Stmt.h
│   └── interpreter/
│       ├── Environment.h
│       ├── Interpreter.h
│       ├── LoxCallable.h
│       └── Return.h
├── src/
│   ├── core/
│   │   └── Lox.cpp
│   ├── scanner/
│   │   └── Lexer.cpp
│   ├── parser/
│   │   ├── ASTPrinter.cpp
│   │   ├── Expr.cpp
│   │   ├── Parser.cpp
│   │   └── Stmt.cpp
│   ├── interpreter/
│   │   ├── Environment.cpp
│   │   ├── Interpreter.cpp
│   │   └── LoxCallable.cpp
│   └── Main.cpp
├── docs/
│   ├── ARCHITECTURE_NOTES.md
│   ├── ASSIGNMENT_PIPELINE.md
│   ├── FILE_STRUCTURE.txt
│   ├── FUNCTION_PIPELINE.md
│   ├── GRAMMAR_NOTATION_REFERENCE.txt
│   ├── INTERPRETER_PIPELINE.md
│   ├── LAMBDA_PIPELINE.md
│   ├── LOXCALLABLE_PIPELINE.md
│   ├── PARSER_FUNCTIONS_EXPLAINED.txt
│   ├── PARSE_TREE_EXAMPLES.txt
│   ├── PARSE_TREE_PRACTICE_15_EXAMPLES.txt
│   ├── RECURSIVE_PARSING.pdf
│   ├── TURING_COMPLETENESS.md
│   ├── VISITOR_PATTERN_COMPLETE_FLOW.md
│   └── images/
│       ├── repl_output.png
│       └── test_output.png
├── test.lox
└── Lox.vcxproj
```

## 🎯 Features Implemented

### Lexer/Scanner (Chapter 4)
- Tokenizes Lox source code into tokens
- Recognizes all Lox token types — single/double character tokens, literals, keywords
- Line number tracking for error reporting
- String and number literal parsing
- Comment support (`//`)

### Parser (Chapter 5 & 6)
- Recursive descent parser for all Lox expressions
- Builds a proper **Abstract Syntax Tree (AST)**
- Handles operator precedence and associativity correctly
- Supports:
  - Arithmetic: `+`, `-`, `*`, `/`
  - Comparison: `<`, `<=`, `>`, `>=`, `==`, `!=`
  - Unary: `-`, `!`
  - Grouping: `(` ... `)`
  - Literals: numbers, strings, `true`, `false`, `nil`

### Interpreter (Chapter 7)
- Tree-walk interpreter that evaluates AST nodes directly
- Implements the **Visitor pattern** on the expression hierarchy
- Supports full expression evaluation: arithmetic, comparison, equality, unary
- `stringify()` for clean result output — trims trailing zeros from doubles, handles `bool` and `nil`
- Runtime error handling with line number reporting
- `isTruthy()` following Lox semantics — only `false` and `nil` are falsy
- Fix for `bool`-in-variant implicit conversion to `double` (C++ quirk with `std::variant`)

### AstPrinter (Chapter 5)
- Implements the **Visitor pattern** on the AST
- Traverses the expression tree and pretty-prints it as a **Lisp-style S-expression**
- Used for debugging and verifying parser correctness
- Example: `1 + 2 * 3` → `(+ 1.000000 (* 2.000000 3.000000))`

### Lox Driver & REPL (Architectural Refactor)
- `Lox.cpp` drives the full pipeline — REPL mode and file execution via `run()`
- Separate error reporting for compiler errors (lexer/parser) vs runtime errors
- Fixed ***circular dependency*** between `Lox.h` and `Interpreter.h`, `Parser.h`, `Lexer.h` via proper layering
- `Common.h` and lower layers kept blind to high-level modules — inner layers don't know about outer ones
- Added `core/Error.h` with a proper error hierarchy: `LoxError` → `LexError`, `ParseError`, `RuntimeError`
- Moved includes from headers to implementation files — headers only include what they strictly need
- Fixed string literal storage bug (trailing quote character)

### Statements & State (Chapter 8)
- `print` statements and expression statements
- Variable declaration (`var`) and assignment (`=`)
- Block scoping with `{ }` — variables are local to their block
- **Lexical environment chain** — `Environment` class with `enclosing` pointer walks up scope chain for variable lookup and assignment
- `executeBlock()` creates a new child `Environment`, executes statements, then restores the previous scope — with proper cleanup on exceptions via `try/catch/rethrow`
- Raw pointer used intentionally for `enclosing` — non-owning observer, parent always outlives child

### Control Flow (Chapter 9)
- `if` / `else if` / `else` — `else if` falls out naturally from the grammar, no special case needed
- `while` loops
- `for` loops — **desugared** into `while` at parse time, no new AST node or interpreter logic needed
- Logical `and` / `or` with **short-circuit evaluation** — `and` returns first falsy value, `or` returns first truthy value
- Lox is now **Turing complete** — can compute anything computable

### Functions (Chapter 10)
- `fun` keyword for function declarations
- First-class functions — stored in variables, passed as arguments, returned from functions
- `LoxCallable` abstract base class — interface for everything callable (functions, native functions, future classes)
- `LoxFunction` runtime object — wraps `FuncStmt` blueprint with a captured closure environment
- `return` statements — implemented via `ReturnException` to cleanly unwind arbitrarily deep call stacks
- Arity checking — argument count validated before `call()` is ever invoked
- Native functions — `clock()` injected into global environment at interpreter startup
- Function scope — each call creates a fresh `Environment` child of the closure
- Known limitation: closures over local scopes segfault — fixed in Chapter 11 with `shared_ptr` environments

### Lambda Functions — Challenge Extension (Chapter 10)
- Anonymous function expressions — `fun(x) { return x * x; }`
- Parsed in `parsePrimary()` — lambda is an **expression**, produces a value
- `LambdaExpr` AST node — params + body, no name
- `LoxLambda` runtime class — inherits `LoxCallable`, identical call mechanics to `LoxFunction`
- Immediately invokable: `fun(a, b) { return a + b; }(3, 5)`
- Storable: `var square = fun(x) { return x * x; };`
- Passable as argument to higher-order functions
- Same closure limitation as `LoxFunction` — fixed together in Chapter 11

## 🖥️ REPL in Action

![REPL Output](Lox/docs/images/repl_output.png)

## 🔧 Building & Running

### Prerequisites
- Visual Studio 2019 or later (with C++20 support)
- Or any C++20 compatible compiler (GCC, Clang)

### Build in Visual Studio
1. Open `Lox.vcxproj` in Visual Studio
2. Select your configuration — **Debug** or **Release**
3. Build the solution: `Ctrl+Shift+B` or **Build → Build Solution**
4. The executable is output to `x64/Debug/Lox.exe` (or `x64/Release/Lox.exe`)

### Running a `.lox` file
After building, open a terminal at the root of the repo and run:

```bash
# From the repo root
./x64/Debug/Lox.exe test.lox

# Or with a path to any .lox file
./x64/Debug/Lox.exe path/to/your/file.lox
```

### REPL mode
Run without arguments to enter the interactive REPL:

```bash
./x64/Debug/Lox.exe
```

### Comprehensive Example
```lox
// 1. Basic lambda stored in variable
var square = fun(x) { return x * x; };
print square(5);   // 25

// 2. Immediately invoked
print fun(a, b) { return a + b; }(3, 5);  // 8

// 3. Passed as argument
fun apply(fn, value) {
    return fn(value);
}
print apply(fun(x) { return x * 2; }, 10);  // 20

// 4. No parameters
var greet = fun() { return "hello"; };
print greet();  // hello

// 5. Use Recursion to print 20 Fibonacci series numbers.
fun fib(n) {
  if (n <= 1) return n;
  return fib(n - 2) + fib(n - 1);
}

for (var i = 0; i < 20; i = i + 1) {
  print fib(i);
}

// 6. Returned from function, Segmentation fault. fix in chapter 11.
fun makeAdder(n) {
    return fun(x) { return x + n; };
}

var add5 = makeAdder(5);
print add5(3);   // 8
print add5(10);  // 15
```

### Output

![Test Output](Lox/docs/images/test_output.png)


## 📖 Learning Notes

### Java → C++ Translation Challenges
- `std::variant` for the `Literal` type (requires C++17+)
- Manual memory management vs Java's garbage collection
- Visitor pattern implementation differs significantly
- Proper use of `std::string` and `std::unique_ptr` for AST nodes — since each node has exactly one parent/owner, `unique_ptr` is the right fit over `shared_ptr`
- `std::variant` with both `bool` and `double` causes implicit conversion issues — C++ prefers converting `bool` to `double`, so comparison results must be explicitly wrapped as `LiteralValue(bool)` to force correct type storage
- **Circular dependency** is a real C++ problem — solved by enforcing strict layer isolation and avoiding high-level includes in low-level headers
- **Header discipline** — only include in headers what is needed for the type declarations; move everything else to the `.cpp` file. Critical at scale
- **Uninitialized pointers** are a classic C++ footgun — a raw `Environment*` member with no initializer points at garbage memory and causes an instant segfault on first use. Always initialize pointers, either inline (`= nullptr`) or in the constructor initializer list
- **For loop desugaring** — `for` is not a new interpreter concept, just the parser assembling `while` + `BlockStmt` nodes. The interpreter never knows a `for` loop existed
- **`std::move` is non-negotiable with `unique_ptr`** — passing a `unique_ptr` without `std::move` is a compile error (copy constructor is deleted by design)
- **Exceptions as control flow** — `ReturnException` is not an error, it's an intentional use of C++ exception machinery to unwind the call stack cleanly from any depth back to `LoxFunction::call()`
- **`shared_ptr` for callables** — functions need shared ownership since multiple variables can reference the same function object. `unique_ptr` would break `var f = someFunction; var g = f;`
- **Forward declaration breaks circular dependencies** — `LoxCallable.h` forward declares `Interpreter` so headers don't include each other. Full `#include` goes in `.cpp` files only

## 🙏 Acknowledgments

- [Bob Nystrom](https://github.com/munificent) for the excellent [*Crafting Interpreters*](https://craftinginterpreters.com/) book
- The original Java implementation (jlox) as reference

## 📝 License

This is a learning project based on *Crafting Interpreters*. The original book and its code are by Bob Nystrom.

---

⭐ Star this repo if you're also learning from *Crafting Interpreters*!