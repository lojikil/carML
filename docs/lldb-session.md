# Just a good thing to note about LLDB

We can introspect AST objects nicely:

```
(lldb) expr (int) printf("%d %s\n", ((AST *)0x0000000100423a20)->tag, ((AST *)0x0000000100423a20)->value);
0 foreach-other
(int) $3 = 16
(lldb) expr (int) printf("%d %s %d\n", ((AST *)0x0000000100423a20)->tag, ((AST *)0x0000000100423a20)->value, ((AST *)0x0000000100423a20)->lenchildren);
0 foreach-other 3
(int) $4 = 18
(lldb) expr (int) printf("%d\n", ((AST *)0x0000000100423a20)->children[0]->tag)
26
(int) $5 = 3
(lldb) expr (int) printf("%d\n", ((AST *)0x0000000100423a20)->children[1]->tag)
1
(int) $6 = 2
(lldb) expr (int) printf("%d\n", ((AST *)0x0000000100423a20)->children[2]->tag)
error: Execution was interrupted, reason: Attempted to dereference an invalid pointer..
The process has been returned to the state before expression evaluation.
(lldb) expr (int) printf("%d\n", ((AST *)0x0000000100423a20)->children[1]->tag)
```
