'
' Tests for CHAIN
'

chain "print \"error1\""
chain "throw \"error2\""
chain "throw \"error3\""
chain "throw \"error4\""

dim ar_code
ar_code << "for i=0 to 10"
ar_code << " print i"
ar_code << "next i"
chain ar_code

chain "throw \"error5\""

Const FILENAME = "chain.tmp"

Sub open_1
  Open FILENAME For Output As #1
End Sub

' ERROR: Missing ')' (does not read last character):
open_1
? #1, "Print Pow(2, 1)";
Close #1
Chain FILENAME

' OK: prints 4 (last character is ":"):
open_1
? #1, "Print Pow(2, 2):";
Close #1
Chain FILENAME

' OK: prints 8 (last character is \n):
open_1
? #1, "Print Pow(2, 3)"
Close #1
Chain FILENAME

' BUG?: prints 16 (reads ONLY the first line,
'      should read file as Array - not as String...):
open_1
? #1, "Print \"POW 2,4\""
? #1, "Print Pow(2, 4)"
? #1, "Print Pow(2, 5)"
? #1, "Print \"done\""
Close #1
Chain FILENAME

' OK: prints 64 128 (but as a String - not as an Array...):
?
open_1
? #1, "Print Pow(2, 6):";
? #1, "Print Pow(2, 7):";
Close #1
Chain FILENAME

' OK: causes an ERROR: Variable is NOT an Array (use DIM)
open_1
? #1, "Print Power(2, 1)" ' (Power instead of Pow)
Close #1
Chain FILENAME

' BUG?: does not report an ERROR (entering endless/long loop - ONLY IF previous "Chain FILENAME" is uncommented):
Chain "Print Power(2, 1)" ' using a "string" instead of filename.

kill FILENAME
