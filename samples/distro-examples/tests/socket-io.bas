' socket-io.bas will start a socket client. The client will send data.
' socket-io.bas will check if data was received correctly

' make socket-io-client.bas executable and then execute it
chmod "../../../samples/distro-examples/tests/socket-io-client.bas", 0o777
exec "../../../samples/distro-examples/tests/socket-io-client.bas"

open "SOCL:10000" as #1

' Test bputc and bgetc
for ii = 0 to 255
    Recv = bgetc(1)
    if(Recv != ii) then throw "BGETC: " + ii + " expected, received " + Recv        
next

' Test print and input
input #1, ans
if(ans != "Test1234") then throw "INPUT: Test1234 expected, received " + ans

input #1, ans, "-", ans2
if(ans != "Test") then throw "INPUT: Test expected, received " + ans
if(ans2 != "1234") then throw "INPUT: 1234 expected, received " + ans2

ans = input(9, 1)
if(ans != "Test1234\n") then throw "INPUT: Test1234 expected, received " + "\"" + ans + "\""    

' Test for bug in SB 12.25: when "0" is received, EOF should not return true
ans = bgetc(1)
if(EOF(1)) then throw "EOF: zero received and eof returns true"

' Test LOF
if(LOF(1) != 9) then throw "LOF: 9 bytes expected. " + LOF(1) + " bytes waiting."

close #1

print "Server connection closed"
