rem test "OUT OF ADDRESS SPACE" error with incorrect kw optimisation
goto 50
a=b
50 a=b

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
