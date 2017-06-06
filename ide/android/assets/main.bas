const app = "main.bas?"
const boldOn = chr(27) + "[1m"
const boldOff = chr(27) + "[21m"
const char_h = txth("Q")
const lineSpacing = 2 + char_h
const onlineUrl = "http://smallbasic.github.io/samples/index.bas"
const idxEdit = 6
const idxFiles = 7
const saveasId = "__bn_saveas__"
const renameId = "__bn_rename__"
const deleteId = "__bn_delete__"
const newId = "__bn_new__"
const viewId = "__bn_view__"
const closeId = "__bn_close__"
const menu_gap = -(txtw(" ") / 2)
const is_sdl = instr(sbver, "SDL") != 0

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
  bn.y = ypos * char_h
  bn.value = value
  bn.label = "[" + lab + "]"
  bn.color = 3
  bn.backgroundColor = 0
  bn.type = "link"
  mk_menu = bn
end

sub info_header(is_welcome)
  if (is_welcome) then
    color 7,0
    print boldOn + spaced("Welcome to SmallBASIC") + boldOff
    print
    color 6,0
    if (is_sdl) then
      print "Popup menus are accessed by a right mouse click. ";
    else
      print "Popup menus are accessed by the menu key (3 vertical dots). ";
    endif
    print "From here, you can do things like toggle the ";
    print "Editor, Run, Adjust Font size..."
    if (is_sdl) then
      randomize
      select case (rnd * 100 % 10)
      case 0: tip = "Press and hold Ctrl then press 'm' to access the menu."
      case 1: tip = "Press and hold Ctrl then press 'p' to take a screenshot."
      case 2: tip = "In the editor, press and hold Ctrl then press 'h' to access help."
      case 3: tip = "Toggle the editor menu option for different run modes."
      case 4: tip = "Editor Live Mode makes your program restart whenever the program changes."
      case 5: tip = "Select the Online option to run a featured program."
      case 6: tip = "Select View source from the menu to display program code."
      case 7: tip = "Select the File option to manage .bas files in the current folder."
      case 8: tip = "You can drop .bas files from the system file manager to load a program."
      end select
      print "Tip: " + tip
    endif
    print
  endif
  color 2,0
end

sub do_okay_button()
  local frm, button
  button.label = "[Close]"
  button.x = (xmax - txtw(button.label)) / 2
  button.y = -1
  button.backgroundColor = 0
  button.color = 3
  button.type = "link"
  frm.inputs << button
  frm = form(frm)
  print
  frm.doEvents()
end

sub do_about()
  cls
  color 7,0
  print " __           _      ___ _"
  print "(_ ._ _  _.|||_) /\ (_ |/ "
  print "__)| | |(_||||_)/--\__)|\_"
  print
  print "Version "; sbver
  print
  print "Copyright (c) 2002-2017 Chris Warren-Smith"
  print "Copyright (c) 1999-2006 Nic Christopoulos" + chr(10)
  print "http://smallbasic.sourceforge.net" + chr(10)
  print "SmallBASIC comes with ABSOLUTELY NO WARRANTY. ";
  print "This program is free software; you can use it ";
  print "redistribute it and/or modify it under the terms of the ";
  print "GNU General Public License version 2 as published by ";
  print "the Free Software Foundation." + chr(10)
  print
  server_info()
  do_okay_button()
  cls
end

sub do_setup()
  local frm

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
  endif

  color 3, 0
  cls
  print "Web service port number: " + env("serverSocket")
  print
  print boldOn + "Select display font."
  print boldOff
  print "Envy Code R:"
  print "  http://damieng.com/envy-code-r"
  print "Inconsolata:"
  print "  Copyright 2006 The Inconsolata Project Authors"
  print "  http://scripts.sil.org/OFL"
  print
  dim frm.inputs(1)
  frm.inputs(0).type="list"
  frm.inputs(0).value="Inconsolata|Envy Code R"
  frm.inputs(0).selectedIndex=env("fontId")
  frm.inputs(0).height=TXTH("Q")*2+4
  frm = form(frm)
  frm.doEvents()
  env("fontId=" + frm.inputs(0).selectedIndex)

  local msg = "You must restart SmallBASIC for this change to take effect."
  local wnd = window()
  wnd.alert(msg, "Restart required")
  color 7, 0
  cls
end

sub server_info()
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

func getFiles()
  local list = files("*.*")
  local entry

  dim result
  for entry in list
    if (lower(right(entry, 4)) == ".bas") then
      result << entry
    endIf
  next entry

  sort result use fileCmpFunc(x,y)
  getFiles = result
end

