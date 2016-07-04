# Lexer bugs:

- Single character identifiers that start with a keyword letter a broken ("b\nb" is seen as one identifier)

# keywords

- `lambda`
- `declare`
- `ref`
- `int`
- `string`
- `char`
- `bool`
- `array`
- `float`
- `match`
- `with`
- `use`
- `module`
- `case`
- `of`
- `define-alien`, `letrec`, `val-alien`

# Parser bugs:

None currently (save for missing functionality)

# Types:

## records:

Records are an interesting case... I do think that `val`-style typing should be used:

    record foo = {
        bar : int
        baz
    }

here, `foo.bar` has an explicit `int` type, whereas `baz` will be inferred from program
usage... The question remains then how we _allocate_ records. I'd prefer to unbox them
at all times, and even go so far as to do what C# does, and flatten records when passed
to functions.

Also, what is the accessor syntax? Haskell does what SRFI-9 does, and defines an accessor
function:

    data Person = Person { first_name :: String
                           , last_name :: String
                           , age :: Int 
                         } deriving (Eq, Ord, Show)

    print_age person = print $ age person
