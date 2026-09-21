' Example for flash file access

import teensy

const FS_READ = 0
const FS_WRITE = 1

fs = teensy.fs

'print "format flash"
'fs.quickformat()

print "1. Free space"
r = fs.free()
print "   Flash used size: ", r[0]
print "   Flash total size: ", r[1]

print "2. Create directory"
fs.mkdir("flash:/TEST")
print "   flash:/Test exists: "; fs.exists("flash:/TEST")

print "3. Open file for writing"
file1 = fs.open("flash:/test.txt", FS_WRITE)
print "   Remove file content (make file empty)"
file1.truncate()
print "   Write string to file"
file1.write("test1\n")
file1.write("test2\n")
file1.write("test3\n")
print "   Flush buffer (force writing)"
file1.flush()
print "   Close file"
file1.close()

print "4. Read from file"
file2 = fs.open("flash:/test.txt", FS_READ)
size = file2.size()
print "   Size of test.txt: "; size
print "   Content of test.txt: "
while(file2.available())
  s = file2.read()
  print "      "; s
wend

print "   Goto position 1 in file"
file2.seek(1)
print "   Get current position"
print "      "; file2.position()
print "   Read one line from current position"
print "      "; file2.read()

file2.close()

print "5. Move file"
fs.rename("flash:/test.txt", "flash:/test2.txt")

print "6. Remove file"
fs.remove("flash:/test2.txt")