sub manageFiles()
  local f, wnd, bn_edit, bn_files, selectedFile

  sub createUI()
    cls
    local num_chars = 42
    local abbr = TXTW(".") * num_chars > xmax
    f.inputs << mk_menu(closeId, "<<", 0)
    f.inputs << mk_menu(viewId, "View", menu_gap)
    f.inputs << mk_menu(renameId, IFF(abbr, "Ren", "Rename"), menu_gap)
    f.inputs << mk_menu(newId, "New", menu_gap)
    f.inputs << mk_menu(deleteId, IFF(abbr, "Del", "Delete"), menu_gap)
    f.inputs << mk_menu(saveasId, IFF(abbr, "SavAs", "Save-As"), menu_gap)
    bn_edit.x = 0
    bn_edit.y = char_h + 4
    bn_edit.width = xmax
    bn_edit.type = "text"
    bn_edit.color = "white"
    bn_edit.resizable = TRUE
    bn_edit.help = "Enter file name, and then click New."
    bn_files.x = x1
    bn_files.y = bn_edit.y + char_h + 2
    bn_files.height = ymax - bn_files.y
    bn_files.width = xmax - x1
    bn_files.color = 2
    bn_files.type = "list"
    bn_files.resizable = TRUE
    bn_files.help = "No .bas files in " + cwd
    f.focus = idxEdit
    f.inputs << bn_edit
    f.inputs << bn_files
    f = form(f)
    f.value = bn_edit.value
  end

  sub reloadList(selectedIndex)
    local f_list = getFiles()
    local f_list_len=len(f_list)
    if (f_list_len == 0) then
      selectedFile = ""
      f.inputs(idxFiles).value = ""
      selectedIndex = 0
    else
      if (selectedIndex == f_list_len) then
        selectedIndex--
      endif
      selectedFile = f_list(selectedIndex)
      f.inputs(idxFiles).value = f_list
    endif
    f.inputs(idxFiles).selectedIndex = selectedIndex
    f.inputs(idxEdit).value = selectedFile
    f.refresh(false)
  end

  sub deleteFile()
    if (len(selectedFile) > 0) then
      wnd.ask("Are you sure you wish to delete " + selectedFile + "?", "Delete File")
      if (wnd.answer == 0) then
        f.refresh(true)
        local selectedIndex = f.inputs(idxFiles).selectedIndex
        try
          kill selectedFile
          reloadList(selectedIndex)
        catch
          wnd.alert("Error renaming file: " + e)
        end try
      endif
    endif
    f.value = ""
  end

  sub duplicateError()
    wnd.alert("File " + newFile + " already exists", "Duplicate File")
  end

  sub renameFile()
    ' retrieve the edit value
    f.refresh(true)
    local newFile = f.inputs(idxEdit).value
    local selectedIndex = f.inputs(idxFiles).selectedIndex
    if (lower(right(newFile, 4)) != ".bas") then
      newFile += ".bas"
    endIf

    if (exist(selectedFile) and selectedFile != newFile) then
      if (exist(newFile)) then
        duplicateError()
      else
        try
          if sv_as then
            copy selectedFile, newFile
          else
            rename selectedFile, newFile
          endif
        catch
          wnd.alert("Error renaming file: " + e)
        end try
        reloadList(selectedIndex)
      endif
    endif
    f.value = selectedFile
  end

  sub viewFile()
    local frm, button
    if (!exist(selectedFile)) then
      wnd.alert("Select a file and try again")
    else
      tload selectedFile, buffer
      wnd.graphicsScreen2()
      cls
      color 7,0
      len_buffer = len(buffer) - 1
      for i = 0 to len_buffer
        print buffer(i)
      next i
      do_okay_button
      wnd.graphicsScreen1()
      f.value = selectedFile
    endIf
  end

  sub createNewFile()
    f.refresh(true)
    local newFile = f.inputs(idxEdit).value

    if (len(newFile) == 0) then
      exit sub
    endIf
    if (lower(right(newFile, 4)) != ".bas") then
      newFile += ".bas"
    endIf
    try
      if (exist(newFile)) then
        duplicateError()
      else
        dim text
        text << "REM SmallBASIC"
        text << "REM created: " + date
        tsave newFile, text
        local f_list = getFiles()
        local f_list_len=len(f_list) - 1
        local i
        for i = 0 to f_list_len
          if (f_list(i) == newFile) then
            f.inputs(idxFiles).selectedIndex = i
            exit for
          endif
        next i
        f.inputs(idxFiles).value = f_list
        f.refresh(false)
        selectedFile = newfile
      endif
    catch e
      wnd.alert("Error creating file: " + e)
    end try
  end

  createUI()
  reloadList(0)
  wnd = window()
  wnd.showKeypad()

  while 1
    f.doEvents()
    select case f.value
    case renameId
      sv_as = false
      renameFile()
    case saveasId
      sv_as = true
      renameFile()
    case deleteId
      deleteFile()
    case newId
      createNewFile()
    case viewId
      viewFile()
    case closeId
      exit loop
    case else
      if (len(f.value) > 0) then
        ' set the edit value
        f.inputs(idxEdit).value = f.value
        f.refresh(false)
        selectedFile = f.value
      endif
    end select
  wend
  cls
end

func changeDir(s)
  local wnd = window()
  try
    chdir s
    return true
  catch e
    wnd.alert(e)
    return false
  end try
end

sub main
  local path, frm
  local is_welcome = (command == "welcome")

  func makeUI(path)
    local frm, bn_files, bn_online, bn_setup, bn_about, bn_new
    local basList, dirList
    dim basList
    dim dirList

    info_header(is_welcome)
    is_welcome = false
    bn_files = mk_menu("_files", "File", 0)
    bn_online = mk_menu(onlineUrl, "Online", menu_gap)
    bn_setup = mk_menu("_setup", "Setup", menu_gap)
    bn_about = mk_menu("_about", "About", menu_gap)
    bn_online.isExit = true

    frm.inputs << bn_files
    frm.inputs << bn_online
    if (!is_sdl) then
      frm.inputs << bn_setup
    endif
    frm.inputs << bn_about
    listFiles frm, path, basList, dirList
    frm.color = 10
    makeUI = form(frm)
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

  path = cwd
  frm = makeUI(path)

  while 1
    frm.doEvents()

    if (isdir(frm.value)) then
      cls
      if (changeDir(frm.value)) then
        path = frm.value
      endif
      frm = makeUI(path)
    elif frm.value == "_about" then
      do_about()
      frm = makeUI(path)
    elif frm.value == "_setup" then
      do_setup()
      frm = makeUI(path)
    elif frm.value == "_files" then
      changeDir(path)
      managefiles()
      frm = makeUI(path)
    elif frm.value == "_back" then
      cls
      go_back()
      frm = makeUI(path)
    fi
  wend
end

main
