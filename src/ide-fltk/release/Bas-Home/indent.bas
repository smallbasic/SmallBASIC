'tool-plug-in
'menu Edit/Indent

split command, "|", args() use trim(x)
local inbuf, outbuf
fname = args(0)
indSize= 4
begin_words = ["while", "if", "elseif", "elif", "else", "repeat", "for", "func", "sub"]
end_words = ["wend", "fi", "endif", "elseif", "elif", "else", "next", "end", "until"]
level = 0

cls
tload fname, inbuf
buflen = len(inbuf)

for i = 0 to buflen-1
    if (i = buflen-1 && len(inbuf(i)) = 0) 
        exit for
    fi

    ln = translate(ltrim(inbuf(i)), chr(8), "")
    ln = translate(ln, "  ", " ")
    if (len(ln) > 0) then
        split ln, " ", words() use lower(trim(x))
        firstWd = words(0)
        if level > 0 && firstWd in end_words then
           level--
        fi
        outbuf << space(level*indSize)+ln
        if firstWd in begin_words then
           level++
        fi
        ## handle if/then on same line with chars following "then"
        if firstWd = "if" && "then" in words && words(len(words)-1) != "then" then
           level--
        fi
    else 
       outbuf << ln
    fi
next i

tsave fname+".bak", inbuf
tsave fname, outbuf
