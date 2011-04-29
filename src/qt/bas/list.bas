? cat(3) + command + cat(0)
f = files(command + "/*")
sort f

? chr(27) + "[ h" + command + "/..;..:"

for a in f
  if (isdir(a) or right(a, 4) == ".bas") then
    ? chr(27) + "[ h" + command + "/" + a + ";" + a + ":"
  endif
next a
