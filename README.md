# Overview of carML

_carML_ (pronounced "car-mul", caramel), previously called _XL/29_ (eXperimental Language No. 29), is meant to take ideas from PowerLogo & Yeti, mixed in with my experiences of working
with [Digamma](http://lojikil.com/p/digamma/). It's meant to be:

- tiny
- low barrier to entry
- no ceiling
- somewhat efficient
- without requiring too much mental cost from the programmer
- "secure"
- implemented and understood quickly
- borrowing ideas from ReasonML, Scala, BitC, OCaml, F#, Digamma, Yeti, PowerLogo, & Project Verona
    - I've been doing a *lot* of work in ReasonML of late for work, so definitely has become a strong muse
    - ReasonML: types, general style, Modules
    - F#: Simpler Module style, exploration of advanced types (like FStar and Fomega give)
    - Scala: type syntax, general style
    - Digamma: focus on C output, focus on Unix integration, production readiness
    - Yeti: simplicity & minimalism
    - PowerLogo: little barrier to entry, no ceiling, and practical focus
    - Project Verona: [this comment on operator precedence in Verona got me thinking](https://lobste.rs/s/jol24u/better_operator_precedence#c_bwd4ij)


This is at least the 3rd time (and probably 5th) time that I've attempted such a thing, so don't mind me at all. This *is* the first time
I've experimented publicly with my PLT designs tho.

# Name

I've experimented with multiple names; HotelML (named for "The HotelML" in NJ) & Melomys (a specious of mouse) were two contenders.
However, I was grabbing caramel macchiattos, which Starbucks labels as "carml mac", and thus named it after the drink.

# Syntax

 Syntactically, it's pretty much a garden variety ML dialect. Minimal keywords like `def` exist to visually break up code, but otherwise
if you're familiar with Yeti, StandardML, OCaml, &c. it should be relatively straight forward. The one location that it is *not* is in the
standard base.

- there are no operators in the standard base. Functions like `(+)` are replaced by `sum`.
- there are no lists or other GC'd data structures in the standard base (this may change based on the implementation of memory).

Remember, while the language is meant to be understood quickly, it's not meant to be a replacement for everything, Like Logo, I'll probably
eventually add something to "preprocess" source code and have operators (or simply make an `expr` form to handle it).

_Note_: I've been working on various ML-dialects for several years, and carML is the one I've decided to run with. Interestingly, 
[Tulip](http://tuliplang.org/) is also the product of several years of tinkering, and has a very similar syntax. It might be interesting
to combine the two (or use carML for the base instead of C), but it's exciting to me that there is movement in the ML space again, esp.
considering that both languages are _not_ derived from existing ML dialects. [Yeti](https://mth.github.io/yeti/) is no longer alone!

# Semantics

- typical ML/PowerLogo/Lisp semantics. 
- types, row polymorphism, polymorphic variants
- Minimal datatypes available from base. Need to figure out GC-less lists
- Deque & Vector ala Rust
- GC'd and GC-less data structures?
- Lambda lifting not closure conversion (to save GC)
- Regions + localized GC?
- Continuations? How would that operate in a GC-less environment?

# Output

- Enyalios-style Human Readable C/Golang
- Let the C/Golang compiler deal with many of the real optimizations
- But we can do some simple ones like inlining, self-TCO, & rewriting HOFs

# Examples

    # cannonical loop example
    def arrayIota arr:array[int] f:function[int => int] = begin
        def arrayIota' arr:array[int] fun:function[int => int] idx:int = begin
            if (< idx (array-length arr)) then {
                set-array-index! arr (fun idx) idx
                arrayIota' arr fun (add idx 1)
            } else () 
        end
        arrayIota' arr f 0
    end

    # note, there is no _real_ reason to use begin
    # form here, but it makes it a bit more pretty.
    def arrayIota1 arr : array[int] fun : function[int => int] = {
        foreach-index! fn x = (set-array-index! arr (fun x) x) arr
    }

    # same as the above, but without the `{}` block
    def arrayIota2 arr:array[int] fun : function[int => int] = (foreach-index! fn x = (set-array-index! arr (fun x) x) arr)

    # but really, all we're doing is mapping some function in place...
    def arrayIota3 arr:array[int] fun:function[int => int] = (map! fun arr)

    def foo x : int => int = (add x 1)

    # constant:
    val f : array[int] = (make-array 10)
    # variable
    var g : array[int] = [0 1 2 3 4 5 6 7 8 9]
    
    arrayIota f foo;
    
    println f;

    # add:
    # ref, make

 See also the _examples_ directory for more indepth & up-to-date examples.

# Implementations

 There are two major implementations:

- `carmlc` which is just a compiler, written in C.
- `carml` which is a self-hosting version.

Note that, when supplied with no arguments, `carmlc` will actually enter a REPL. However,
this REPL does not evaluate the supplied expressions for _values_, but rather for _code_:

    >>> def foo x = (sum x x)
    (define-function foo (x)
        (call (identifier sum) (identifier x) (identifier x)))

The purpose is more to see what the IR/code will look like, rather than what the code does. It 
is meant more as a test bed than an actual REPL.

# File naming conventions

There are a few different "types" of files in the system

- `*.c.carml` is a file that is written in carML, but targets C directly
- `*.go.carml` is a file written in carML, but targets Golang directly
- `*.carml` just a plain carML file, which makes no assumptions as to the underlying system

Eventually I'd like to be able to include files via an SRFI-0-style mechanism, by which we can
simply import which files we'd like to use at compile time based on flags and such

# License

See `LICENSE` for details (ISC license).
