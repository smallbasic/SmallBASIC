rem test "OUT OF ADDRESS SPACE" error with incorrect kw optimisation
goto 50
a=b
50 a=b

rem a terrible alternative to select/case
a = 1
100
ON a GOTO 110,120,130
110 a = 2:GOTO 100
120 a = 3:GOTO 100
130

for i=0 to 5
	on i gosub 10,20, 30 , 40
next i
end

10 ? "10"
return
20 ? "20"
return
30 ? "30"
return
40 ? "40"
return
