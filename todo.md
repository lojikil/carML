# The Current language TODOs

A quick list of the current `TODO`s.

1. **DONE** Add complex types to `val`
1. Add complex types to records
1. **DONE** Add complex types to `let` 
1. Parse `@`/`declare` forms
1. Make function definitions & `let` forms accept `begin` style function calls (i.e. avoid using `()`)
1. `match`/`case` form, with guards.
1. Figure out a decent backing for Rust-style deques (possibly implemented from records + arrays)
1. Partial application syntax: `$()`, including `_` as filler
1. Hoare-logic (pre, post, invariants, &c.)
1. Runtime, which should be pretty minimal
1. Compile to C in the style of Enyalios
1. an `extern` or `alien` form for easy FFI
1. Parsing of variants
1. Compilation of variants
1. Parsing of polymorphic variants
1. Compilation of polymorphic variants
1. OCaml/SML-style Modules, and their application to higher-kinded types
1. Friendlier REPL, with keyboard support, and a simple Logo-style `edit` command
1. **DONE** Syntax updates: `record`
1. Syntax updates: `def`
1. Syntax updates: `match`
1. make the types parsing code more modular; could easily extract that out into a function
1. Finally fix the frakking lexer to not goof up internal states

# Begin-style function calls

_NOTE_ the fifth point above is the following:

currently, `def` and `let` forms must enclose function calls with `()`:

    def foo x = (sum x x)
    let x = 95 in (foo x)

Whereas in `begin` blocks, they needn't be:

    def foo x = {
        sum x x
    }

I thought about just requiring users to use `begin` forms, but that means simple functions cannot be
succinctly written. things like `foo` above shouldn't require 3 lines of text. Think about simple
helper functions: 

    def f x = sum (mul x 5.3) 10
    map f someVector

Additionally, I want the repl to be able to handle those sorts of calls as well:

    >>> println "foo"
    foo
    _ : Unit
    >>> def f x = sum x 10 # secretly returns void
    >>> f (f 10)
    _ : Int = 30

# Syntax Updates

I've been thinking about certain syntax choices I made originally, and thinking about updating them:

- **DONE** remove the `=` in `record`s: `record foo { x int; y int;}`
- allow the use of begin blocks directly following parameter lists: `def foo x { println x; sum x x}`
- still allow `def foo x = { block stuff ... }`
- match forms to just directly use begin blocks: `match x { Some y => ... ; None => ... }`
