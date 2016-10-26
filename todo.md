# The Current language TODOs

A quick list of the current `TODO`s.

1. **DONE** Add complex types to `val`
1. **DONE** Add complex types to records
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
1. **DONE** (not in the most elegant way, mind, but...) Finally fix the frakking lexer to not goof up internal states
1. Csharp-style record parameter unboxing
1. Error handling: `Either`
1. Lexer-as-stream

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

# Pre-requisites, Post-requisites, Invariants

I'd like to embed some notion of the usual Hoare-logic forms into the language as extensions or _refinements_ to
the `declare` or `@` form. I was thinking something similar to:

    @foo x Num y Num -> Num
    @/requires foo >= x y
    @/returns foo + x y
    def foo x y = ...

Not sure how I want the Hoare-logic to look; `@<name> /requires` vs `@/requires <name>`...

# Csharp style record parameter unboxing

So C# will actually unbox structs that are passed as args:

    foo(mystruct);

will actually become

    foo(mystruct.member0, mystruct.member1, ...);

iff the struct is not modified (iirc). 

# Error handling

I don't really want to get too fancy with Error handling. I think it should be simple enough to just
use `Either` heavily for all core functions. Like an `os.open` could theoretically look like:

    @os.open string int -> Either a b 
    def os.open path mode {
        let res = (alien "open" path mode) in
        if (< res 0) then
            (Left (os.errno_lookup res))
        else
            (Right res)
    }

Yes, this would mean that the other side will always have to have some sort of `match` form to
test what the result actually is, but that would mean that error handling should be relatively
simple. I _like_ how Digamma implements SRFIs 23 & 34, but I don't know if I want to have that
much "stuff" in carML.

# Lexer-as-stream

The idea here is that a simple lexer should consume _all_ tokens from input, regardless of their syntactic
validity, prior to returning to the parser. The parser then can do the usual RDP, but instead of operating
on the file buffer, the RDP then can operate on the _token_ buffer. Thus, the file stream is completely
consumed (there are no dangling tokens the parser will consume once an error has occurred: the entire
stream can be abandoned), and we can implement nicer parser stuffs.

# what in the world...

    >>> foo [1,2,3,4]
    (identifier foo)
    >>> (array-literal (integer 1) (integer 2) (integer 3) (integer 4)) 
    >>> r
    (identifier r)
    >>> r [1 2 3 4]
    (identifier r)
    >>> (array-literal (integer 1) (integer 2) (integer 3) (integer 4)) 
    >>> car [1 2 3 4]

It never finished reading with the `car`...
