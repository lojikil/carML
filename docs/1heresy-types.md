# 1 Heresy: More thoughts on types

_2018-APR-01_

Just to cut the question short first, no this isn't April Fool's, and no, it's not an Easter discussion either, as I'm Orthodox. However,
I've been thinking about types more again today, in light of some linearization work I was doing on the type processing in carML currently,
as well as my [previous heresy on function types](0heresy-functiontypes.md). Currently, complex types don't *know* their parameterized types.
For example: `array` is technically equivalent to `array[Any]`. This leads to some complexity in the parser: because we allow for `array`, we
have to jump through some hoops to parse the type definitions. This is to make things like `array` come out to C as `void *`, but is the 
short syntax _really_ worth it? Thinking about it further, it seems like the shortcut isn't really worth the trouble:

- it increases our ambiguity in parsing, and leads to the large state machine I currently use to decide syntax transforms at automatons
- I think it _decreases_ readability, and is better written as `array[Any]` or `deque[Any]`

So I think that's the best: I'll make complex types "aware" of their parameterized types, such that an `array` *must* be followed by an
array-literal of types (for example). This will cut down on the type parsing code (yay less code!), and will mean that users must be
more explicit about the types the specify to the demand system (tho for obvious reasons the demand system will deman/infer a type when
one is *not* specified by the user). 
