const boldOn = "\033[1m"
const boldOff = "\033[21m"
const scrollHome = "\033m"
const char_h = txth("Q")
const char_w = txtw(".")
const lineSpacing = 2 + char_h
const wnd = window()
const theme = wnd.theme
const colBkGnd = theme.background
const colText  = theme.text5
const colFile  = theme.text2
const colDir   = theme.text3
const colText2 = theme.text4
const colNav   = theme.text1
const colNav2  = theme.text6
const menu_gap = -(char_w / 2)
const is_android = instr(sbver, "Android") != 0
const is_sdl = instr(sbver, "SDL") != 0
const onlineUrl = "http://smallbasic.github.io/samples/index.bas"
const idxEdit = 6
const idxFiles = 7
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
const scratch_file = HOME + "/scratch.bas"

func mk_menu(value, lab, x)
  local bn
  bn.x = x
  bn.y = ypos * char_h
  bn.value = value
  bn.label = "[" + lab + "]"
  bn.color = colNav
  bn.type = "link"
  return bn
end

func mk_scratch()
  local text
  local result = false

  if (not exist(scratch_file)) then
    dim text
    text << "rem Welcome to SmallBASIC"
    text << "rem"
    if (is_sdl) then
      text << "rem Press F1 or F2 for keyword help."
      text << "rem Press and hold Ctrl then press 'h' for editor help."
      text << "rem Press and hold Ctrl then press 'r' to RUN this program."
      text << "rem Click the right mouse button for menu options."
    else
      text << "rem Press the 3 vertical dots for menu options."
      text << "rem Press and drag the line numbers to scroll."
    endif
    try
      tsave scratch_file, text
      result = true
    catch e
      logprint e
      wnd.alert("Failed to create: " + scratch_file)
    end try
  else
    result = true
  endif
  return result
end

sub do_okay_button(bn_extra)
  local frm, button
  button.label = "[Close]"
  button.x = (xmax - txtw(button.label)) / 2
  button.y = ypos * char_h
  button.color = colNav
  button.type = "link"
  if (ismap(bn_extra)) then
    frm.inputs << bn_extra
  endif
  frm.inputs << button
  frm = form(frm)
  print
  frm.doEvents()
end

sub clear_screen()
  color colText, colBkGnd
  cls
end

sub do_about()
  cls
  color colText
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
  color colText
  print "Version "; sbver
  print
  print "Copyright (c) 2002-2021 Chris Warren-Smith"
  print "Copyright (c) 1999-2006 Nicholas Christopoulos" + chr(10)

  local bn_home
  bn_home.x = 2
  bn_home.y = ypos * char_h
  bn_home.type = "link"
  bn_home.isExternal = true
  if (is_sdl) then
    bn_home.label = "https://smallbasic.github.io"
  else
    bn_home.label = "https://smallbasic.github.io/pages/android.html"
  endif
  bn_home.color = colNav
  print:print

  color colText2
  print "SmallBASIC comes with ABSOLUTELY NO WARRANTY. ";
  print "This program is free software; you can use it ";
  print "redistribute it and/or modify it under the terms of the ";
  print "GNU General Public License version 2 as published by ";
  print "the Free Software Foundation." + chr(10)
  print
  color colText
  server_info()
  do_okay_button(bn_home)
  clear_screen()
end

sub do_setup()
  local frm

  color colText, colBkGnd
  cls
  print boldOn + "Setup web service port number."
  print boldOff
  print "Enter a port number to allow web browser or desktop IDE access. "
  print
  print "Values outside the range [1024-65535] will disable this feature."
  print
  print "Press <enter> to leave this screen without making any changes."
  print
  print "The current setting is: " + env("serverSocket")
  print
  color colText, colBkGnd
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

  color colText, colBkGnd
  cls
  print "Web service port number: " + env("serverSocket")
  print
  print boldOn + "Select display font."
  print boldOff
  print "Envy Code R:"
  print "  http://damieng.com/envy-code-r"
  print "Inconsolata:"
  print "  Copyright 2006 The Inconsolata Project"
  print "  http://scripts.sil.org/OFL"
  print "Ubuntu:"
  print "  https://ubuntu.com/legal/font-licence"
  print
  dim frm.inputs(1)
  frm.inputs(0).type="list"
  frm.inputs(0).value="Inconsolata|Envy Code R|UbuntuMono"
  frm.inputs(0).selectedIndex=env("fontId")
  frm.inputs(0).height=TXTH("Q")*3+4
  frm.inputs(0).width=TXTW("Q")*12
  frm = form(frm)
  frm.doEvents()
  env("fontId=" + frm.inputs(0).selectedIndex)

  local msg = "You must restart SmallBASIC for this change to take effect."
  wnd.alert(msg, "Restart required")
  clear_screen()
end

sub server_info()
  local serverSocket = env("serverSocket")
  local ipAddr = env("IP_ADDR")

  if (len(serverSocket) > 0 && int(serverSocket) > 1023 && int(serverSocket) < 65536 && len(ipAddr)) then
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
  local emptyNode

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

  func androidWalker(node)
    if (node.depth == 0 && node.dir == 0 && lower(right(node.name, 4)) == ".bas") then
      basList << node
    endif
    return node.depth == 0
  end

  if (is_android) then
    path = env("EXTERNAL_DIR")
    if (len(path) > 0) then
      dirwalk path, "", use androidWalker(x)
    endif
    path = env("INTERNAL_DIR")
    if (len(path) > 0) then
      dirwalk path, "", use androidWalker(x)
    endif
    path = env("LEGACY_DIR")
    if (len(path) > 0) then
      dirwalk path, "", use androidWalker(x)
    endif
  else
    dirwalk path, "", use walker(x)
  endif
