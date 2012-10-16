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
  print e + "31m" + "Welcome to SmallBASIC"
  print e + "32m" + "Welcome to SmallBASIC"
  print e + "33m" + "Welcome to SmallBASIC"
  print e + "34m" + "Welcome to SmallBASIC"
  print e + "0m"
  space_print "Welcome to SmallBASIC"
end

sub icon() 
  print "..                  .                     ..."
  print "   7B@B@@@B@B@B@B@G   ,OB@@@B@B@B@B@B@B@i  .."
  print " vMB@@@B@B@u.,iO@B@B2 .O@@@B@@@B@;:,5B@@@Mv ."
  print " 5B@O@B@B@O.   M@@B@@, 5BMZ@B@B@7    ZB@@@U  "
  print " 5@BOB@B@BN     i7uXr .X@M@B@B@Bv    M@B@BS  "
  print " 7Z@B@B@B@@qivr.      ,OBBB@B@B@XvrruBB@Bu: ."
  print ".  ;@B@B@B@B@B@B@B@r  ,E@OBB@B@M@B@B@B@B2   ."
  print "               uB@B@M. SB@B@@@B@7   .MB@B@U  "
  print " 2@@@@@B@B@:   U@BBBG  5@B@B@B@Bj   :M@B@Bj ."
  print " GB@B@B@B@B    YB@B@B. 1B@B@B@B@i    8B@@@5  "
  print " .:O@B@B@B@B@B@B@B@u. :B@B@B@B@@@B@B@B@B@:. ."
  print "    iuukS5jXOO8U7v.    .i7rrrrr7vFkXuYv2   .."
  print " :v.              .:::,                 .7i ."
  print " FB@B@Oi iSGNZL, 7M@B@B@B@B@@@B@@@B@BMM@B@k. "
  print ".  v@Bk8q. ,FuuF7  :0B@PJr:.,i: ,ii..7@@u   ."
  print "..   2BM1Z2  :PFLUi  rO@Ok28MMOBM@F70@k    .."
  print "...   .X@OkFv  rEFLv,  Y@B@MOZOBNrPBO.  ....."
  print ".....   :GBOkui  7Ek7r  .kBPqM57u@Oi   ......"
  print ".......  .YM@GEYr:iJqFLi..i5GqOMOu:  ........"
  print ".........  :q@BMY. :LFk1:  :JqM8i  .........."
  print "...........  UO,  .  vL   . .Lr   ..........."
  print "............    ....    ....   .............."
end

sub logo()
  print "  / __/_ _  ___ _/ / / _ )/ _ | / __/  _/ ___/"
  print " _\ \/  ' \/ _ `/ / / _  / __ |_\ \_/ // /__  "
  print "/___/_/_/_/\_,_/_/_/____/_/ |_/___/___/\___/  "
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
  icon
  logo  
  intro
  buttons
  about
else 
  listFiles command
endif
