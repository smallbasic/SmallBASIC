#!../../../src/platform/console/sbasic

' This file (socket_io_client.bas) will be called by socket_io.bas
' Shebang needs to link to correct sbasic file.

open "SOCL:127.0.0.1:10000" as #1

' Test bputc and bgetc
for byte = 0 to 255
    bputc #1, byte
next

' Test print
print #1, "Test1234"
print #1, "Test-1234"
print #1, "Test1234"

' Test EOF bug
byte = 0
bputc #1, byte

' Test LOF
print #1, "Test1234"

close #1

print "Client connection closed"
