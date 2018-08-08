const app = "main.bas?"
const boldOn = chr(27) + "[1m"
const boldOff = chr(27) + "[21m"
const scrollHome = chr(27) + "m"
const char_h = txth("Q")
const char_w = txtw(".")
const lineSpacing = 2 + char_h
const idxEdit = 6
const idxFiles = 7
const colGrey = rgb(100,100,100)
const menu_gap = -(char_w / 2)
const is_sdl = instr(sbver, "SDL") != 0
const onlineUrl = "http://smallbasic.github.io/samples/index.bas"
const saveasId = "__bn_saveas__"
const renameId = "__bn_rename__"
const deleteId = "__bn_delete__"
const newId = "__bn_new__"
const viewId = "__bn_view__"
const closeId = "__bn_close__"
const sortNameId = "_sort_name"
const sortSizeId = "_sort_size"
const sortDateId = "_sort_date"
const filesId = "_files"
const setupId = "_setup"
const aboutId = "_about"
const backId = "_back"
const scratchId = "_scratch"
const scratch_file = HOME + "scratch.bas"

func mk_bn(value, lab, fg)
  local bn
  bn.x = 2
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

func mk_scratch()
  local text
  local result = false

  if (not exist(scratch_file)) then
    dim text
    text << "rem Welcome to SmallBASIC"
    text << "rem"
    if (is_sdl) then
      text << "rem Press F1 for keyword help."
      text << "rem Press and hold Ctrl then press 'h' for editor help."
      text << "rem Press and hold Ctrl then press 'r' to RUN this program."
      text << "rem Click the right mouse button for menu options."
    else
      text << "rem Press the 3 vertical dots for menu options."
    endif
    try
      tsave scratch_file, text
      result = true
    catch e
      local wnd = window()
      wnd.alert("Failed to create scratch file: " + e)
    end try
  else
    result = true
  endif
  return result
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
  color 2,0
  if (char_w * 45 < xmax) then
    print "   ____          _______  ___   _____________"
    print "  / ____ _ ___ _/ / / _ )/ _ | / __/  _/ ___/"
    print " _\ \/  ' / _ `/ / / _  / __ |_\ \_/ // /__  "
    print "/___/_/_/_\_,_/_/_/____/_/ |_/___/___/\___/  "
  else
    print " __           _      ___ _"
    print "(_ ._ _  _.|||_) /\ (_ |/ "
    print "__)| | |(_||||_)/--\__)|\_"
  endif
  print
  color 7,0
  print "Version "; sbver
  print
  print "Copyright (c) 2002-2018 Chris Warren-Smith"
  print "Copyright (c) 1999-2006 Nicholas Christopoulos" + chr(10)
  print "https://smallbasic.sourceforge.io" + chr(10)
  color colGrey,0
  print "SmallBASIC comes with ABSOLUTELY NO WARRANTY. ";
  print "This program is free software; you can use it ";
  print "redistribute it and/or modify it under the terms of the ";
  print "GNU General Public License version 2 as published by ";
  print "the Free Software Foundation." + chr(10)
  print
  color 7,0
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

  if (len(serverSocket) > 0 && int(serverSocket) > 0 && len(ipAddr)) then
    serverSocket = ipAddr + ":" + serverSocket
    print boldOff + "Web Service: " + boldOn + serverSocket
    print boldOff + "Access token: " + boldOn + env("serverToken")
    print boldOff
  fi
end

func fileCmpFunc0(l, r)
  local f1 = lower(l.name)
  local f2 = lower(r.name)
  local n = iff(f1 == f2, 0, iff(f1 > f2, 1, -1))
  return iff(l.dir == r.dir, n, iff(l.dir, 1, -1))
end

func fileCmpFunc1(l, r)
  local f1 = lower(l.name)
  local f2 = lower(r.name)
  local n = iff(f1 == f2, 0, iff(f1 > f2, -1, 1))
  return iff(l.dir == r.dir, n, iff(l.dir, 1, -1))
end

func fileCmpFunc2(l, r)
  local f1 = l.size
  local f2 = r.size
  local n = iff(f1 == f2, 0, iff(f1 > f2, 1, -1))
  return iff(l.dir == r.dir, n, iff(l.dir, 1, -1))
end

func fileCmpFunc3(l, r)
  local f1 = l.size
  local f2 = r.size
  local n = iff(f1 == f2, 0, iff(f1 > f2, -1, 1))
  return iff(l.dir == r.dir, n, iff(l.dir, 1, -1))
end

func fileCmpFunc4(l, r)
  local f1 = l.mtime
  local f2 = r.mtime
  return iff(f1 == f2, 0, iff(f1 > f2, 1, -1))
end

func fileCmpFunc5(l, r)
  local f1 = l.mtime
  local f2 = r.mtime
  return iff(f1 == f2, 0, iff(f1 > f2, -1, 1))
end

sub loadFileList(path, byref basList)
  erase basList
  local emptyNode, storage

  func walker(node)
    if (node.depth==0) then
      node.path = 0
      if (node.dir && left(node.name, 1) != ".") then
        node.size = ""
        basList << node
      else if (lower(right(node.name, 4)) == ".bas") then
        basList << node
      endif
    endif
    return node.depth == 0
  end
  dirwalk path, "", use walker(x)

  if (path = "/" && len(basList) == 0 && !is_sdl) then
     storage = env("EXTERNAL_STORAGE")
     if (len(storage) > 0) then
       emptyNode.name = mid(storage, 2)
       emptyNode.dir = true
       emptyNode.size = ""
       emptyNode.mtime = 0
       basList << emptyNode
     endif
  endif
end

