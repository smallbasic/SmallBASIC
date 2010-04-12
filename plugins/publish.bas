'tool-plug-in
'menu Publish Online

split command, "|", args() use trim(x)
fname = args(0)

pkgHome = lower(env("PKG_HOME"))
path = lower(left(fname, len(pkgHome)))
if (pkgHome == path) then
  logprint "Please do not submit packaged programs"
  exit
fi 

logprint "This will publish " + fname + " to smallbasic.sourceforge.net."
logprint "The server will only accept a limited number of submissions from a single IP address"
input "Enter Y to continue", k
if (upper(k) != "Y") then
  exit
fi

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

