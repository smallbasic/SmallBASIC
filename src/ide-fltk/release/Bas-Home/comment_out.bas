'editor-plug-in
'menu Edit/Uncomment Region

split command, " ", args() use trim(x)
local inbuf, outbuf
fname = args(0)
row = args(1)
col = args(2)
s1_row = args(3)
s1_col = args(4)
s2_row = args(5)
s2_col = args(6)

? "Uncommenting between lines "+s1_row+" and "+ s2_row

tload fname, inbuf
buflen = len(inbuf)
for i = 0 to buflen-1
    if (i = buflen-1 && len(inbuf(i)) = 0) 
        exit for
    fi
    if (i => s1_row && i < s2_row)
        s1 = inbuf(i)
        if (instr(1, s1, "'") = 1 || instr(1, s1, "#") = 1)
           s1 = mid(s1, 2)
        elseif (instr(1, s1, "rem ") = 1)
           s1 = mid(s1, 5)
        fi
        outbuf << s1
    else 
       outbuf << inbuf(i)
    fi
next i

tsave fname+".bak", inbuf
tsave fname, outbuf
