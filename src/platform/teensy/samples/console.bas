' Console.bas provides a simple interface for
' accessing flash (teensy 4.0 and 4.1), SD card
' (teensy 4.1) and manipulating input and output
' pins. After uploading this file to the teensy
' type "help" for more information.

import teensy

const FS_READ = 0
const FS_WRITE = 1

const fs = teensy.fs
const usbSerial = teensy.openSerial()

teensy.SetInteractive(1)

CurrentDirectory = ""

print 
print "\e[1mSmallBASIC console v1.0\e[0m"
print "Type 'help' for a list of commands"

while(1)
  if(usbSerial.ready())
    s = usbSerial.receive()
    if(len(s) > 0)
      s = trim(s)
      SPLIT s, " ", v
      cmd = lcase(v[0])

      par_error = 1

      select case cmd
        case "ls"
          if(ubound(v) > 0)
            cmd_ls(v[1])
          else
            cmd_ls(-1)
          endif
          par_error = 0
        case "cd"
          if(ubound(v) > 0)
            cmd_cd(v[1])
            par_error = 0
          endif
        case "mv"
          if(ubound(v) > 1) 
            cmd_mv(v[1], v[2])
            par_error = 0
          endif
        case "rm"
          if(ubound(v) > 0)
            cmd_rm(v[1])
            par_error = 0
          endif
        case "cp"
          if(ubound(v) > 1) 
            cmd_cp(v[1], v[2])
            par_error = 0
          endif
        case "mkdir"
          if(ubound(v) > 0)
            cmd_mkdir(v[1])
            par_error = 0
          endif
        case "print"
          if(ubound(v) > 0)
            cmd_print(v[1])
            par_error = 0
          endif
        case "exit"
          print "See you soon."
          exit
        case "run"
          if(ubound(v) > 0)
            cmd_run(v[1])
            par_error = 0
          endif
        case "getfile"
          if(ubound(v) > 0)
            cmd_getfile(v[1])
            par_error = 0
          endif
        case "help"
          cmd_help()
        case "setpin"
          if(ubound(v) > 1)
            cmd_setpin(v[1], v[2])
            par_error = 0
          endif
        case "getpin"
          if(ubound(v) > 0)
            cmd_getpin(v[1])
            par_error = 0
          endif
        case "analogwrite"
          if(ubound(v) > 1)
            cmd_analogwrite(v[1], v[2])
            par_error = 0
          endif
        case "analogread"
          if(ubound(v) > 0)
            cmd_analogread(v[1])
            par_error = 0
          endif
        case "info"
          cmd_info()
        case "format"
          cmd_format()
      end select
      if(par_error) then print_error("Parameter missing")
    endif
  else
    delay(10)
  endif
wend

func GetFullFile(f)
  local Directory

  if(f == -1) then return -1
  if(f == "/") then return ""
  if(right(f) == "/") then f = chop(f)

  ' Absolute path starting with /
  if(left(f, 4) == "/sd/") then return("sd:" + mid(f, 4))
  if(f == "/sd") then return("sd:")
  if(left(f, 7) == "/flash/") then return("flash:" + mid(f, 7))
  if(f == "/flash") then return("flash:")
  if(left(f) == "/") then return(-1)

  ' path starts with one or more ../
  Directory = CurrentDirectory
  if(left(f, 3) == "../")
    while(1)
      if(left(f, 3) == "../")
        Directory = leftoflast(Directory, "/")
        f = mid(f, 4)
      else
        return GetFullFile(Directory + "/" + f)
      endif
    wend
  endif

  if(f == "..")
    if(Directory == "/sd" or Directory == "/flash") then return ""
    return GetFullFile(leftoflast(Directory, "/"))
  endif

  ' Relative path
  if(CurrentDirectory == "/") then return(-1)
  if(left(f, 2) == "./") then return(GetFullFile(CurrentDirectory + "/" + mid(f, 3)))
  return(GetFullFile(CurrentDirectory + "/" + f))
end

