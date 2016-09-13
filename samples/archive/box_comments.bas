'
' This program will convert C function comments from this:
' 
' //
' // my comment
' //
' void foo()
'
' to this:
'
' /**
'  * my comment
'  */
' void foo()
'
'

dim out_buffer
tload trim(command), buffer
local b_len = len(buffer) - 1
in_block = false
after_header = false

for ln = 0 to b_len
  if (!after_header && left(buffer(ln+1), 1) == "#") then
     after_header = true
  fi

  if (after_header && !in_block && ln+1 < b_len && buffer(ln) == "//" && left(buffer(ln+1), 2) == "//") then
    out_buffer << "/**"
    out_buffer <<  " *" + if(len(buffer(ln+1)) > 2, mid(buffer(ln+1), 3), " TODO add comment")
    in_block = true
    ln++
  elif in_block
    'continue block
    if (buffer(ln) == "//") then
      if (ln+1 < b_len && left(buffer(ln+1), 2) != "//") then
        out_buffer <<  " */"
        in_block = false
      else
        out_buffer <<  " *"
      fi
    elif (left(buffer(ln), 2) == "//")
      out_buffer <<  " * " + trim(mid(buffer(ln), 3))
    else
      in_block = false
      out_buffer <<  buffer(ln)
    fi
  elif (left(buffer(ln), 3) == " * ") then
    ' cleanup existing comments
    out_buffer <<  " * " + ltrim(mid(buffer(ln),3))
  else
    out_buffer <<  buffer(ln)
  fi
next ln

tsave trim(command+"_new"), out_buffer
