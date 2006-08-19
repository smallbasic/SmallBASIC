#
# add {} around single line "if" statements
#

if (len(command$) = 0) then
    ? "Usage: add_brace file.c"
else
    tload inputFile, buffer
    bufferLen = len(buffer)-1    
    
    out = 0
    while (i < bufferLen)
        s = rtrim(buffer(i))
        if (instr(s, "if") != 0) then
            if (rinstr(s, ")") = len(s)) then
               s += "{"
               out << s
               while 1
                  i++
                  if (i >= bufferLen) then
                      exit loop
                  fi
                  s = rtrim(buffer(i))
                  if (rinstr(s, ";") = len(s)) then
                      s += "}"
                      out << s
                      exit loop
                  else 
                      out << s
                  fi
               wend
            fi
        else 
           out << s
        fi
        i++
    wend
    tsave inputFile+".new", out
fi

