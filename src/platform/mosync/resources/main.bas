const app = "main.bas?"
const exitLinkType = "exit_link"
const exitButtonType = "exit_button"
const linkType = "link"

sub space_print(s)
  local ch, len_s
  len_s = len(s)
  for ch = 1 to len_s 
    print mid(s, ch, 1) + " ";
  next ch
end

sub intro()
  local i
  for i = 1 to 4
    color i, 0: print "Welcome to SmallBASIC"
  next i
  color 7, 0
  space_print "Welcome to SmallBASIC"
  print ""
end

sub about()
  local bn_ok
  cls
  print "  / __/_ _  ___ _/ / / _ )/ _ | / __/  _/ ___/"
  print " _\ \/  ' \/ _ `/ / / _  / __ |_\ \_/ // /__  "
  print "/___/_/_/_/\_,_/_/_/____/_/ |_/___/___/\___/  "
  print
  print "Version 0.11.2"
  print
  print "Copyright (c) 2002-2012 Chris Warren-Smith"
  print "Copyright (c) 2000-2006 Nicholas Christopoulos." + chr(10)
  print "http://smallbasic.sourceforge.net" + chr(10)
  print "SmallBASIC comes with ABSOLUTELY NO WARRANTY."
  print "This program is free software; you can use it"
  print "redistribute it and/or modify it under the terms of the"
  print "GNU General Public License version 2 as published by"
  print "the Free Software Foundation."
  print 
  color 10, 8
  button xmax / 2, ypos * txth("A"), 0, 0, bn_ok,  "OK"
  doform
  color 7, 0
  cls
end

sub listFiles(path, byref basList, byref dirList)
  local fileList, ent, esc, name, lastItem

  erase basList
  erase dirList
  
  if (right(path, 1) != "/") then
    path += "/"
  endif

  color 7, 0
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

  color 3, 0
  lastItem = len(dirList) - 1
  for i = 0 to lastItem
    button 0, -1, 0, 0, dirList(i), "[" + dirList(i) + "]", linkType
    dirList(i) = path + dirList(i)
    print
  next ent

  color 2, 0
  lastItem = len(basList) - 1
  for i = 0 to lastItem
    button 0, -1, 0, 0, basList(i), basList(i), exitLinkType
    basList(i) = path + basList(i)
    print
  next ent
end

sub main
  local basList, dirList, path
  local form_var, bn_back, bn_about, bn_online
  local y_height, do_intro

  dim basList
  dim dirList

  form_var = ""
  bn_back = "_back"
  bn_about= "_about"
  bn_online = "http://smallbasic.sourceforge.net/?q=export/code/1102"
  y_height = txth(about_button) + 5

  sub make_ui(path, welcome)
    color 10, 8
    button 0,  0, 0, 0, bn_back,   "Back"
    button -4, 0, 0, 0, bn_online, "Online", exitButtonType
    button -4, 0, 0, 0, bn_about,  "About"
    at 0, y_height
    if (welcome) then
      intro
    fi
    listFiles path, basList, dirList        
  end

  sub go_back
    local backPath, index
    backPath = ""
    index = iff(isstring(path), rinstr(path, "/"), 0)
    if (index > 0 && index == len(path)) then
      index = rinstr(left(path, index - 1), "/")
    fi
    if (index == 1) then
      index++
    fi
    if (index > 0) 
      backPath = left(path, index - 1)
    else 
      backPath = "/"
    endif
    path = backPath
  end

  do_intro = false
  if (command == "welcome") then
    do_intro = true
  fi
  path = cwd

  make_ui path, do_intro

  while 1
    doform form_var
    cls

    if (isdir(form_var)) then
      path = form_var    
      chdir path
      make_ui path, false
    elif form_var == "About" then
      about
      make_ui path, false
    elif form_var == "Back" then
      go_back
      make_ui path, false
    fi
  wend
end

main
