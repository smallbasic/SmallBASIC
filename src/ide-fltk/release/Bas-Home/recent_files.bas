'app-plug-in
'menu Recent Files

historyFile = env("HOME")+"/.smallbasic/history.txt"
if (exist(historyFile)) then
   htxt = "<body bgcolor=white><h1>Recent Files</h1>"
   open historyFile for input as #1
   while not eof (1)
      lineinput #1, a
      htxt += "<a href='"+a+"'>"+a+"</a><br>"
   wend
   close #1
   html htxt, "", 2, 2, xmax, ymax
else 
   ? "Unable to open history file: "; historyFile
fi
