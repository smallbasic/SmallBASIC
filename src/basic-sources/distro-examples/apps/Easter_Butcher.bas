'
'	Easter-date calculator
'
? "Easter by the method of "+chr$(27)+"[1mButcher"+chr$(27)+"[21m"
10 input "Enter a year : ",yr
if yr=0 then 99
ta=yr%19
tb=yr\100
tc=yr%100
td=tb\4
te=tb%4
tf=(tb+8)\25
tg=(tb-tf+1)\3
th=(19*ta+tb-td-tg+15)%30
ti=tc\4
tk=tc%4
tl=(32+2*te+2*ti-th-tk)%7
tm=(ta+11*th+22*tl)\451
day=th+tl-7*tm+114
mon=day\31
day=day%31+1
if mon=3 then
 ? "Sunday, March ";day;", ";yr
else
 ? "Sunday, April ";day;", ";yr
fi
goto 10
99 end


