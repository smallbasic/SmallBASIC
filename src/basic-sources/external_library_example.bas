#!/usr/bin/sbasic -mexample1 -q
'
' External-module
'
' This example runs the command LIBTEST which is created as module (sync/modules/)
'
?
? "Remember you must call the sbasic with the option -m"
? "sbasic -mexample1 exteral_library_example.bas"
?
libtest 1920, "the second parameter"
? "Also, " + libfunctest
?
? "Ok"
