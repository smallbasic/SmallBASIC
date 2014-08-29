OPTION PREDEF QUIET
OPTION PREDEF COMMAND "hello world"
OPTION BASE 1

DIM a(3)
? lbound(a), ubound(a)
redim a(4)
? lbound(a), ubound(a)
DIM a()
? lbound(a), ubound(a)

OPTION BASE 0
DIM a(3)
? lbound(a), ubound(a)

? "COMMAND$="; command$


