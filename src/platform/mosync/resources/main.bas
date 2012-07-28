
sub slow_print(s)
  local ch, len_s
  len_s = len(s)
  for ch = 1 to len_s 
    ? mid(s, ch, 1) + " " + chr(3);
    delay 40
  next ch
end

sub intro()
  e = chr(27)+"["
  ? e+"90m"+e+"31m"+"Welcome to SmallBASIC"
  ? e+"91m"+e+"32m"+"Welcome to SmallBASIC"
  ? e+"92m"+e+"33m"+"Welcome to SmallBASIC"
  ? e+"93m"+e+"34m"+"Welcome to SmallBASIC"
  ? e+"0m"+e+"90m"
  slow_print "Welcome to SmallBASIC"
  ? chr$(27)+"[90m"
end

intro

print
print "Copyright (c) 2002-2012 Chris Warren-Smith"
print "Copyright (c) 2000-2006 Nicholas Christopoulos."
print
print "http://smallbasic.sourceforge.net"
print
print "SmallBASIC comes with ABSOLUTELY NO WARRANTY."
print "This program is free software; you can use it"
print "redistribute it and/or modify it under the terms of the"
print "GNU General Public License version 2 as published by"
print "the Free Software Foundation."
