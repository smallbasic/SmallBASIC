'editor-plug-in
'menu Edit/Comment Out Region

split command, " ", args() use trim(x)
local inbuf, outbuf
fname = args(0)
row = args(1)
col = args(2)
s1_row = args(3)
s1_col = args(4)
s2_row = args(5)
s2_col = args(6)

tload fname, inbuf
buflen = len(inbuf)
for i = 0 to buflen-1
    if (i = buflen-1 && len(inbuf(i)) = 0) 
        exit for
    fi
    if (i => s1_row && i < s2_row) 
        outbuf << "'"+inbuf(i)
    else 
       outbuf << inbuf(i)
    fi
next i

tsave fname+".bak", inbuf
tsave fname, outbuf


