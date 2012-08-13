sub space_print(s)
  local ch, len_s
  len_s = len(s)
  for ch = 1 to len_s 
    print mid(s, ch, 1) + " ";
  next ch
end

sub intro()
  local e
  e = chr(27) + "["
  print e + "90m" + e + "31m" + "Welcome to SmallBASIC"
  print e + "91m" + e + "32m" + "Welcome to SmallBASIC"
  print e + "92m" + e + "33m" + "Welcome to SmallBASIC"
  print e + "93m" + e + "34m" + "Welcome to SmallBASIC"
  print e + "0m" + e + "90m"
  space_print "Welcome to SmallBASIC"
  print chr$(27) + "[90m"
end

sub about()
  print "Copyright (c) 2002-2012 Chris Warren-Smith"
  print "Copyright (c) 2000-2006 Nicholas Christopoulos." + chr(10)
  print "http://smallbasic.sourceforge.net" + chr(10)
  print "SmallBASIC comes with ABSOLUTELY NO WARRANTY."
  print "This program is free software; you can use it"
  print "redistribute it and/or modify it under the terms of the"
  print "GNU General Public License version 2 as published by"
  print "the Free Software Foundation."
end

sub buttons() {
  local e
  e = chr(27) + "[ B"
  print chr(10);
  print e + "http://smallbasic.sourceforge.net/?q=export/code/70|Online programs" + chr(28);
  print " " + e + "filemgr.bas|Open file" + chr(28) + chr(10)
end

intro
buttons
about
