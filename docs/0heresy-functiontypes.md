# Heresy 0: function/procedure types

_2018-MAR-11_

I was thinking today about adding code to parse types, and had a bit of an epiphany (or maybe a bit of a "fuck this"):

Why have special code to parse higher-order function types just to keep syntax similar to function declarations themselves?

The traditional answer to declaring HOFs in functional languages looks something like:

    name: (type0 type1 type2 ... => returnType)

where `name` is some binding and we have a list of types. I was thinking about the work I've been doing wrt Scala-style types,
and how much easier that makes parsing of type declarations: most of the `of` forms can disappear, and deciding if something
is a new tag or a type-member is as simple as reading an array literal. This got me to thinking about how much code I want to
remove, oh but damn I still need to add `(type...)` parsing for HOFs. Then I had a thought: doesn't Scala and several other
langauges support some notion of a `Function` type? Can't we do something similar?

## A Modest ~Proposal~ Heresy

Functions & procedures are just fancy objects and types about them should be simple. Introducing new parsing code just to
parse HOFs is overkill, as we can introduce two new types, `Function` and `Procedure`, which capture the notion of functions
and procedures (or, functions that do not return a value, Unit functions):

    name: Function[int int]

`name` is a function from `int` to `int`. For purposes of beauty and perhaps visual delineation, `=>` should work within, but
be ignored by the compiler:

    name: Function[int => int]

Functions that are only called for their side effects may be easily modeled two ways:

    doSomething: Function[int => ()]

That is to say, `doSomething` does something on integers and returns no value, or:

    doSomethingElse: Procedure[int]

Here `doSomethingElse` is a "procedure" (to get into Scheme-style "lambda calculus of procedures") over integers. Nullary
functions that return no values may be introduced with `Function[()]`, and procedures can be introduced simply with
`Procedure`, since the carML type parsing already assumes tagged words without other specification are types.

I need to masticate on this a bit more, but it's intriguing to me.
