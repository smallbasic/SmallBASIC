'app-plug-in
'menu BASIC Explorer

sub listFiles(basHome, byref htxt)
  # add files from BAS_HOME
  basHomeFiles = files(basHome+"*.bas")
  sort basHomeFiles
  for a in basHomeFiles
    i++
    if i%2 = 1 then
        htxt += "<tr bgcolor=#c1c1c1>"
    else
        htxt += "<tr bgcolor=white>"
    fi
    htxt += "<td width=70%>" +a +"</td>"
    fullpath  = basHome + a
    htxt += "<td><a href='" +fullpath+"'>[Edit]</a></td>"
    htxt += "<td><a href='!"+fullpath+"'>[Run]</a></td>"
    htxt += "</tr>"
  next i
  # add final blank
  htxt += "<tr bgcolor=white><td></td></tr>"
end

env("TITLE=BASIC Explorer")
historyFile = env("BAS_HOME")+"/history.txt"
app = env("PKG_HOME")+"/plugins/filemgr.bas"

if command = "X" then
  historyFile = env("BAS_HOME")+"history.txt"
  kill historyFile
fi

htxt = "<body bgcolor=white><b><font size=3>"
if len(command) = 0 OR command = "R" OR len(command) > 1 then
  htxt += "[Recent Files]"
else
  htxt += "<a href='!"+app+" R'>[Recent Files]</a>"
fi
if command = "S" then
  htxt += " [Samples]"
else
  htxt += " <a href='!"+app+" S'>[Samples]</a>"
fi
if command = "H" then
  htxt += " [Home]"
else
  htxt += " <a href='!"+app+" H'>[Home]</a>"
fi

if command = "T" then
  htxt += " [Temp]"
else
  htxt += " <a href='!"+app+" T'>[Temp]</a>"
fi

htxt += "</b><br>"
if len(command) = 0 OR (command != "S" and command != "H" and command != "T") then
  htxt += "<hr><h4>Recent Files</h4><table>"
  if (exist(historyFile)) then
    sortedFiles = 0
    open historyFile for input as #1
    while not eof (1)
      lineinput #1, a
     ' a = translate(a, "\\", "/")
      sortedFiles << a
    wend
    close #1

    i=1
    sort sortedFiles
    for a in sortedFiles
      i++
      if i%2 = 1 then
        htxt += "<tr bgcolor=#c1c1c1>"
      else
        htxt += "<tr bgcolor=white>"
      fi
        htxt += "<td width=70%>" +a +"</td>"
        htxt += "<td><a href='" +a+"'>[Edit]</a></td>"
        htxt += "<td><a href='!"+a+"'>[Run]</a></td>"
        htxt += "</tr>" 
    next a
  fi

  # add the scratch file
  scratchFile = env("HOME")+"/.smallbasic/untitled.bas"
  i++
  if i%2 = 1 then
    htxt += "<tr bgcolor=#c1c1c1>"
  else
    htxt += "<tr bgcolor=white>"
  fi
  htxt += "<td width=70%>untitled.bas</td>"
  htxt += "<td><a href='"+scratchFile+"'>[Edit]</a></td>"
  htxt += "<td><a href='!"+scratchFile+"'>[Run]</a></td></tr>"
  htxt += "<tr bgcolor=white><td></td></tr>"
  cmd = env("BAS_HOME")+"recent_files.sbx X"
  htxt += "</table><br><b><a href='!"+cmd+"'>[Erase History]</a>"
elif command = "S" then
  htxt += "<hr><h4>Samples</h4><table>"
  listFiles ENV("BAS_HOME"), htxt
elif command = "H" then
  htxt += "<hr><h4>Home</h4><table>"
  listFiles ENV("HOME")+"/", htxt
elif command = "T" then
  htxt += "<hr><h4>Temp</h4><table>"
  listFiles ENV("TEMP")+"/", htxt
fi
    
html htxt, "", 2, 2, xmax+1, ymax+1

