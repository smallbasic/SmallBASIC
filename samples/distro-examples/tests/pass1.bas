OPTION PREDEF QUIET
OPTION PREDEF TEXTMODE
OPTION PREDEF COMMAND FOO

if (command <> "FOO") then
  print "ERROR"
else 
  print "OK"
end if

UNITPATH = "/foo"
if ENV("UNITPATH") != "/foo" then
  print "ERROR" + " WAS:" + ENV("UNITPATH")
else 
  print "OK"
end if

