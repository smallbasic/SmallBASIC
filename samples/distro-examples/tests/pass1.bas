OPTION PREDEF QUIET
OPTION PREDEF TEXTMODE
OPTION PREDEF COMMAND FOO

if (command <> "FOO") then
  print "ERROR"; " "; command
else 
  print "OK"
end if

SBASICPATH = "/foo"
if ENV("SBASICPATH") != "/foo" then
  print "ERROR" + " WAS:" + ENV("SBASICPATH")
else 
  print "OK"
end if
