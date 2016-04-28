'
' Test for refactoring regression
'
blue=1
red=2
green=3

if false then blue=4:red=5:green=6

if (1 <> blue OR red <> 2 OR green <> 3) then
  throw "Inline IF error: blue=" + blue + " red=" + red + " green=" + green
endif

' normal if
if 1 then:? "Normal IF - Ok":else:? "ERROR":fi

' inline if
if 1 then ? "Inline IF - Ok"
if 0 then ? "ERROR" else ? "Ok"
if 1 then ? "Ok" else ? "ERROR"
if 1 then 500 else 600
200
if 0 then 500 else 600
210
if 1 then if 1 then if 1 then ? "3 inline IFs"
if 1 then if 1 then if 0 then ? "ERROR: Not supported, yet!" else ? "Ok"
if 1 then if 0 then if 1 then ? "ERROR: Not supported, yet!" else ? "ERROR: Not supported, yet!"
? IF(1,"True","False")
end

500
? "label 500 - Ok"
goto 200   

600
? "label 600 - Ok"
goto 210


