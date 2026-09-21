' Example for SD card file access

import teensy

const FS_READ = 0
const FS_WRITE = 1

fs = teensy.fs

print "1. Free space"
r = fs.free()
print "   SD used size: ", r[2]
print "   SD total size: ", r[3]

print "2. Create directory"
fs.mkdir("sd:/TEST")
print "   sd:/Test exists: "; fs.exists("sd:/TEST")

print "3. Open file for writing"
file1 = fs.open("sd:/test.txt", FS_WRITE)
print "   Remove file content (make file empty)"
file1.truncate()
print "   Write string to file"
file1.write("sdtest1\n")
file1.write("sdtest2\n")
file1.write("sdtest3\n")
print "   Flush buffer (force writing)"
file1.flush()
print "   Close file"
file1.close()

print "4. Read from file"
file2 = fs.open("sd:/test.txt", FS_READ)
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
fs.rename("sd:/test.txt", "sd:/test2.txt")

print "6. Remove file"
fs.remove("sd:/test2.txt")
