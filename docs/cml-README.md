# Overview

 A simple, runtime-less, GC-less ML dialect that compiles to Human-Readable C. Syntactically, it is most similar to
ATS & Yeti, and a previous project I had worked on called "Spearhead".

# Rationale 

 I wanted a hyper-minimal ML dialect for system work, but something simpler than ATS. While I can, and do, use 
PreDigamma (my restricted Scheme dialect), having a super small ML dialect to throw around isn't bad either.
Additionally, it allows me to experiment with items I find interesting in a GC-less environment (PreDigamma still
requires the use of a GC, for now).

# Syntactic Forms

- `{ ... }` a begin/end pair, with statements separates by `;`
- `match ... with ... chtam` a match form, with guard-clauses, binding, and separated by '|'
- `if ... then ... else ...` No single-armed if
- `when ...` Single-armed if
- `val ... = ...` a variable binding
- `let ... in ...` name binding
- `define ... (parameters ...) = ...` function binding
- `fn (parameters ...) = ...` Lambda
- `define-alien ... (parameters ...)` : CFFI
- `val-alien ...` extern C variable
- `define-record ... is ... end` record definitions
- `define-variant ... = ...` (polymorphic) variant definitions

# Default Library

- Strings: string-length, string-index, string-string, string-substring, string-match, string-glob
- Vectors: vector-length, vector-index, vector-subvector
- GC: region-gc, region-alloc, region-free

# C Interaction

# Examples

