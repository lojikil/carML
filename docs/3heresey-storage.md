# Variable Storage is a pointless implementation detail (most of the time)

I have been thinking about this a lot lately: what is the difference between the following:

    def foo x:int => Either[int, string] = {
        ...
    }

    def bar x:int => ref[Either[int, string]] = {
        ...
    }

Assuming they undertake the same options? For most programmers, they do not *really* care that something
is a reference and something else is a value; that is an implementation detail they do not care about.
Eventually, I would like carML to get to the point where most of the time we do *not* care about such things,
until it is needed (for something like C interop). I run into it all the time, thinking about types and such,
and whilst the compiler should be able to adapt, there is no need to *have* to specify storage location for
most things (until you know you do). I think the thing that drives me most with this is:

    a low barrier to entry and no ceiling.

I want carML to be relatively consumable for most folks, but I would *also* like those hardcore few who are
looking to replace C or the like to be able to do so, all with the same language. Rust has done well there,
although, like ATS, the barrier to entry is still quite high (tho clearly nothing has as high a barrier as 
ATS does, goodness). 
