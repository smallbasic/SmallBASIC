'
'http://sourceforge.net/tracker/?func=detail&aid=1683450&group_id=22348&atid=375102
'
'The following code cause a syntax error:

func a(x)
 ? "Func a"
 func b(y)
  ? "Func b"
  select y
  case 1
 case else
 end select
 end
 func c(z)
 ? "Func c"
 end

 i = b(x)
 i = c(x)
end

x = a(1)
stop

'Error:
'
'Comp error at main 17
'
'Description:
'Undefined SUB/FUNC code C
'
'If we commented out the selec-case-endselect structure everything is fine.
'
'Probably the "end" from "end select" and function "end" has unintended
'coincidenc.
