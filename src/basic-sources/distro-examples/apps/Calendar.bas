#sec:Main
'
'	Calendar (PalmOS version)
'
const box=18
const lmar=80-box*7/2
const tmar=34
dim ds(0 to 6),ms(12,2)
for i=0 to 6 : read ds(i) : next
data "Su","Mo","Tu","We","Th","Fr","Sa"
for i=1 to 2 : for j=1 to 12 : read ms(j,i)
next : next
data "January","February","March","April","May","June","July","August","September","October","November","December"
data 31,28,31,30,31,30,31,31,30,31,30,31
repeat
  at 3,148
  input "Month,Year? ",ln$
  cls
  if ln$=0 then stop
  split ln$,"/,",v
  if len(v)>2 then
    m=val(v(0))
    y=val(v(1))
    d=(1461*(y+4800+(m-14)\12)\4+367*(m-2-12*((m-14)\12))\12-3*((y+4900+(m-14)\12)\100)\4+1)%7
    ms(2,2)=28+(y%4=0 and (y%100<>0 or y%400=0))
    n=ms(m,1)+" "+str$(y)
    at 1+(160-txtw(n)-len(n))/2,tmar/2-txth(n)
    print cat(1);n;cat(0)
    rem # of rows
    i=(ms(m,2)+d+6)\7
    rem verticals
    for n=0 to 7 : line lmar+box*n,tmar,lmar+box*n,tmar+box*i : next
    rem horizontals
    for n=0 to i : line lmar,tmar+box*n,lmar+box*7,tmar+box*n : next
    for n=0 to 6
      at lmar+1+box*n+(box-txtw(ds(n)))/2,tmar-2-txth(ds(n))
      print ds(n)
    next
    for i=0 to ms(m,2)-1
      row=(d+i)\7
      col=(d+i)%7
      n=str$(i+1)
      at lmar+1+box*col+(box-txtw(n))/2,tmar+1+box*row+(box-txth(n))/2
      print n
    next
  endif
until
