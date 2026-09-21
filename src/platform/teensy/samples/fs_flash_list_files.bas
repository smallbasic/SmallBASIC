' Example for listing files in a directory of flash memory

import teensy

const FS_READ = 0
const FS_WRITE = 1

fs = teensy.fs

file = fs.open("flash:/")

while(1)
  s = ""
  name = file.getNextFilename()
  if(name == 0) then exit
  
  file2 = fs.open(name)
  if(file2.isDirectory()) then s = "/"
  file2.close()
  print name; s
wend

