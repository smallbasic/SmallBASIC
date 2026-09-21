' Example for copying a file from flash to SD card

import teensy

const FS_READ = 0
const FS_WRITE = 1

fs = teensy.fs

print "1. Create a test-file"
print "   Open file for writing"
file1 = fs.open("flash:/test.txt", FS_WRITE)
print "   Remove file content (make file empty)"
file1.truncate()
print "   Write strings to file"
file1.write("test1\n")
print "      "; "test1"
file1.write("test2\n")
print "      "; "test2"
file1.write("test3\n")
print "      "; "test3"
print "   Flush buffer (force writing)"
file1.flush()
print "   Close test file"
file1.close()

print "2. Copy test-file from flash to sd"
print "   Open new file on sd"
file_sd = fs.open("sd:/test.txt", FS_WRITE)
print "      Remove file content (make file empty)"
file_sd.truncate()
print "   Open test-file on flash"
file_flash = fs.open("flash:/test.txt", FS_READ)
print "   Read data from flash and write to sd"
while(file_flash.available())
  s = file_flash.read(1)   ' read one byte
  file_sd.write(s)
wend
print "   Close files"
file_flash.close()
file_sd.close()

print "3. Print content of copied file"
print "   Open file on sd"
file_sd = fs.open("sd:/test.txt", FS_READ)
print "   Read data:"
while(file_sd.available())
  s = file_sd.read()   ' read one line
  print "      "; s
wend
print "   Close file"
file_sd.close()
