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


