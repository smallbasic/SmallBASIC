'tool-plug-in
'menu Publish

split command, "|", args() use trim(x)
fname = args(0)
tload fname, buffer

local url = "http://smallbasic.sourceforge.net?q=node/1021"

dim out
out << "<form name=publish method=post enctype=multipart/form-data action=" + url + ">"
out << "<input type=text name=submitted[filename] style=display:none value='" + fname + "'>"
out << "<input type=hidden name=form_id value=webform_client_form_1021 />"
out << "<input type=hidden name=op value=Submit />"
out << "<input type=hidden name=submitted[sbasic] value=sbasic />"
out << "<textarea name=submitted[code] style=display:none>"

len_b = len(buffer) - 1
for i = 0 to len_b
  textLine = buffer(i)
  textLine = translate(textLine, "<", "&lt;")
  textLine = translate(textLine, ">", "&gt;")
  textLine = translate(textLine, "\"", "&quot;")
  out << textLine
next i

out << "</textarea>"
out << "</form>"
out << "<script>"
out << "window.onload = function() {"
out << " document.publish.submit();"
out << "}"
out << "</script>"

fname = env("BAS_HOME") + "/publish.html"
tsave fname, out
html fname

