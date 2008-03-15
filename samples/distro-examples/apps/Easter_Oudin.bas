'
'	Easter-date calculator
'
? "Easter by the method of "+chr$(27)+"[1mOudin"+chr$(27)+"[21m"
10 input "Enter a year : ",yr
if yr=0 then 99
tc = yr\100
tg = yr%19
tk = (tc-17)\25
tl = (tc-tc\4-(tc-tk)\3+19*tg+15)%30
tla=tl\28
tlb=29\(tl+1)
tlc=(21-tg)\11
tl=tl-tla*(1-tla*tlb)*tlc
tj=(yr+yr\4+tl+2-tc+tc\4)%7
tl=tl-tj
month=3+(tl+40)\44
day=tl+28-31*(month\4)
if month=3 then
 ? "Sunday, March ";day;", ";yr
else
 ? "Sunday, April ";day;", ";yr
fi
goto 10
99 end
