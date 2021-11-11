# Overview

CarML has no operator precedence, but something I've been thinking about is how to make it nicer. We often end up with
pipelines like: `somelambda $ foo $ bar (get baz 10) 20` which, whilst not terrible, could be nicer for some operations.
Consider arithmetic: currently in carML, adding three numbers is `(+ 1 $ + 2 3)`. Again, not terrible, but...

There [was an article on lobste.rs talking about operator precedence](https://lobste.rs/s/jol24u/better_operator_precedence) which
had [an interesting comment](https://lobste.rs/s/jol24u/better_operator_precedence#c_bwd4ij):

> In Verona, we’re experimenting with no operator precedence. a op1 b op2 c is a parse error. You can chain sequences of the same operator but any pair of different operators needs brackets. You can write (a op1 b op1 c) op2 d or a op1 b op1 (c op2 d) and it’s obvious to the reader what the order of application is. This is a bit more important for us because we allow function names to include symbol characters and allow any function to be used infix, so statically defining rules for application order of a append b £$%£ c would lead to confusion.

This maps what I've been thinking about (no operator precedence, just use brackets) but does make it much nicer to have arithmetic pipelines: the example
above would just become `(1 + 2 + 3)`. I'd still have to implement *some* form of Shunting Yard for this, but I should do that anyway (and move away
from the current approach, which is just to insert some `call` forms when we see a `$`). This would also help sort out what to do with `$` vs `|>`.
