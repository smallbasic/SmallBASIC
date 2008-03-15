'
'	Easter date calculator
'
? "Easter by the method of "+chr$(27)+"[1mCarter"+chr$(27)+"[21m"
10 input "Enter a year : ",yr
if yr=0 then 99
if yr<1900 or yr>2099 then 89
tb=225-11*(yr%19)
td=(tb-21)%30+21
te=(yr+yr\4+td+1)%7
day=td+7-te
if day<32 then
 ? "Sunday, March ";day;", ";yr
else
 ? "Sunday, April ";day-31;", ";yr
fi
goto 10
89 ? "1899 < year < 2100"
goto 10
99 end