sub cmd_ls(dir)
  local file, path, file2, name

  if(dir == -1 or dir == "/")
    if(CurrentDirectory == "" or dir == "/")
      print "\n\e[32m\e[1mList directory: /\e[0m"
      print "\e[34m[flash]\n\e[34m[sd]\e[0m"
      exit sub
    endif
    path = CurrentDirectory + "/"
  else
    path = dir
  endif

  path = GetFullFile(path) + "/"
  if(path == -1)
    print_error("Path not valid")
    exit sub
  endif

  file = fs.open(path)
  if(!file)
    print_error("Path not found")
    exit sub
  endif  
  if(!file.isDirectory())
    file.close()
    exit sub
  endif

  print "\n\e[32m\e[1mList directory "; path; "\e[0m"

  while(1)
    name = file.getNextFilename()
    if(name == 0)
      exit loop
    endif

    file2 = fs.open(path + name)
    if(file2.isDirectory())
      print "\e[34m[";name;"]\e[0m"
    else
      if(rightof(name,".") == "bas")
        print "\e[35m\e[1m";name; "\t\t\e[0m "; file2.size(); " Bytes"
      else
        print name; "  "; "\t\t"; file2.size(); " Bytes"
      endif
    endif
    file2.close()
  wend
  file.close()
end

sub cmd_cd(directory)
  local file, path

  path = GetFullFile(directory)
  if(path == -1)
    print_error("Path not valid")
    exit sub
  endif
  if(path == "")
    CurrentDirectory = ""
    print "\nchange to directory /"
    exit sub
  endif

  file = fs.open(path)
  if(!file)
    print_error("Not a directory")
    exit sub
  endif
  
  if(file.isDirectory())
    if(left(path, 3) == "sd:")
      CurrentDirectory = "/sd" + mid(path, 4)
    else if(left(path, 6) == "flash:")
      CurrentDirectory = "/flash" + mid(path, 7)
    endif
    if(right(CurrentDirectory) == "/") then CurrentDirectory = chop(CurrentDirectory)
    print "\nchange to directory "; CurrentDirectory
  else
    print_error("Not a directory")
  endif
  file.close()
end

sub cmd_mv(a, b)
  a = GetFullFile(a)
  b = GetFullFile(b)

  if(a == -1 or b == -1)
    print_error("File not valid")
    exit sub
  endif

  if(left(a) != left(b))
    print_error("files can't be moved between different file systems")
    exit sub
  endif

  print "\nmove "; a; " to "; b
  if(!fs.rename(a,b)) then print_error("could not move file")
end

sub cmd_rm(f)
  f = GetFullFile(f)
  if(f == -1)
    print_error("File not valid")
    exit sub
  endif

  print "\nremove "; f
  if(!fs.remove(f)) then print_error("could not remove file")
end

sub cmd_cp(f1, f2)
  local file1, file2, s, path

  f1 = GetFullFile(f1)
  f2 = GetFullFile(f2)

  if(a == -1 or b == -1)
    print_error("File not valid")
    exit sub
  endif

  print "\ncopy "; f1; " to "; f2
  
  file1 = OpenFile(f1)
  if(!file1)
    print_error("could not open source file")
    exit sub
  endif

  file2 = fs.open(f2, FS_WRITE)
  if(!file2)
    file1.close()
    print_error("could not open destination file")
    exit sub
  endif
  file2.truncate()

  while(file1.available())
    s = file1.read(1)
    file2.write(s)
  wend

  file1.close()
  file2.close()
end

sub cmd_mkdir(dir)
  dir = GetFullFile(dir)

  if(dir == -1)
    print_error("Directory not valid")
    exit sub
  endif

  print "\ncreate directoy "; dir
  if(!fs.mkdir(dir))
    print_error("could not create directory")
  endif
end

sub cmd_print(f)
  local file, s

  f = GetFullFile(f)
  if(f == -1)
    print_error("File not valid")
    exit sub
  endif

  file = OpenFile(f)
  if(!file)
    exit sub
  endif

  while(file.available())
    s = file.read(1)
    print chr(s);
  wend

  file.close()
end

sub print_error(s)
  print "\e[31mERROR: ";s;"\e[0m"
end

