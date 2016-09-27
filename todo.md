A quick list of the current `TODO`s.

1. **DONE** Add complex types to `val`
1. Add complex types to records
1. Add complex types to `let` 
1. Parse `@` forms
1. Make function definitions & `let` forms accept `begin` style function calls (i.e. avoid using `()`)
1. `match`/`case` form, with guards.
1. Figure out a decent backing for Rust-style deques (possibly implemented from records + arrays)
1. Partial application syntax: `$()`, including `_` as filler
1. Hoare-logic (pre, post, invariants, &c.)
1. Runtime, which should be pretty minimal
1. Compile to C in the style of Enyalios
1. Compilation of variants
1. Compilation of polymorphic variants
1. OCaml/SML-style Modules, and their application to higher-kinded types
1. Friendlier REPL, with keyboard support, and a simple Logo-style `edit` command

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