end

sub listFiles(byref frm, path, sortDir, byref basList)
  local lastItem, bn, abbr, gap, n, lab, name, txtcol, i
  local name_col = colNav
  local size_col = colNav
  local date_col = colNav

  sub fix_path
    if (right(path, 1) != "/") then path += "/"
  end

  fix_path
  loadFileList(path, basList)
  select case sortDir
  case 0
    sort basList use filecmpfunc0(x,y)
  case 1
    sort basList use filecmpfunc1(x,y)
    name_col = colNav2
  case 2
    sort basList use filecmpfunc2(x,y)
    size_col = colNav2
  case 3
    sort basList use filecmpfunc3(x,y)
    size_col = colNav2
  case 4
    sort basList use filecmpfunc4(x,y)
    date_col = colNav2
  case 5
    sort basList use filecmpfunc5(x,y)
    date_col = colNav2
  end select

  func mk_bn(value, labText, labCol)
    local bn
    bn.value = value
    bn.label = labText
    bn.color = labCol
    bn.x = 3
    bn.y = -lineSpacing
    return bn
  end

  sub mk_label(labText, labCol, x, y)
    local bn = mk_bn(0, labText, labCol)
    bn.type = "label"
    bn.x = x
    bn.y = y
    frm.inputs << bn
  end

  sub mk_link(value, labText, labCol, x, y)
    local bn = mk_bn(value, labText, labCol)
    bn.type = "link"
    bn.x = x
    bn.y = y
    frm.inputs << bn
  end

  if (is_android) then
    mk_link(sortNameId, "[Name]", name_col, 0, -linespacing)
  else
    mk_label("Files in " + path, colText, 3, -lineSpacing)
    mk_link(backId, "[Go up]", colNav, 0, -linespacing)
    mk_link(sortNameId, "[Name]", name_col, -(char_w * 8), -1)
  endif

  abbr = iff(char_w * 38 > xmax, true, false)
  if (not abbr) then
    mk_link(sortSizeId, "[Size]", size_col, -(char_w * iff(is_android, 23, 8)), -1)
    mk_link(sortDateId, "[Date]", date_col, -(char_w * 6), -1)
  endif

  lastItem = len(basList) - 1
  for i = 0 to lastItem
    node = basList(i)
    txtcol = iff(node.dir, colDir, colFile)
    name = node.name
    if (node.path) then
      path = node.path
      fix_path
    endif
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
      bn = mk_bn(0, node.size + space(n) + timestamp(node.mtime), colText)
      bn.type = "label"
      bn.y = -1
      gap = 29 - len(name)
      bn.x = -(iff(gap > 1, gap, 1) * char_w)
    endif
    frm.inputs << bn
  next i
end

sub manageFiles()
  local f, bn_edit, bn_files, selectedFile

  func fileCmpFunc(l, r)
    local f1 = lower(l)
    local f2 = lower(r)
    return iff(f1 == f2, 0, iff(f1 > f2, 1, -1))
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
    f.inputs << mk_menu(renameId, iff(abbr, "Ren", "Rename"), menu_gap)
    f.inputs << mk_menu(newId, "New", menu_gap)
    f.inputs << mk_menu(deleteId, iff(abbr, "Del", "Delete"), menu_gap)
    f.inputs << mk_menu(saveasId, iff(abbr, "SavAs", "Save-As"), menu_gap)
    bn_edit.x = 0
    bn_edit.y = char_h + 4
    bn_edit.width = xmax
    bn_edit.type = "text"
    bn_edit.color = colText2
    bn_edit.resizable = TRUE
    bn_edit.help = "Enter file name, and then click New."
    bn_files.x = x1
    bn_files.y = bn_edit.y + char_h + 2
    bn_files.height = ymax - bn_files.y
    bn_files.width = xmax - x1
    bn_files.color = colText
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
      color colText
      cls
      len_buffer = len(buffer) - 1
      for i = 0 to len_buffer
        print buffer(i)
      next i
      do_okay_button(nil)
      clear_screen()
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

  clear_screen()
  path = cwd
  frm = makeUI(path, sortDir)

  while 1
    frm.doEvents()

    if (isdir(frm.value)) then
      if (changeDir(frm.value)) then
        cls
        path = frm.value
        frm = makeUI(path, sortDir)
      endif
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
      env("sortDir=" + sortDir)
      frm = makeUI(path, sortDir)
    elif (frm.value == sortSizeId) then
      cls
      sortDir = iff(sortDir==3,2,3)
      env("sortDir=" + sortDir)
      frm = makeUI(path, sortDir)
    elif (frm.value == sortDateId) then
      cls
      sortDir = iff(sortDir==5,4,5)
      env("sortDir=" + sortDir)
      frm = makeUI(path, sortDir)
    fi
  wend
end

main
