Just a quick note for tuples:

```
func bar() struct {string; int; } {
	return struct {string; int;}{"test", 10,}
}
```

This also is a huge gotcha in go:

```
// this works
return struct {string
int
}{"test", 10,}

// this is an error
return struct {string int}{"test", 10}

// because you need to have semicolons in there
```
