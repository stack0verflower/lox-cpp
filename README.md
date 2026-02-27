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

**Coming Next:**
- ⏳ Chapter 10: Functions
- ⏳ Chapter 11: Resolving and Binding
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
│       └── Interpreter.h
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
│   │   └── Interpreter.cpp
│   └── Main.cpp
├── docs/
│   ├── ARCHITECTURE_NOTES.md
│   ├── ASSIGNMENT_PIPELINE.md
│   ├── FILE_STRUCTURE.txt
│   ├── GRAMMAR_NOTATION_REFERENCE.txt
│   ├── INTERPRETER_PIPELINE.md
│   ├── PARSER_FUNCTIONS_EXPLAINED.txt
│   ├── PARSE_TREE_EXAMPLES.txt
│   ├── PARSE_TREE_PRACTICE_15_EXAMPLES.txt
│   ├── TURING_COMPLETENESS.md
│   ├── VISITOR_PATTERN_COMPLETE_FLOW.md
│   └── images/
│       ├── repl_output.png
│       └── test_output.png
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
// --- Variables & Arithmetic ---
var x = 10;
var y = 3;
print x + y;        // 13
print x * y;        // 30
print x == y;       // false
print x > y;        // true

// --- Strings ---
var name = "Lox";
print name + " v0.9"; // Lox v0.9

// --- Scoping ---
var a = "global";
{
    var a = "outer";
    {
        var a = "inner";
        print a;    // inner
    }
    print a;        // outer
}
print a;            // global

// --- If / Else If / Else ---
var score = 85;
if (score >= 90) {
    print "A";
} else if (score >= 80) {
    print "B";      // B
} else {
    print "C";
}

// --- While Loop ---
var i = 1;
var sum = 0;
while (i <= 10) {
    sum = sum + i;
    i = i + 1;
}
print sum;          // 55

// --- For Loop ---
var factorial = 1;
for (var n = 1; n <= 6; n = n + 1) {
    factorial = factorial * n;
}
print factorial;    // 720

// --- Fibonacci ---
var fa = 0;
var fb = 1;
for (; fa < 100; ) {
    print fa;
    var temp = fa;
    fa = fb;
    fb = temp + fb;
}
// 0 1 1 2 3 5 8 13 21 34 55 89

// --- Nested Loops ---
var result = 0;
for (var outer = 1; outer <= 3; outer = outer + 1) {
    for (var inner = 1; inner <= 3; inner = inner + 1) {
        result = result + 1;
    }
}
print result;       // 9

// --- Logical Operators ---
print true and false;   // false
print false or true;    // true
print nil or "default"; // default (short-circuit)
```

### Output

![REPL Output](Lox/docs/images/test_output.png)

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

## 🙏 Acknowledgments

- [Bob Nystrom](https://github.com/munificent) for the excellent [*Crafting Interpreters*](https://craftinginterpreters.com/) book
- The original Java implementation (jlox) as reference

## 📝 License

This is a learning project based on *Crafting Interpreters*. The original book and its code are by Bob Nystrom.

---

