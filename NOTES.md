# Lexer bugs:

- doesn't appear to see the semi-colon in here:

```
$ ./c29
>>> def arrayIota arr f = begin
    def foo idx = begin
        set-array-index! arr idx f ;
        foo (add idx 1) ;
parse error: illegal semicolon within parenthetical call
>>> (tag 29)

>>> def arrayIota arr f = begin
    def foo idx = begin
        set-array-index! arr idx f ;
        foo (add idx 1 ) ;
parse error: illegal semicolon within parenthetical call
>>> (tag 29)

>>> def arrayIota arr f = begin
    def foo idx = begin
        set-array-index! arr idx f ;
        foo ( add idx 1 )
    end
    foo 10 ;
parse error: illegal semicolon within parenthetical call
>>> (tag 29)

>>> 
```

- single-charater numbers & idents fail:

```
0
1
(identifier 0
 1)
t
b
(identifier t
 b)
```

- something weird with semi-colons:

```
$ ./c29
>>> (this is a test )
(call (identifier this)(identifier is)(identifier a)(identifier test))
>>> (tag 29)

>>> (this i a test)
(call (identifier this)(identifier i a)(identifier test))
>>> (tag 29)

>>> begin
sum 11 22   
sum (div 11 22) 33
end
end
end
end
end
end
end
)
end
(begin 1
     (call (identifier sum)(integer 11)(integer 22)(identifier sum)(call (identifier div)(integer 11)(identifier 22)(integer 33)))
)

>>> 
```

# keywords

-`lambda`
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

# Parser bugs:

- `begin` form does not properly break on TNEWL:

_This might actually be a lexer bug too... testing with TSEMI & TEND work as expected :| _

```
$ ./c29
>>> def foo x = begin
    println x  
    sum (div x 100 ) 10
end
(define foo (parameter-list (identifier x)) 
     (begin 4
          (call (identifier println)(identifier x))
          (identifier sum)
          (call (identifier div)(identifier x)(integer 100))
          (integer 10)))

>>> def foo x = begin
    println x ;
    sum (div x 100 ) 10 ;
end
(define foo (parameter-list (identifier x)) 
    (begin 2
        (call (identifier println)(identifier x))
        (call (identifier sum)(call (identifier div)(identifier x)(integer 100))(integer 10))))
```
