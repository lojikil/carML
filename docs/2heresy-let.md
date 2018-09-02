# `let` signals intent about variable scope

I was just thinking about the notion of `let` vs `val`: with a `let`, we're signaling that we *do not* want this variable
to live on outside its scope. That intentionality is probably harmless in most cases; for example, if we have a function such as:

    function foo x:int =
        let y:int = (+ x 10) in
            (printf "y is %d\n" y)

we don't *actually* care what happens to `y` afterward, as the function terminates. However, what if we have something more nuanced?

    function bar x:int = {
        let y:int = (+ x 10) in
            (printf "y is %d\n" y)
        let y:int = (+ x 20) in
            (printf "y is %d\n" y)
    }

here, we're clearly signaling intent that `y` shouldn't outlive the scope of the enclosing `let`. This isn't *really* revolutionary
thinking, it's just interesting to me given that I am supporting both `let` and `val/var` in carML, and that support has an interesting
impact on how we code: the `let` form is clearly trivially rewritten to a fresh symbol, and is easily compsed into SSA via ANF, but a
`val` may have larger impact depending upon its exposure to other programs (and indeed, we may not want to do SSA/ANF for programs we
expect humans to consume...)
