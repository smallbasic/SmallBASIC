'app-plug-in
'menu Recent Files

env("APP-TITLE=Recent Files")
historyFile = env("HOME")+"/.smallbasic/history.txt"
htxt = "<body bgcolor=white><h1>Recent Files</h1><table>"

if (exist(historyFile)) then
    ? "Wait!"
    open historyFile for input as #1
    i=1
    while not eof (1)
        lineinput #1, a
        i++
        if i%2 = 1
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

scratchFile = env("HOME")+"/.smallbasic/untitled.bas"
i++
if i%2 = 1
    htxt += "<tr bgcolor=#c1c1c1>"
else
    htxt += "<tr bgcolor=white>"
fi

htxt += "<td width=70%>untitled.bas</td>"
htxt += "<td><a href='"+scratchFile+"'>[Edit]</a></td>"
htxt += "<td><a href='!"+scratchFile+"'>[Run]</a></td></tr>"
htxt += "<tr bgcolor=white><td></td></tr>"

cmd = ENV("BAS-HOME")+"eraseHistory.bas"
htxt += "</table><br><input onclick=!"+cmd+" value='Erase History'>"
html htxt, "", 2, 2, xmax+1, ymax+1
