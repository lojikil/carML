# Lexer bugs:

- single-character idents fail:

```
t
b
(identifier t
 b)
```

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
