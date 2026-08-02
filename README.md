# HyDrop.jl 💧

**HyDrop.jl** is a custom-built, C-style interpreted scripting language, designed and implemented from scratch as a compiler construction project. It features a hand-crafted lexer, a grammar-driven parser, and a tree-walking interpreter — built using **Flex**, **Bison**, and **C**.

Beyond the standard control-flow and arithmetic constructs expected of a teaching language, HyDrop.jl ships with a set of unconventional, "real-world inspired" language features — self-destructing variables, auto-updating reactive variables, a fixed-precision `money` type, automatic retry blocks, execution benchmarking, variable history tracking, and built-in safety guards against infinite loops and division-by-zero crashes.

```
==========================================
   HyDrop.jl Compiler (Parsing Engine)
==========================================
```

---

## Table of Contents

- [Overview](#overview)
- [Language Features](#language-features)
- [How It Was Built](#how-it-was-built)
- [Project Architecture](#project-architecture)
- [Repository Structure](#repository-structure)
- [Getting Started](#getting-started)
- [Syntax at a Glance](#syntax-at-a-glance)
- [Example Programs](#example-programs)
- [Challenges Faced](#challenges-faced)
- [Roadmap](#roadmap)
- [License](#license)

---

## Overview

HyDrop.jl (`.jl` script files, compiled into `hydrop.exe`) is a dynamically-typed, imperative scripting language. Every program is parsed into an **Abstract Syntax Tree (AST)** and then walked and executed directly by the C runtime — there is no intermediate bytecode or VM.

The language was built in phases, each one layering new capability on top of a stable core:

| Phase | Focus | Highlights |
|-------|-------|------------|
| **Phase 1** | Core Fundamentals | Variables, arithmetic, comparisons, `if/else`, `while`, `for`, `show`, `input` |
| **Phase 3** | Security & Stability | `let_once` (quantum/self-destructing variables), infinite-loop guard, divide-by-zero guard |
| **Phase 4** | Advanced Software Engineering | `history()` time-travel tracking, `benchmark {}` execution timing, `draw_chart()`, `dump_memory()` |
| **Phase 5** | Modern Industry Solutions | `reactive` auto-updating variables, `money` fixed 2-decimal type, `retry/on_fail` fault-tolerant blocks |
| **Phase 6 & 7** | Architecture & Advanced Real-World | `clean_code`, `safe_extract`, `async_task`, `validate` (pipeline groundwork) |

---

## Language Features

### 🧮 Core Programming
- Variables via `let`, reassignment, arithmetic (`+ - * /`)
- Comparison operators: `> < >= <= == !=`
- Control flow: `if / else`, `while`, `for`
- `show` (print) and `input` (read from stdin)
- Strings with `\n` / `\t` escape sequences, and string+value concatenation via `+`
- Numeric **unit literals**: `5km` and `500m` are automatically normalized to a base unit (meters) at lex time

### 🛡️ Safety & Security
- **Divide-by-zero guard** — halts execution with a memory snapshot instead of crashing silently
- **Infinite-loop guard** — any `while` loop is force-stopped after 10,000 iterations
- **`let_once` (Quantum Variables)** — a value that self-destructs the instant it is read, simulating one-time secrets (e.g. OTPs)

### 🧠 Reactive & Financial Types
- **`reactive`** — declares a variable bound to a formula; it automatically recomputes whenever any variable it depends on changes
- **`money`** — a numeric type that is always rounded and displayed to exactly 2 decimal places, avoiding floating-point display artifacts

### 🧰 Developer Tooling (built into the language itself)
- **`history(var)`** — toggles time-travel tracking on a variable and prints its full value history
- **`benchmark { ... }`** — times a block's execution in milliseconds
- **`draw_chart(var)`** — renders an instant ASCII bar chart for a numeric variable
- **`dump_memory()`** — prints a full snapshot of the interpreter's symbol table
- **`--explain` flag** — runs the interpreter in verbose mode, logging every memory write and branch decision as it happens

### 🔁 Fault Tolerance
- **`retry(n) { ... } on_fail { ... }`** — re-executes a block up to `n` times if a runtime fault (currently: division by zero) occurs, falling back to an `on_fail` block if every attempt fails, using `setjmp`/`longjmp` for control-flow recovery

---

## How It Was Built

HyDrop.jl follows the classical three-stage compiler pipeline:

1. **Lexical Analysis (`lexer.l`)** — Flex scans the raw `.jl` source and converts it into a stream of tokens (keywords, identifiers, numbers, strings, operators). This is also where unit-suffixed numbers (`km`/`m`) and escaped strings are pre-processed.
2. **Syntax Analysis (`parser.y`)** — Bison consumes the token stream according to a formal grammar and builds an **Abstract Syntax Tree**, resolving operator precedence (`* /` bind tighter than `+ -`, which bind tighter than comparisons) along the way.
3. **Tree-Walking Interpretation (`ast.c` / `ast.h`)** — The AST is walked recursively. `eval_expr()` resolves expressions to `float` values, while `execute_ast()` drives statements and control flow. A simple linear symbol table (`Variable memory[MAX_VARS]`) tracks each variable's value, type flags (`is_reactive`, `is_quantum`, `is_money`, `is_tracked`) and history buffer.

The toolchain is orchestrated by `run.bat`, which regenerates the parser/lexer from source, compiles everything with GCC, and immediately runs the resulting binary against a chosen script.

---

## Project Architecture

```
 ┌────────────┐      tokens      ┌────────────┐      AST      ┌────────────┐
 │  lexer.l   │ ───────────────▶ │  parser.y  │ ─────────────▶ │   ast.c    │
 │  (Flex)    │                  │  (Bison)   │                │ (Interpreter) │
 └────────────┘                  └────────────┘                └────────────┘
       │                               │                              │
       ▼                               ▼                              ▼
   lex.yy.c                     parser.tab.c/.h                  hydrop.exe
```

- **`lexer.l`** → generates `lex.yy.c`
- **`parser.y`** → generates `parser.tab.c` / `parser.tab.h`
- **`ast.c` + `ast.h`** → the runtime: AST node constructors, the symbol table, expression evaluation, and statement execution
- All three are compiled and linked together into a single executable: `hydrop.exe`

---

## Repository Structure

```
hydrop-lang/
├── lexer.l              # Flex specification – tokenizer rules
├── parser.y              # Bison grammar – syntax rules & AST construction
├── ast.h                  # AST node types & function declarations
├── ast.c                  # AST node constructors, symbol table & interpreter
├── parser.tab.c            # Generated by Bison (checked in for convenience)
├── parser.tab.h            # Generated by Bison (checked in for convenience)
├── lex.yy.c                # Generated by Flex (checked in for convenience)
├── run.bat                 # One-command build & run script (Windows)
│── test1.jl            # Core features: arithmetic, comparisons, if/else, while, input
│── test2.jl             # history(), unit literals, benchmark, draw_chart, let_once
│── test3.jl              # reactive variables, money type, retry/on_fail, for loop
│── test4.jl               # dump_memory() and error-handling demonstration
├── README.md                   # This file
└── Language_Manual.txt          # Full syntax guide & compile/run instructions
```

---

## Getting Started

### Prerequisites (Windows)
- [GnuWin32 Bison](https://gnuwin32.sourceforge.net/packages/bison.htm) or [win_bison](https://github.com/lexxmark/winflexbison) → `win_bison`
- [win_flex](https://github.com/lexxmark/winflexbison) → `win_flex`
- [MinGW GCC](https://sourceforge.net/projects/mingw/) or any standard C compiler on your `PATH`

> The project assumes `win_bison` and `win_flex` are installed at `C:\sdk\`. Update the paths in `run.bat` if your setup differs.

### Build & Run

```bat
:: From the project root
run.bat test1.jl
```

This single command will:
1. Clean previous build artifacts
2. Regenerate the parser from `parser.y` (Bison)
3. Regenerate the lexer from `lexer.l` (Flex)
4. Compile everything with GCC into `hydrop.exe`
5. Immediately execute your chosen `.jl` script

If you run `run.bat` with no argument, it will prompt you interactively for a filename.

### Manual build (Linux/macOS, using GNU Flex & Bison)

```bash
bison -d parser.y
flex lexer.l
gcc parser.tab.c lex.yy.c ast.c -o hydrop -lm
./hydrop examples/test1.jl
```

### Debug / verbose mode

```bat
hydrop.exe test1.jl --explain
```

Runs the script with step-by-step memory and branch-decision logging enabled.

---

## Syntax at a Glance

```javascript
// Variables & arithmetic
let a = 10;
let b = 3;
show "a + b = " + (a + b);

// Conditionals
if (a > b) {
    show "a is bigger";
} else {
    show "b is bigger";
}

// Loops
while (a > 0) {
    a = a - 1;
}
for (let i = 0; i < 5; i = i + 1) {
    show "i = " + i;
}

// Distance literals (auto-converted to meters)
let distance = 5km + 500m;

// Reactive variables (auto-recompute on dependency change)
reactive y = a * 2;

// Fixed-precision financial type
money bill = 0.1 + 0.2;   // displays as 0.30

// One-time / self-destructing variable
let_once otp = 4521;      // destroyed after first read

// Fault-tolerant execution
retry(3) {
    let result = 100 / divisor;
} on_fail {
    show "All attempts failed.";
}

// Developer tooling
history(a);        // toggle/inspect value history
benchmark { ... }  // time a block
draw_chart(sales);  // ASCII bar chart
dump_memory();       // print the whole symbol table
```

A complete reference of every keyword, operator, and construct is available in **[`Language_Manual.txt`](./Language_Manual.txt)**.

---

## Example Programs

The `examples/` folder contains four scripts that collectively exercise every feature of the language:

- **`test1.jl`** — arithmetic, all comparison operators, `if/else`, `while`, `input`, string concatenation and escape sequences
- **`test2.jl`** — `history()`, unit literals (`km`/`m`), `benchmark {}`, `draw_chart()`, the infinite-loop guard, and `let_once` (including its expected crash on second read)
- **`test3.jl`** — `reactive` variables, the `money` type, `retry/on_fail` fault tolerance, and `for` loops
- **`test4.jl`** — `dump_memory()` and the runtime's error-handling/crash behavior on invalid syntax

---

## Challenges Faced

- **Ambiguous grammar around `for`-loops** — the `for (init; cond; update) { body }` construct had to be desugared inside the grammar action itself into an equivalent `while` loop wrapped around the init/update statements, since the interpreter has no native `NODETYPE_FOR`.
- **Division-by-zero recovery inside `retry`** — rather than crashing on the first failed attempt, `retry` needed to *recover* control flow mid-expression-evaluation. This was solved with `setjmp`/`longjmp`, jumping straight back out of `eval_expr()` into the retry loop without unwinding the whole interpreter.
- **Reactive variable propagation** — every `set_var()` call had to trigger a full re-evaluation pass (`recompute_reactive()`) across the symbol table so dependent `reactive` variables stay in sync, while being careful not to create infinite recomputation chains.
- **Floating-point precision for `money`** — plain `float` arithmetic (e.g. `0.1 + 0.2`) doesn't land cleanly on `0.30`, so the `money` type rounds to 2 decimal places at assignment time and is formatted with `%.2f` rather than `%g` at print time.
- **String vs. numeric concatenation with `+`** — since the language only has one arithmetic operator set, `show "a = " + a` needed special-case handling (`contains_string()` / `print_node()`) to decide whether `+` means numeric addition or string concatenation, based on the AST shape.
- **Self-destructing (`let_once`) variables** — required threading an extra "quantum" flag through the whole variable lifecycle (`set_var`, `get_var`) so a read could both *return* and *invalidate* a value in a single operation.

---

## Roadmap

- [ ] Full pipeline execution for `validate` and `|>` (currently tokenized but not yet wired into the grammar's evaluation logic)
- [ ] Real thread execution for `async_task` (currently reserved as a keyword)
- [ ] User-defined functions
- [ ] Arrays / lists (bracket tokens are already reserved)
- [ ] A proper type system beyond `float` / `string` / `money`

---

## License

This project is released under the [MIT License](./LICENSE). Feel free to fork, extend, and experiment with the language.

---

*Built as a compiler design project — a hands-on exploration of lexical analysis, LALR parsing, and tree-walking interpretation using Flex, Bison, and C.*
