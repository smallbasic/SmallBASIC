#
# add curley braces '{', '}' around single line block statements
#

if (len(command$) = 0) then
    ? "Usage: add_brace file.c"
else
    main command$
fi

sub main(byref filename)
    local buffer, bufferLen, s, out, i, 

    tload filename, buffer
    bufferLen = len(buffer)-1    

    sub end_brace
      s += "{"
      out << s
      while 1
         i++
         if (i >= bufferLen) then
             exit loop
         fi
         s = rtrim(buffer(i))
         if (rinstr(s, ";") = len(s)) then
           if (left(s, 2) != "//") then
             s += "}"
             out << s
             exit loop
           else
             out << s 
           fi
         else 
           out << s
         fi
      wend
    end
    
    out = 0
    while (i < bufferLen)
      s = rtrim(buffer(i))
      trim_s = ltrim(s)
      found = 0
      if (left(trim_s, 2) = "if" || left(trim_s, 7) = "else if") then
        if (rinstr(s, ")") = len(s)) then
          found = 1
          end_brace
        fi
      elif (trim_s = "else") then
         found = 1
         end_brace
      fi
      if (found = 0) then
         out << s
      fi
      i++
    wend
    tsave filename+".new", out
end

