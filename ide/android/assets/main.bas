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
  print " __           _      ___ _"
  print "(_ ._ _  _.|||_) /\ (_ |/ "
  print "__)| | |(_||||_)/--\__)|\_"
  print
  print "Version 0.11.4"
  print
  print "Copyright (c) 2002-2013 Chris Warren-Smith"
  print "Copyright (c) 2000-2006 Nic Christopoulos" + chr(10)
  print "http://smallbasic.sourceforge.net" + chr(10)
  print "SmallBASIC comes with ABSOLUTELY NO WARRANTY. ";
  print "This program is free software; you can use it ";
  print "redistribute it and/or modify it under the terms of the ";
  print "GNU General Public License version 2 as published by ";
  print "the Free Software Foundation." + chr(10)
  print "Envy Code R Font v0.8 used with permission ";
  print "http://damieng.com/envy-code-r" + chr(10)
  print 
  color 10, 8
  button xmax / 2, ypos * txth("A"), 0, 0, bn_ok,  "OK"
  doform
  color 7, 0
  cls
end

sub setup()
  color 3,0
  print cat(1) + "Setup web service port number." + cat(0)
  print
  print "Enter a port number to allow web browser or desktop IDE access. ";
  print "Enter -1 to diable this feature, or press <enter> to leave ";
  print "this screen without making any changes."
  print "Note: You must restart SmallBASIC for changes to take effect. ";
  print "The current setting is: " + env("serverSocket")
  print
  color 15,3
  input socket
  if (len(socket) > 0) then
    env("serverSocket=" + socket)
    randomize timer
    token = ""
    for i = 0 to 8
      token += chr (asc("A") + ((rnd * 1000) % 20))
    next i
    env("serverToken=" + token)
  endif
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
  bn_setup= "_setup"
  bn_online = "http://smallbasic.sourceforge.net/?q=export/code/1102"
  y_height = txth(about_button) + 10

  sub make_ui(path, welcome)
    color 10, 8
    button 0,  0, 0, 0, bn_back,   "Go up"
    button -4, 0, 0, 0, bn_online, "Online", exitButtonType
    button -4, 0, 0, 0, bn_setup,  "Setup",
    button -4, 0, 0, 0, bn_about,  "About"
    at 0, y_height
    if (welcome) then
      intro
      
      serverSocket = env("serverSocket")
      if (len(serverSocket) > 0) then
        print cat(0)
        print "Web Service port: " + cat(1) + serverSocket + cat(0)
        print "Access token: " + cat(1) + env("serverToken") + cat(0)
        print
      fi
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
    elif form_var == "Setup" then
      setup
      make_ui path, false
    elif form_var == "Go up" then
      go_back
      make_ui path, false
    fi
  wend
end

main
