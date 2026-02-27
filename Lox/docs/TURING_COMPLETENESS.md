# Turing Completeness

## The Cooking Analogy

Before control flow, Lox is like a recipe with no loops — you follow fixed steps, it ends, you're done. You can make exactly one serving, one time, with no decisions:

> "Add 1 egg. Add 2 cups flour. Bake."

That's it. You can't say *"keep stirring until smooth"* or *"if it's too thick, add water."*

After adding `if` and `while`, Lox gets:

> "Keep stirring **until** the batter is smooth."  
> "**If** it's too thick, add water."  
> "Repeat this **for** each guest."

Now the recipe can adapt, repeat, and make decisions. It's a real recipe — not just a fixed list of steps.

---

## What is a Turing Machine?

A **Turing machine** is a theoretical model invented by Alan Turing in 1936. It was never meant to be built — it was meant to answer one question:

> *"What is the fundamental limit of what can be computed?"*

The machine itself is absurdly simple:
- An **infinite tape** (memory)
- A **read/write head** that moves along the tape
- A **set of rules** — if you see X, write Y, move left/right

That's it. And yet, Turing proved this simple machine can compute *anything computable*.

---

## What is Turing Completeness?

A system is **Turing complete** if it is equivalent in computational power to a Turing machine. The minimum ingredients are:

- **Conditional branching** — `if`
- **Repetition** — `while` / `for`
- **Memory** — variables

That's the entire bar. And it's surprisingly low — things you wouldn't expect are Turing complete:

- Microsoft Excel (with enough formulas)
- CSS (with certain tricks)
- Conway's Game of Life (just pixels flipping on a grid)
- Minecraft redstone circuits

None of them look like programming languages. But they all cross the threshold.

---

## What Can't You Compute Without Loops?

```lox
// Sum 1 to 100 without loops? You'd have to literally write:
var sum = 1 + 2 + 3 + 4 + 5; // ... all the way to 100
// Impossible to generalize
```

You're stuck. The program always does the same fixed number of steps.

## What Can You Compute With Loops?

```lox
var sum = 0;
var i = 1;
while (i <= 100) {
    sum = sum + i;
    i = i + 1;
}
print sum; // 5050
```

Now it generalizes. Fibonacci, factorials, searching, sorting — anything. The language goes from a fancy calculator to a real programming language.

---

## The Halting Problem

The deeper implication Turing discovered is the **halting problem**:

> *You cannot write a general program that looks at any other program and tells you whether it will finish or loop forever.*

This is mathematically proven to be impossible for any Turing complete system.

The cooking analogy: *"Can I know in advance if this recipe will ever finish cooking?"*  
Turing's answer: **No. You just have to let it run and see.**

This is not a limitation of our tools or intelligence — it is a fundamental mathematical truth about computation itself.

---

## Why Does This Matter for Lox?

Before Chapter 9, Lox could not loop. Every program terminated in a fixed number of steps — it was fundamentally limited, like a recipe with no repetition.

After Chapter 9, Lox has `if`, `while`, and `for`. It crosses the threshold into the same class as every real programming language ever built — C, Python, Java, all of them.

That's the milestone Nystrom is marking. Not just "we added loops." But: **Lox can now compute anything.**

---

*Reference: Crafting Interpreters, Chapter 9 — Control Flow*
