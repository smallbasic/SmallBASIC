const app = "main.bas?"

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
  print e + "http://smallbasic.sourceforge.net/?q=export/code/1102|Online programs" + chr(28);
  print " " + e + app + "/|Open file" + chr(28) + chr(10)
end

sub listFiles(path)
  local fileList, ent, esc, basList, dirList, name, backPath, index

  dim basList
  dim dirList

  backPath = ""
  index = iff(isstring(path), rinstr(path, "/"), 0)
  if (index > 1) 
    backPath = left(path, index - 1)
  endif

  if (right(path, 1) != "/") then
    path += "/"
  endif

  print "Files in " + path
  
  esc = chr(27) + "[ H"
  fileList = files(path)
  
  for ent in fileList
    name = ent
    if (isdir(path + name)) then
      dirList << name
    else if (right(ent, 4) == ".bas") then
      basList << name
    endif
  next ent
  
  sort dirList
  sort basList

  if (path != "/") then
    print " " + chr(27) + "[ B" + app + backPath + "|Back" + chr(28)
  endif    

  for ent in dirList
    print " " + esc + app + path + ent + "|[" + ent + "]" + chr(28)
  next ent

  print chr(27) + "[1;32m";
  for ent in basList
    print " " + esc + path + ent + "|" + ent + chr(28)
  next ent
end

if (command == "welcome") then
  intro
  buttons
  about
else 
  listFiles command
endif
