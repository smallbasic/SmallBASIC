' 
'	Using LOADLN,SPLIT
'
'	This program displays an CSV file
'
TLOAD "mycsv.csv", A
PRINT "Name";TAB(30);"e-mail"
PRINT STRING$(47,"-")
FOR L IN A
  SPLIT L, ",", B USE TRIM(x)
  IF LEN(B)<2 THEN EXIT ' For empty lines
  PRINT B(0);TAB(30);B(1)
NEXT
END