func OpenFile(name)
  local file

  file = fs.open(name, FS_READ)
  if(!file)
    print_error("not a file")
    return 0
  endif
  if(file.isDirectory())
    print_error("is directory")
    file.close()
    return 0
  endif
  return file
end

sub cmd_getfile(name)
  local file, bytes, c
  
  name = GetFullFile(name)
  if(name == -1)
    print_error("File not valid")
    exit sub
  endif

  file = fs.open(name, FS_WRITE)
  if(!file)
    print_error("could not write to file")
    exit sub
  endif
  file.truncate()

  print "\nWaiting for upload."
  teensy.SetInteractive(0)

  while(!usbSerial.ready())
    delay(200)
  wend

  bytes = 0
  while(usbSerial.ready())
    c = usbSerial.receive(1)
    file.write(c, 1)
    bytes++
  wend

  file.close()
  Print bytes; " Bytes received"
  teensy.SetInteractive(1)
end

sub cmd_run(name)
  local file, bytes, s

  name = GetFullFile(name)
  if(name == -1)
    print_error("File not valid")
    exit sub
  endif

  file = fs.open(name, FS_READ)
  if(!file)
    print_error("could not open file")
    exit sub
  endif

  dim s
  while(file.available())
    s << file.read()
  wend
  chain s
end

sub cmd_format()
  if(!fs.Quickformat()) then print_error("could not format flash")
end

sub cmd_help()
  print
  print "ls          | List files:  ls | ls /test | ls .."
  print "cd          | Change directory: cd / | cd /dir1 | cd .."
  print "cp          | Copy file: cp a.txt b.txt | cp a.txt /test/a.txt"
  print "mv          | Move file: mv a.txt b.txt | mv a.txt /test/a.txt"
  print "rm          | Delete file or direcrtory: rm a.txt | rm /test/a.txt"
  print "mkdir       | Create directory: mkdir dir1 | mkdir dir1/dir2"
  print "print       | Print file: print a.txt"
  print "run         | Run file: run a.bas"
  print "getfile     | Get file: getfile a.txt"
  print "exit        | Exit console"
  print "format      | Quickformat of flash file system"
  print "info        | Print system information"
  print "setpin      | Set pin to high or low: setpin 13 0 | setpin 13 1
  print "getpin      | Get pin level. Returns high or low: getpin 10
  print "analogwrite | Set pin to 0 to 255: analogwrite 13 128
  print "analogread  | Get ADC value of pin: analogread 10
  print
  print "Absolute paths start with /"
  print "Relative paths start without /"
  print
end

sub cmd_setpin(PinNumber, level)
  local Pin
  PinNumber = val(PinNumber)
  level = val(level)
  Pin = teensy.openDigitalOutput(PinNumber)
  Pin.write(level)
end

sub cmd_getpin(PinNumber)
  local Pin
  PinNumber = val(PinNumber)
  Pin = teensy.openDigitalInput(PinNumber)
  print Pin.read()
end

sub cmd_analogwrite(PinNumber, level)
  local Pin
  PinNumber = val(PinNumber)
  level = val(level)
  Pin = teensy.openAnalogOutput(PinNumber)
  Pin.write(level)
end

sub cmd_analogread(PinNumber)
  local Pin
  PinNumber = val(PinNumber)
  Pin = teensy.openAnalogInput(PinNumber)
  print Pin.read()
end

sub cmd_info()
  local RAM, FileSystem
  RAM = teensy.free()
  FileSystem = fs.free()
  print "\n\e[1mMemory usage\e[0m"
  print "STACK free : "; RAM[0]; " Bytes"
  print "HEAP  free : "; RAM[1]; " Bytes"
  print "FLASH used : "; FileSystem[0];  " Bytes"
  print "FLASH total: "; FileSystem[1];  " Bytes"
  print "SD    used : "; FileSystem[2];  " Bytes"
  print "SD    total: "; FileSystem[3];  " Bytes"
  print "\e[1mCPU\e[0m"
  print "TEMP       : "; teensy.GetTemp(); " °C"
  print "SPEED      : "; teensy.GetCPUSpeed(); " MHz"
end