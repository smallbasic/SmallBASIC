'
'	Day of week
'
dim days(0 to 6), mons(1 to 12)
for m=0 to 6 : read days(m) : next m
for m=1 to 12 : read mons(m) : next m
data "Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"
data "January","February","March","April","May","June","July","August","September","October","November","December"
split date$,"/",v
m=val(v(1))
d=val(v(0))
y=val(v(2))
?"Today is ";
goto 11
10 rem
input "Date ?  ",ln$
if ln$=0 then stop
split ln$,"/,",v
if ubound(v)>1 then if len(v(2)) then y=val(v(2))
if ubound(v)>0 then if len(v(1)) then d=val(v(1))
if len(v(0)) then m=val(v(0))
11 rem
?days((1461*(y+4800+(m-14)\12)\4+367*(m-2-12*((m-14)\12))\12-3*((y+4900+(m-14)\12)\100)\4+d)%7);", ";mons(m);" ";d;", ";y
goto 10

