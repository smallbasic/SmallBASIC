const app = "main.bas?"
const boldOn = chr(27) + "[1m"
const boldOff = chr(27) + "[21m"
const lineSpacing = 2 + txth("Q")
const onlineUrl = "http://smallbasic.sourceforge.net/?q=export/code/1243"

func spaced(s)
  local ch, len_s
  len_s = len(s)
  local out = ""
  for ch = 1 to len_s
    out += mid(s, ch, 1) + " "
  next ch
  spaced = out
end

func mk_bn(value, lab, fg)
  local bn
  bn.x = 0
  bn.y = -lineSpacing
  bn.value = value
  bn.label = lab
  bn.color = fg
  mk_bn = bn
end

func mk_menu(value, lab, x)
  local bn
  bn.x = x
  bn.y = 0
  bn.value = value
  bn.label = lab
  bn.color = 7
  bn.backgroundColor = 1
  bn.type = "tab"
  mk_menu = bn
end

sub intro(byref frm)
  local i, bn
  for i = 1 to 4
    bn = mk_bn(0, "Welcome to SmallBASIC", i)
    bn.type = "label"
    frm.inputs << bn
  next i
  bn = mk_bn(0, spaced("Welcome to SmallBASIC"), 7)
  bn.type = "label"
  frm.inputs << bn
end

sub do_about()
  local frm, button
  cls
  print " __           _      ___ _"
  print "(_ ._ _  _.|||_) /\ (_ |/ "
  print "__)| | |(_||||_)/--\__)|\_"
  print
  print "Version 0.12.4"
  print
  print "Copyright (c) 2002-2015 Chris Warren-Smith"
  print "Copyright (c) 1999-2006 Nic Christopoulos" + chr(10)
  print "http://smallbasic.sourceforge.net" + chr(10)
  print "SmallBASIC comes with ABSOLUTELY NO WARRANTY. ";
  print "This program is free software; you can use it ";
  print "redistribute it and/or modify it under the terms of the ";
  print "GNU General Public License version 2 as published by ";
  print "the Free Software Foundation." + chr(10)
  print "Envy Code R Font v0.8 used with permission ";
  print "http://damieng.com/envy-code-r" + chr(10)
  print
  serverInfo

  button.x = xmax / 2
  button.y = ypos * lineSpacing
  button.label = "Close"
  button.backgroundColor = 8
  button.color = 10
  frm.inputs << button
  frm = form(frm)
  frm.doEvents()
  cls
end

sub do_newfile()
  color 3, 0
  cls
  print boldOn + "Create new program."
  print boldOff + "To enable editing, display the menu then select Editor [ON]."
  print "Press <enter> to leave this screen without making any changes."
  print
  local valid_file = false
  while (!valid_file)
    input "Enter file name: ", file
    if (len(file) == 0) then
      exit loop
    endif
    if (leftoflast(file, ".bas") == 0) then
      file += ".bas"
    endif
    try
      if (exist(file)) then
        print "File " + file + " already exists"
      else
        dim text
        text << "REM SmallBASIC"
        text << "REM created: " + date
        tsave file, text
        valid_file = true
      endif
    catch e
      print "Error creating file: " e
    end try
  wend
  color 7, 0
  cls
end

sub do_setup()
  color 3, 0
  cls
  print boldOn + "Setup web service port number."
  print boldOff
  print "Enter a port number to allow web browser or desktop IDE access. ";
  print "Enter -1 to disable this feature, or press <enter> to leave ";
  print "this screen without making any changes."
  print "The current setting is: " + env("serverSocket")
  print
  color 15, 3
  input socket
  if (len(socket) > 0) then
    env("serverSocket=" + socket)
    randomize timer
    token = ""
    for i = 1 to 6
      token += chr (asc("A") + ((rnd * 1000) % 20))
    next i
    env("serverToken=" + token)
    local msg = "You must restart SmallBASIC for this change to take effect"
    local wnd = window()
    wnd.alert(msg, "Restart required")
  endif
  color 7, 0
  cls
end

sub serverInfo()
  local serverSocket = env("serverSocket")
  local ipAddr = env("IP_ADDR")

  if (len(serverSocket) > 0 && len(ipAddr)) then
    serverSocket = ipAddr + ":" + serverSocket
    print boldOff + "Web Service: " + boldOn + serverSocket
    print boldOff + "Access token: " + boldOn + env("serverToken")
    print boldOff
  fi
end

func fileCmpFunc(l, r)
  local f1 = lower(l)
  local f2 = lower(r)
  fileCmpFunc = IFF(f1 == f2, 0, IFF(f1 > f2, 1, -1))
end

sub listFiles(byref frm, path, byref basList, byref dirList)
  local fileList, ent, name, lastItem, bn, bn_back

  erase basList
  erase dirList

  if (right(path, 1) != "/") then
    path += "/"
  endif

  bn = mk_bn(0, "Files in " + path, 7)
  bn.type = "label"
  bn.x = 0
  bn.y = -lineSpacing
  frm.inputs << bn

  bn_back = mk_bn("_back", "[Go up]", 3)
  bn_back.type = "link"
  bn_back.x = 0
  bn_back.y = -lineSpacing
  frm.inputs << bn_back

  fileList = files(path)

  for ent in fileList
    name = ent
    if (isdir(path + name) && left(name, 1) != ".") then
      dirList << name
    else if (lower(right(ent, 4)) == ".bas") then
      basList << name
    endif
  next ent

  sort dirList use fileCmpFunc(x,y)
  sort basList use filecmpfunc(x,y)

  lastItem = len(dirList) - 1

  for i = 0 to lastItem
    bn = mk_bn(path + dirList(i), "[" + dirList(i) + "]", 3)
    bn.type = "link"
    frm.inputs << bn
  next ent

  lastItem = len(basList) - 1
  for i = 0 to lastItem
    bn = mk_bn(path + basList(i), basList(i), 2)
    bn.type = "link"
    bn.isExit = true
    frm.inputs << bn
  next ent
end

sub main
  local basList, dirList, path
  local frm, bn_about, bn_online, bn_new
  local do_intro

  dim basList
  dim dirList

  bn_setup = mk_menu("_setup", "Setup", -1)
  bn_new = mk_menu("_new", "New", -1)
  bn_about = mk_menu("_about", "About", -1)
  bn_online = mk_menu(onlineUrl, "Online", 0)
  bn_online.isExit = true

  func make_ui(path, welcome)
    local frm
    frm.inputs << bn_online
    if (osname != "SDL") then
     frm.inputs << bn_setup
    endif
    frm.inputs << bn_new
    frm.inputs << bn_about

    if (welcome) then
      intro(frm)
    fi

    listFiles frm, path, basList, dirList
    frm.color = 10
    rect 0, 0, xmax, lineSpacing COLOR 1 filled
    at 0, 0
    make_ui = form(frm)
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
  frm = make_ui(path, do_intro)

  while 1
    frm.doEvents()

    if (isdir(frm.value)) then
      frm.close()
      path = frm.value
      chdir path
      frm = make_ui(path, false)
    elif frm.value == "_about" then
      frm.close()
      do_about()
      frm = make_ui(path, false)
    elif frm.value == "_setup" then
      frm.close()
      do_setup()
      frm = make_ui(path, false)
    elif frm.value == "_new" then
      frm.close()
      do_newfile()
      frm = make_ui(path, false)
    elif frm.value == "_back" then
      frm.close()
      go_back
      frm = make_ui(path, false)
    fi
  wend
end

main
