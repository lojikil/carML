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


This is at least the 3rd time (and probably 5th) time that I've attempted such a thing, so don't mind me at all. This *is* the first time
I've experimented publically with my PLT designs tho.

# Name

I've experimented with multiple names; HotelML (named for "The HotelML" in NJ) & Melomys (a specious of mouse) were two contenders. However,
my [partner](https://twitter.com/foiltheplot) and I were in NYC, where we grabbed coffee from Starbucks. She's a big fan of caramel 
macchiattos, which Starbucks labels as "carml mac", and thus I've named it after her.

# Syntax

 Syntactically, it's pretty much a garden variety ML dialect. Minimal keywords like `def` exist to visually break up code, but otherwise
if you're familiar with Yeti, StandardML, OCaml, &c. it should be relatively straight forward. The one location that it is *not* is in the
standard base.

- there are no operators in the standard base. Functions like `(+)` are replaced by `sum`.
- there are no lists or other GC'd data structures in the standard base (this may change based on implementation of the memory).

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

- Enyalios-style Human Readable C
- Let the C compiler deal with many of the real optimizations
- But we can do some simple ones like inlining, self-TCO, & rewriting HOFs

# Examples

    # cannonical loop example
    def arrayIota arr f = begin
        def arrayIota' arr fun idx = begin
            if (< idx (array-length arr)) then {
                set-array-index! arr (fun idx) idx
                arrayIota' arr fun (add idx 1)
            } else ()
        end
        arrayIota' arr f 0
    end
    
    # same as the above, but here we
    # use the closure in arrayIota' to
    # capture variables, instead of
    # explicitly passing them in
    declare arrayIota0 array of Num (Num -> Num) -> ()
    def arrayIota0 arr fun = begin
        def arrayIota' idx = begin
            if (< idx (array-length arr)) then {
                set-array-index! arr (fun idx) idx;
                arrayIota' (add idx 1);
            } else ()
        end
        arrayIota' 0;
    end

    # same as the above, but using `foreachIndex`
    # note, there is no _real_ reason to use begin
    # form here, but it makes it a bit more pretty.
    @arrayIota1 array of Num (Num -> Num) -> ()
    def arrayIota1 arr fun = {
        foreachIndex fn x = (set-array-index! arr (fun x) x) arr
    }

    # same as the above, but without the `{}` block
    @arrayIota2 array of Num (Num -> Num) -> ()
    def arrayIota2 arr fun = (foreachIndex fn x = (set-array-index! arr (fun x) x) arr)

    # same as the above, but using `letrec`
    @arrayIota3 array of Num (Num -> Num) -> ()
    def arrayIota3 arr fun = {
        letrec internalIota = fn idx = {
            set-array-index! arr (fun idx) x
            internalIota (sum idx 1)
        } in
        internalIota 0
    }

    # eventually, `def` forms should work
    # like `begin` forms
    def foo x = (add x 1)
     
    val f = (array 10)
    
    arrayIota f foo;
    
    println f;

    # add:
    # ref, make

# License

See `LICENSE` for details (ISC license).
