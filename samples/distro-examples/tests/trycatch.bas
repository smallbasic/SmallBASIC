? "start"
try
  ? "inner try"
  if (1==1) then
    try
    catch
      ? "should never be printed"
    end try
    if (2==2) then
      try
       open "com2000:" AS #1
       try
         ? "should never be printed"
       catch
         ? "should never be printed"
       end try
       ? "should never be printed"
      catch "err"
       ? "should never be printed"
      catch err
       ? "open failed", err
      end try
      ? "after try
      throw "an error has occurred"
      ? "should never be printed"
    fi
  fi
  try
   ? "should never be printed"
  catch
   ? "should never be printed"
  end try
  open "com2000:" AS #1
  ? "should never be printed"
catch "some error that is not thrown"
  ? "should never be printed"
catch "an error has occurred"
  ? "catch by error name"
end try

? "outer after try"

rem
rem Test whether the stack is unwound correctly
rem

iter = 100
cnt = 0
i = 0

while (i < iter)
  try
    while (true)
      if (true)
        if (true)
           throw "foo"
        end if
      end if
    wend
  catch err
    select case err
    case "foo"
      cnt++
    end select
  end try
  i++
wend

if (cnt <> iter) then
  print "Test failed: "; cnt; " <> "; iter
end if

REM Test for multi-item case catching
select case "Cool"
 case "null", 1,2,3,4,5,6,7,8,"Cool","blah"
 case "not cool"
   throw "epic fail"
  case else
   throw "FAIL"
end select

try
 select case "Y"
  case "YES", "yes", "y", "Y"
    throw "okay"
 end select
 throw "bad"
catch "okay"
 REM okay
end try
