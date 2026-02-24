# Architecture & Design Notes

> Lessons learned during the architectural overhaul of the Lox interpreter (Chapter 8 groundwork)

---

## 1. Circular Dependency

### What is it?
A **circular dependency** occurs when two files include each other, directly or indirectly.

```cpp
// Lox.h includes Interpreter.h
#include "interpreter/Interpreter.h"

// Interpreter.h includes Lox.h to use Lox::error()
#include "core/Lox.h"
```

The compiler gets stuck in an infinite include loop and fails.

### How it happened here
`Interpreter.h` needed `Lox::error()` to report runtime errors. `Lox.h` needed `Interpreter.h` to run the interpreter. Classic two-way dependency.

### How to fix it
The root cause is usually a **design problem** — a lower-level module (Interpreter) should not be calling into a higher-level module (Lox). Fix the architecture, not the include.

**Solution used:** Pass errors upward via exceptions (`RuntimeError`) instead of calling `Lox::error()` directly. The Interpreter throws, Lox catches. Dependency flows one way now.

```
Lox.cpp  →  Interpreter  →  (throws RuntimeError)
   ↑                               |
   └───────────────────────────────┘  (catches)
```

---

## 2. Error Hierarchy Design

### The Problem
One generic error type for everything is messy. A lexing error, a parse error, and a runtime error are fundamentally different — they happen at different phases and need different handling.

### The Solution — `core/Error.h`

```
LoxError  (base)
├── LexError      (lexing phase — bad token, unterminated string)
├── ParseError    (parsing phase — unexpected token, missing semicolon)
└── RuntimeError  (evaluation phase — type mismatch, divide by zero)
```

Each error type carries the relevant context (e.g. `RuntimeError` carries the `Token` where it occurred for line reporting).

### Why this matters
At the top level (`Lox.cpp`), you can catch each type separately and report them differently:

```cpp
catch (LexError& e)     { /* compiler error */ }
catch (ParseError& e)   { /* compiler error */ }
catch (RuntimeError& e) { /* runtime error  */ }
```

This mirrors how real compilers work — GCC, Clang, MSVC all have separate error categories for separate phases.

---

## 3. Layer Isolation (Architectural Blindness)

### The Principle
Lower layers must not know about higher layers. Dependencies only flow **downward**.

```
Main.cpp         ← top level, knows everything
Lox.cpp          ← knows about Interpreter, Parser, Lexer
Interpreter      ← knows about Expr, Token, Common
Parser           ← knows about Expr, Token, Common
Lexer            ← knows about Token, Common
Common.h         ← knows nothing, base of everything
```

### The Rule
If `Common.h` includes `Interpreter.h`, something is wrong. `Common.h` is the foundation — it should have zero knowledge of what's built on top of it.

Same for `Parser` — it should be completely blind to `Interpreter` and `Lox`. Parser just produces an AST, it doesn't care what happens to it.

### Why it matters
- Prevents circular dependencies
- Makes modules independently testable
- Makes the codebase easier to reason about — you always know what a module can and cannot touch
- Scales well — at 100 files, this discipline is the difference between maintainable and spaghetti

---

## 4. Header Include Discipline

### The Rule
> Only include in a header file what is **necessary for the type declarations in that header**. Everything else goes in the `.cpp`.

### Bad Practice
```cpp
// Interpreter.h
#include <iostream>      // ← only used in Interpreter.cpp, not needed here
#include <string>        // ← only used in Interpreter.cpp
#include "core/Lox.h"    // ← only used in Interpreter.cpp
```

Every file that includes `Interpreter.h` now silently pulls in all of the above — even if it doesn't need any of it.

### Good Practice
```cpp
// Interpreter.h — only what's needed for declarations
#include "core/Common.h"
#include "parser/Expr.h"

// Interpreter.cpp — everything else
#include <iostream>
#include <string>
#include "interpreter/Interpreter.h"
```

### Why it matters at scale
Imagine 100 header files each pulling in 10 unnecessary headers. Every translation unit recompiles all of them. Build times explode. At large codebases (like Chromium, LLVM), header discipline is enforced strictly for this reason.

A useful rule of thumb: **if removing an include from a header doesn't break the header's type declarations, it doesn't belong there.**

---

*These notes are part of the Lox C++ interpreter project — a personal implementation of the language from [Crafting Interpreters](https://craftinginterpreters.com/) by Bob Nystrom.*
