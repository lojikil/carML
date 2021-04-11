# thoughts on modules

I've been thinking a lot about what I want to do for modules. I really like how OCaml and SML handle this, but I think
they are quite complex. On the flip side, I think [Mythryl's take on Packages and APIs](https://mythryl.org/my-Packages_and_APIs.html) is
quite fascinating, but not really where I want to go. I think F# strikes a happy medium; you can have nested Modules, and they are
quite simple, but you can manipulate the environment to make things relatively clean (from user syntax perspective) as well.

Part of my thinking here is that carML already has mechanisms to parameterize types:

```
type Foo X {
    Bar int
    Blah X
}
```

Syntactically we match the semantic namespace of a constructor to it's type:

```
match x with
    (Foo.Bar y) => y
    else => 10
end
```

And the generated C/Golang code relies on this fact. That's not to say that we cannot break this, of course, but rather I do
like namespacing constructors, and only a few languages do this. So I thought I'd make a proposal to fix these with some syntax.

Part of _why_ I'm thinking about this now is that I want to add some simple functions to the base system that can be easily
rewritten for the various output types, but are nicely named. For example, whilst I can easily support `string-map!` currently,
it would be nice to define a module `Strings` that includes a `map!` function.

# Proposal

- add `::` and `:=` to syntax as operators, like `|>` and `$`
    - this proposal doesn't require `:=`, it's just if I do the work on `::` I may as well do `:=` too
- fix `use` to load modules from some set of paths
- support `Foo::bar` which means the `bar` element of module `Foo`
- add `module` and `open-module` as syntactic forms in the F# style
