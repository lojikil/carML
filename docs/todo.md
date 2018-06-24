# The Current language TODOs

A quick list of the current `TODO`s.

1. **DONE** Add complex types to `val`
1. **DONE** Add complex types to records
1. **DONE** Add complex types to `let` 
1. **DONE** Add `var` form
1. **DONE** type tags
1. **FIXED** _BUG_: `Foo Bar int` as type constructor is parsed as `(type-constructor Foo (complex-type Bar Int))` when it should be `(type-constructor Foo Bar Int)` (2 members)
1. **FIXED** _BUG_: look at how the `experiments/sexpr.carml` is being parsed, and note that several constructors are missing
1. **DONE** (test case for the above: `Foo Bar int`)
1. **FIXED** _BUG_: `def bar h:Url => Url` is parsed as `(parameter-list (ident h) (tag Url))` when it should be `(parameter-list (parameter-def h (complex-type (tag Url))))`
1. **FIXED** (test case for the above: `def foo f:Url => Url = f`)
1. **FIXED** _BUG_: complex return types
1. _TEST_: add more complex type tests  
1. add test runner: parse SExpression output...
1. **DONE** parse HOFs in declarations
1. **DONE**Review switch to Scala-style `[]` for types.
1. Parse `@`/`declare` forms
1. **WONTFIX in C version** Update `val`, `let`, records to use the new `declare` type parser
1. **WONTFIX** Make function definitions & `let` forms accept `begin` style function calls (i.e. avoid using `()`)
1. `match`/`case` form, with guards.
1. Figure out a decent backing for Rust-style deques (possibly implemented from records + arrays)
1. **DONE**: Partial application syntax: `$()`, including `_` as filler
1. make `$` and `|>` work as they would in Haskell & OCaml/F# respectively.
1. integrate `$` and `|>` with `$()`
1. Hoare-logic (pre, post, invariants, &c.)
1. make `mung_single_type` for reading a single type, use for `@` forms
1. Runtime, which should be pretty minimal
1. **DONE** Compile to C in the style of Enyalios
1. **DONE** an `extern` or `alien` form for easy FFI
1. **DONE** Finish the `use` form
1. write an actual inclusion algorithm for the compiler to consume `use`d libraries
1. **DONE** Parsing of variants
1. **DONE** Compilation of variants
1. **DONE** Parsing of polymorphic variants
1. **DONE** Compilation of polymorphic variants
1. **DONE** Make sure types are correct for types/polys
1. OCaml/SML-style Modules, and their application to higher-kinded types
1. `deque` and memory model
1. investigate untagged unions (`int | float` as a type)
1. Friendlier REPL, with keyboard support, and a simple Logo-style `edit` command
1. **DONE** Syntax updates: `record`
1. **WONTFIX** Syntax updates: `def`
1. **WONTFIX** Syntax updates: `match`
1. **WONTFIX** Syntax updates: `fn`
1. **WONTFIX in C version** _REVIEW_ make `let` & `var/var` treat items like a call form (as in `val r : int = sum x 10` instead of `val r : int = (sum x 10)`)
1. **WONTFIX in C version** make the types parsing code more modular; could easily extract that out into a function
1. **DONE** (not in the most elegant way, mind, but...) Finally fix the frakking lexer to not goof up internal states
1. Csharp-style record parameter unboxing
1. **DONE** Error handling: `Either`
1. **DONE** Option types
1. **WONTFIX in C version** Lexer-as-stream
1. in the base C compiler & carML self-hosting one, include a defined list of compiler errors with friendly data
1. add line numbers to errors
1. **DONE** Fix type state transition, which fails for Tagged types (`Url`)
1. **DONE** Fix parsing of single line `begin`: `{sum x 10}` fails to parse properly
1. **DONE** Fix `float`, `bool`, and `char` parsing
1. Add sized ints/uints/floats (e.g. `uint8`) as types (what about `U8` or `U64`? that works...)
1. **DONE** Fix edge case: complex type right before `=>` fails `def foo bar : Url => int = ...` fails
1. add nano-pass: a-normal form (ANF)
1. add nano-pass: lambda lifting
1. add nano-pass: ANF => SSA
1. add nano-pass: rewrite `let`/`letrec` => `val` + temporary binding
1. add nano-pass: constant folding
1. add nano-pass: select the correct reified constructor implementation
1. add nano-pass: demand-driven type inference
1. **DONE** fix complex type handling in `typespec2c`
1. fix match with `type`s
1. Investigate: method of determining effects, and how that could make ANF easier (lift once for two calls)
1. **DONE** Investigate: currently there is a syntax ambiguity in `begin` forms: is `t` a unary function call, or an identifier? Fix: identifier
1. Tests, both for IR and C
1. JS, Java, C++ backends, but written in carML itself and using the SExpression output.

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

