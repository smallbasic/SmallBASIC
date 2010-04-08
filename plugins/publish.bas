'tool-plug-in
'menu Publish

split command, "|", args() use trim(x)
fname = args(0)
tload fname, buffer

local url = "http://smallbasic.sourceforge.net/publish.php"

dim out
out << "<form name=publish method=post action=" + url + ">"
out << "<textarea name=code style=display:none>"

len_b = len(buffer) - 1
for i = 0 to len_b
  out << buffer(i)
next i

out << "</textarea>"
out << "</form>"
out << "<script>"
out << "window.onload = function() {"
out << "document.publish.submit();"
out << "}"
out << "</script>"

fname = env("PKG_HOME") + "/publish.html"
tsave fname, out
html fname

