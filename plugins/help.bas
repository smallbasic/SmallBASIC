# $Id$

#
# return the help database filename
#
func getHelpInputFilename
  getHelpInputFilename = env("PKG_HOME") + "/sbasic_ref.csv"
end

#
# write the help html file
#
func getHelpOutputFilename
  getHelpOutputFilename = env("BAS_HOME") + "help.html"
end

#
# remove any surrounding quotes from the given string
#
func unquote(s)
  if (left(s, 1) == "\"") then
    unquote = mid(s, 2, len(s)-2)
  else
    unquote = s
  fi
end

#
# show the available chapter names
#
sub showChapters
  local out
  out << "<h1>Help Index</h1>"
  out << "<a href=!Console>Console</a><br>"
  out << "<a href=!Data>Data</a><br>"
  out << "<a href=!Date>Date</a><br>"
  out << "<a href=!File>File</a><br>"
  out << "<a href=!Graphic>Graphics</a><br>"
  out << "<a href=!Language>Language</a><br>"
  out << "<a href=!Math>Math</a><br>"
  out << "<a href=!String>String</a><br>"
  out << "<a href=!System>System</a><br>"
  tsave getHelpOutputFilename, out
end

#
# show the contents of the given chapter
#
sub showChapter(chapter)
  local row, contents, nextChapter, out, keyword

  tload getHelpInputFilename, contents
  out << "<h1>" + chapter + "</h1>"
 
  for row in contents
    split row, ",", cols
    if len(cols) > 0 then
      nextChapter = unquote(cols(0))
      if (nextChapter == chapter) then
        keyword = unquote(cols(2))
        out << "<a href=" + keyword + ">" + keyword + "</a><br>"
      fi
    fi
  next row

  out << "<hr><a href=^>[Index]</a>"
  tsave getHelpOutputFilename, out
end

#
# show context sentive help
#
sub showContext(keyword)
  local row, contents, kw, out

  tload getHelpInputFilename, contents
  max_contents = len(contents) - 1
  out = 0
  
  for i = 0 to max_contents
    split contents(i), ",", cols, "\"\""
    if len(cols) > 0 then
      kw = unquote(cols(2))
      if (kw == keyword) then
        chapter = unquote(cols(0))
        out << "<h1>" + chapter + " - " + keyword + "</h1>"
        out << unquote(cols(3)) 'synopsis
        out << "<br><br>"
        out << unquote(cols(4)) 'help information
        out << "<hr>"

        rem draw next and previous links
        out << "<a href=^>[Index]</a>"
        out << "<a href=!" + chapter + ">[Up]</a>"
        if (i > 0) then
          split contents(i - 1), ",", cols
          kw = unquote(cols(2))
          out << "<a href=" + kw + ">[" + kw + "]</a>"
        fi
        if (i < max_contents - 1) then
          split contents(i + 1), ",", cols
          kw = unquote(cols(2))
          out << "<a href=" + kw + ">[" + kw + "]</a>"
        fi
        exit for
      fi
    fi
  next i

  if (isarray(out) == 0) then
    out << "Keyword not found: " + keyword
    out << "<hr><a href=^>[Index]</a>"
  fi
    
  tsave getHelpOutputFilename, out
end

#
# program entry
#
sub main
  if len(command) = 0 or left(command, 1) = "^" then
    showChapters
  elif left(command, 1) = "!" then
    showChapter right(command, len(command)-1)
  else
    showContext upper(command)
  fi
end

main

