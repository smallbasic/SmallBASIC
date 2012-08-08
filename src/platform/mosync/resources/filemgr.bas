const app = "filemgr.bas?"

sub listFiles(path)
  local fileList, ent, esc, basList, dirList, name

  dim basList
  dim dirList
  
  esc = chr(27) + "[ B"
  fileList = files(path)
  
  for ent in fileList
    name = path + ent
    if (isdir(name)) then
      dirList << name
    else if (right(ent, 4) == ".bas") then
      basList << name
    endif
  next ent
  
  sort dirList
  sort basList

  if (path != "/") then
    print esc + app + path + "/../" + "|" + path + chr(28)
  endif    

  for ent in dirList
    print esc + app + ent + "|" + ent + chr(28)
  next ent

  for ent in basList
    print esc + ent + "|" + ent + chr(28)
  next ent
end

listFiles iff(len(command) > 0, command, "/")
