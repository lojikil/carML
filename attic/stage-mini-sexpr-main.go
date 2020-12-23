// this is a Go fragment that... calls code compiled by
// the carML compiler to run a simple S-Expression parser
//
// the point here is mostly to show how this would work,
// but it's also useful in my testing, to show that we
// can easily integrate the two
//
// to use this, compile mini-sexpression.carml with the
// Golang flag (`+g`) and append this file to the end of
// the result, adding `package main` and `import "fmt"`
// to the top of the resulting pass. This is literally
// how I've been testing the compiler...
func main() {
    var token Token
    src := "(call (identifier +) (integer 10) (integer 20))"
    idx := 0
    offset := 0
    fmt.Printf("src: %s\n", src)
    for ; idx < 15; idx++ {
        token = next(src, offset)
        offset = token.lexeme_offset + len(token.lexeme)
        fmt.Printf("token: %s, offset: %d\n", token.lexeme, offset)
    }

    result := read_list(src, 0)

    fmt.Printf("result: %T%v\n", result, result)

    roundtrip := sexpression2string(result)

    fmt.Printf("round trip? %s\n", roundtrip)
}
