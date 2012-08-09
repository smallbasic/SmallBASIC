const app = "filemgr.bas?"

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
    print " " + esc + app + backPath + "|< back >" + chr(28)
  endif    

  for ent in dirList
    print " " + esc + app + path + ent + "|[" + ent + "]" + chr(28)
  next ent

  print chr(27) + "[1;32m";
  for ent in basList
    print " " + esc + path + ent + "|" + ent + chr(28)
  next ent
end

listFiles command