**DONE, 16OCT2017** Also, this is broken:

     >>> def foo x = { sum x x }
    (define foo (parameter-list (identifier x))
        (begin
            (identifier sum)
            (identifier x)
            (identifier x)))
    >>> def foo x = {
    sum x x
    }
    (define foo (parameter-list (identifier x))
        (begin
            (call (identifier sum) (identifier x) (identifier x))))

 **DONE, 13OCT2017** Furthermore, this actually introduces a syntax & semantic ambiguity:

     def foo x : int => int = {
         var t : int = 10
         set! t (sum t x)
         t
     }

Technically, we want the _value_ of `t`, but the current compiler thinks
that `t` is actually an unary function application. Need to chew on how 
to fix that... OCaml requires explicit `()` for unary, so I could do
the same... not really sure I like that, but it is one solution.

# Scala-style types

 It's pretty appealing to me to use Scala-style type declarations... this would mean `@` would be freed from
declaring types to simply adding annotations like in Scala... Furthermore, it would make parsing a bit easier 
to boot. This would imply that:

    @declare foo Array of Int => Int
    def foo x = ...

Would become:

    def foo x: Array[Int] = ...

I also wonder if we can still use `[]` for `indexGet` because the type language & the term language need-not
be 1 to 1...

# Syntax Updates

I've been thinking about certain syntax choices I made originally, and thinking about updating them:

- **DONE** remove the `=` in `record`s: `record foo { x int; y int;}`
- **WONTFIX** allow the use of begin blocks directly following parameter lists: `def foo x { println x; sum x x}`
- **WONTFIX** allow the use of begin blocks directly following `fn` parameter lists: `fn x { println x; sum x x}`
- **N/A** still allow `def foo x = { block stuff ... }`
- **WONTFIX** match forms to just directly use begin blocks: `match x { Some y => ... ; None => ... }`

# Pre-requisites, Post-requisites, Invariants

I'd like to embed some notion of the usual Hoare-logic forms into the language as extensions or _refinements_ to
the `declare` or `@` form. I was thinking something similar to:

    @foo x Num y Num -> Num
    @/requires foo >= x y
    @/returns foo + x y
    def foo x y = ...

Not sure how I want the Hoare-logic to look; `@<name> /requires` vs `@/requires <name>`...

_Update_: I've been playing with F\* for a while now, and reading up about its
[KreMLin](https://fstarlang.github.io/general/2016/09/30/introducing-kremlin.html) backend... the idea to have
refinements & pre/post conditions as `{block}` is pretty appealing to me.

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

# **DONE** what in the world...

Figured it out: parser has a case for 'c', but no productions, so it can't actually parse anything that
starts with a 'c'

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

# Partial Application form (really, a specialization form)

I'm thinking that `$()` will denote partial application. For example:

    def foo x y z = + x (* y z)

So here, the function `foo` requires 3 parameters, but there may be cases wherein we
do **not** wish to use all three, and this is where `$()` forms would come into play:

    let bar = $(foo 10 _ 12) in
    println (bar 11)

Here, the "call" to `bar` is just rewritten to be `foo 10 11 12`. I've wondered how, or if, it
should support memoizing parameters; for example, consider the following:

    def foo x y = + x y
    def baz x = * x 10
    let bar = $(foo _ (baz 11)) in
    println (bar 12)

Should we compute and store the value `(baz 11)`, or should we rerun that calculation at each
application of the specialization form? Both have implications, but I'm wondering which is the
less surprising of the two? I guess the fact that we're going for an SRFI-26 style here is the
real answer to capturing...

## Interaction with thrushing forms

I **do** wish to support `|>` and the like. I think Haskell's `$` form has less appeal here, because
I'm already avoiding as many parens as possible, but `|>` from the various ML dialects is interesting.

    (|>
        foo
        $(bar _ 11)
        baz
        $(blah _ 12))

It'll be interesting to have a form that doesn't introduce a lambda capture but _does_ allow for
specialization...

Thinking about `$` though, that might be interesting to have...

    println (int_to_string (sum 54 100))
    # vs
    println $ int_to_string $ sum 54 100

Could make `$` the only infix form...
