# Lox Interpreter (C++)

A C++ implementation of the Lox programming language from [*Crafting Interpreters*](https://craftinginterpreters.com/) by Bob Nystrom.

## 📚 About

This project is my journey through *Crafting Interpreters*, translating the Java implementation (jlox) to modern C++. It's a learning project focused on understanding interpreter design, lexical analysis, parsing, and language implementation.

## 🚧 Current Status

**Completed:**
- ✅ **Chapter 4: Scanning** - Lexical analysis and tokenization

**Coming Next:**
- ⏳ Chapter 5: Representing Code (AST)
- ⏳ Chapter 6: Parsing Expressions
- ⏳ Chapter 7: Evaluating Expressions
- ⏳ And more...

## 🎯 Features Implemented

### Lexer/Scanner
- Tokenizes Lox source code into tokens
- Recognizes all Lox token types:
  - Single-character tokens: `(`, `)`, `{`, `}`, `,`, `.`, `-`, `+`, `;`, `*`
  - One or two character tokens: `!`, `!=`, `=`, `==`, `>`, `>=`, `<`, `<=`
  - Literals: numbers (integers and floats), strings, identifiers
  - Keywords: `and`, `class`, `else`, `false`, `for`, `fun`, `if`, `nil`, `or`, `print`, `return`, `super`, `this`, `true`, `var`, `while`
- Line number tracking for error reporting
- String and number literal parsing
- Comment support (`//`)

## 🔧 Building

### Prerequisites
- Visual Studio 2019 or later (or any C++20 compatible compiler)
- C++20 standard library support (for `std::variant`)

### Compilation
Currently a Visual Studio project. Open `Lox.sln` and build.

**Note:** The project currently runs a hardcoded test in `main()`. CLI argument handling will be added in future chapters.

## 🚀 Usage

Currently, the lexer is tested with hardcoded source code. Full CLI and REPL functionality coming in later chapters.

### Example Test Code
```lox
var x = 42;
var y = 3.14;
var name = "Lox";

if (x >= 10) {
    print name + " value:";
    print x + y;
}
```

### Current Output (Tokens)
```
Token(36, var, 4)
Token(19, x, 4)
Token(13, =, 4)
Token(21, 42, 4)
Token(8, ;, 4)
Token(36, var, 5)
Token(19, y, 5)
Token(13, =, 5)
Token(21, 3.14, 5)
Token(8, ;, 5)
Token(36, var, 6)
Token(19, name, 6)
Token(13, =, 6)
Token(20, "Lox", 6)
Token(8, ;, 6)
Token(28, if, 8)
Token(0, (, 8)
Token(19, x, 8)
Token(16, >=, 8)
Token(21, 10, 8)
Token(1, ), 8)
Token(2, {, 8)
Token(31, print, 9)
Token(19, name, 9)
Token(7, +, 9)
Token(20, " value:", 9)
Token(8, ;, 9)
Token(31, print, 10)
Token(19, x, 10)
Token(7, +, 10)
Token(19, y, 10)
Token(8, ;, 10)
Token(3, }, 11)
Token(38, EOF, 13)
```

## 📖 Learning Notes

### Java → C++ Translation Challenges
- `std::variant` for the `Literal` type (requires C++17+)
- Manual memory management considerations
- Different standard library utilities
- Proper use of `std::string` vs `const char*`

## 🙏 Acknowledgments

- [Bob Nystrom](https://github.com/munificent) for the excellent [*Crafting Interpreters*](https://craftinginterpreters.com/) book
- The original Java implementation (jlox) as reference

## 📝 License

This is a learning project based on *Crafting Interpreters*. The original book and its code are by Bob Nystrom.

---

⭐ Star this repo if you're also learning from *Crafting Interpreters*!
