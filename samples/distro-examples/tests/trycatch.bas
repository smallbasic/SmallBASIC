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

sub s1(err)
  try
    s2(err)
  catch "err1"
    ? "err1"
  catch "err2"
    ? "err2"
  catch "err3"
    ? "err3"
  end try
end
sub s2(err)
  try
    s3(err)
  catch "err4"
    ? "err4"
  catch "err5"
    ? "err5"
  catch "err6"
    ? "err6"
  end try
end
sub s3(err)
  if 1==1 then
    if 1==1 then
      if 1==1 then
         throw err
      endif
    endif
  endif
end

caughtError = FALSE
try
  s1("some string")
catch "some"
  caughtError = TRUE
end try

if (!caughtError) then
  throw "Error not caught!!!"
endif
