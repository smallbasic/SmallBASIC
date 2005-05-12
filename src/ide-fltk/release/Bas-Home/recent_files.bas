'app-plug-in
'menu BASIC Explorer

env("APP-TITLE=Recent Files")
historyFile = env("HOME")+"/.smallbasic/history.txt"
htxt = "<body bgcolor=white><h4>Recent Files</h4><table>"

if (exist(historyFile)) then
    ? "Wait!"
    open historyFile for input as #1
    i=1
    while not eof (1)
        lineinput #1, a
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
    wend
    close #1
    cls
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

# add files from BAS_HOME
basHome = ENV("BAS_HOME")
basHomeFiles = files(basHome+"*.bas")
htxt += "</table><hr><h4>Samples</h4><table>"
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

cmd = basHome+"eraseHistory.bas"
htxt += "</table><br><input onclick=!"+cmd+" value='Erase History'>"
html htxt, "", 2, 2, xmax+1, ymax+1