sub listFiles(byref frm, path, sortDir, byref basList)
  local lastItem, bn, abbr, gap, n, lab, name, txtcol, i
  local name_col = 3
  local size_col = 3
  local date_col = 3

  if (right(path, 1) != "/") then
    path += "/"
  endif

  loadFileList(path, basList)
  select case sortDir
  case 0
    sort basList use filecmpfunc0(x,y)
  case 1
    sort basList use filecmpfunc1(x,y)
    name_col = 6
  case 2
    sort basList use filecmpfunc2(x,y)
    size_col = 5
  case 3
    sort basList use filecmpfunc3(x,y)
    size_col = 6
  case 4
    sort basList use filecmpfunc4(x,y)
    date_col = 5
  case 5
    sort basList use filecmpfunc5(x,y)
    date_col = 6
  end select

  bn = mk_bn(0, "Files in " + path, 7)
  bn.type = "label"
  bn.x = 0
  bn.y = -lineSpacing
  frm.inputs << bn

  bn = mk_bn(backId, "[Go up]", 3)
  bn.type = "link"
  bn.x = 0
  bn.y = -linespacing
  frm.inputs << bn

  bn = mk_bn(sortNameId, "[Name]", name_col)
  bn.type = "link"
  bn.x = -(char_w * 8)
  bn.y = -1
  frm.inputs << bn

  abbr = iff(char_w * 38 > xmax, true, false)
  if (not abbr) then
    bn = mk_bn(sortSizeId, "[Size]", size_col)
    bn.type = "link"
    bn.x = -(char_w * 8)
    bn.y = -1
    frm.inputs << bn

    bn = mk_bn(sortDateId, "[Date]", date_col)
    bn.type = "link"
    bn.x = -(char_w * 6)
    bn.y = -1
    frm.inputs << bn
  endif

  lastItem = len(basList) - 1
  for i = 0 to lastItem
    node = basList(i)
    txtcol = iff(node.dir, 3, 2)
    name = node.name
    if (abbr) then
      bn = mk_bn(path + name, name, txtcol)
      bn.type = "link"
      if (!node.dir) then bn.isExit = true
    else
      if (len(name) > 27) then
        lab = left(name, 27) + "~"
      else
        lab = name
      endif
      bn = mk_bn(path + name, lab, txtcol)
      bn.type = "link"
      if (!node.dir) then bn.isExit = true
      frm.inputs << bn

      gap = 12 - len(str(node.size))
      n = iff(gap > 1, gap, 1)
      bn = mk_bn(0, node.size + space(n) + timestamp(node.mtime), colGrey)
      bn.type = "label"
      bn.y = -1
      gap = 29 - len(name)
      bn.x = -(iff(gap > 1, gap, 1) * char_w)
    endif
    frm.inputs << bn
  next i
end

sub manageFiles()
  local f, wnd, bn_edit, bn_files, selectedFile

  func fileCmpFunc(l, r)
    local f1 = lower(l)
    local f2 = lower(r)
    return IFF(f1 == f2, 0, IFF(f1 > f2, 1, -1))
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
    return result
  end

  sub createUI()
    cls
    local num_chars = 42
    local abbr = char_w * num_chars > xmax
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
  try
    chdir s
    return true
  catch e
    local wnd = window()
    wnd.alert(e)
    return false
  end try
end

sub main
  local path, frm
  local sortDir = env("sortDir")
  if (len(sortDir) == 0) then sortDir = 0

  func makeUI(path, sortDir)
    local frm, bn_files, bn_online, bn_setup, bn_about, bn_new, bn_scratch
    local basList
    dim basList

    bn_files = mk_menu(filesId, "File", 0)
    bn_online = mk_menu(onlineUrl, "Online", menu_gap)
    bn_scratch = mk_menu(scratchId, "Scratch", menu_gap)
    bn_setup = mk_menu(setupId, "Setup", menu_gap)
    bn_about = mk_menu(aboutId, "About", menu_gap)
    bn_online.isExit = true

    frm.inputs << bn_files
    frm.inputs << bn_online
    frm.inputs << bn_scratch
    if (!is_sdl) then
      frm.inputs << bn_setup
    endif
    frm.inputs << bn_about
    listFiles frm, path, sortDir, basList
    frm = form(frm)
    print scrollHome;
    return frm
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
  frm = makeUI(path, sortDir)

  while 1
    frm.doEvents()

    if (isdir(frm.value)) then
      cls
      if (changeDir(frm.value)) then
        path = frm.value
      endif
      frm = makeUI(path, sortDir)
    elif frm.value == aboutId then
      do_about()
      frm = makeUI(path, sortDir)
    elif frm.value == setupId then
      do_setup()
      frm = makeUI(path, sortDir)
    elif frm.value == filesId then
      if (changeDir(path)) then
        managefiles()
      endif
      frm = makeUI(path, sortDir)
    elif frm.value == scratchId then
      if (mk_scratch())
        frm.close(scratch_file)
      endif
    elif frm.value == backId then
      cls
      go_back()
      frm = makeUI(path, sortDir)
    elif (frm.value == sortNameId) then
      cls
      sortDir = iff(sortDir==0,1,0)
      env("sortDir="+sortDir)
      frm = makeUI(path, sortDir)
    elif (frm.value == sortSizeId) then
      cls
      sortDir = iff(sortDir==3,2,3)
      env("sortDir="+sortDir)
      frm = makeUI(path, sortDir)
    elif (frm.value == sortDateId) then
      cls
      sortDir = iff(sortDir==5,4,5)
      env("sortDir="+sortDir)
      frm = makeUI(path, sortDir)
    fi
  wend
end

main
