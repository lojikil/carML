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
