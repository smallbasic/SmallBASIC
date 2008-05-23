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
  out << "<h1>Help Topics</h1>"
  out << "<a href=!Console>Console</a><br>"
  out << "<a href=!Data>Data</a><br>"
  out << "<a href=!Date>Date</a><br>"
  out << "<a href=!File>File</a><br>"
  out << "<a href=!Graphic>Graphics</a><br>"
  out << "<a href=!Language>Language</a><br>"
  out << "<a href=!Math>Math</a><br>"
  out << "<a href=!String>String</a><br>"
  out << "<a href=!System>System</a><br>"
  
  'show additional index information
  out << "<hr>"
  out << "<a href=~>[Index]</a><br>"  
  
  tsave getHelpOutputFilename, out
end

#
# sort by keyword
#
func sortKeywords(x, y)
  local result, cols_a, cols_b, kw_a, kw_b

  result = 0
  split x, ",", cols_a
  split y, ",", cols_b  
  if len(cols_a) > 0 and len(cols_b) > 0 then
    kw_a = unquote(cols_a(2))
    kw_b = unquote(cols_b(2))
    result = IFF(kw_a == kw_b, 0, IFF(kw_a > kw_b, 1, -1))
  fi
  sortKeywords = result
end

#
# show alphabetical groups
#
sub showIndex()
  local contents, keyword, row, nextKw, kw, nextChapter

  tload getHelpInputFilename, contents
  sort contents use sortKeywords(x,y)

  nextKw = ""

  for row in contents
    split row, ",", cols
    if len(cols) > 0 then
      keyword = unquote(cols(2))
      kw = mid(keyword, 1,1)      
      if (nextKw != kw) then
        out << "<hr><b>" + kw + "</b><br>"
        nextKw = kw
      fi
      nextChapter = unquote(cols(0)) 
      out << "<a href=" + keyword + ">" + keyword + &
             "</a> - " + nextChapter +"<br>"
    fi
  next

  out << "<hr><a href=^>[Topics]</a>"
  tsave getHelpOutputFilename, out
end

#
# show the contents of the given chapter
#
sub showChapter(chapter)
  local row, contents, nextChapter, out, keyword, chapterKeywords

  tload getHelpInputFilename, contents
  out << "<h1>" + chapter + "</h1>"
 
  for row in contents
    split row, ",", cols
    if len(cols) > 0 then
      nextChapter = unquote(cols(0))
      if (nextChapter == chapter) then
        chapterKeywords << unquote(cols(2))
      fi
    fi
  next row

  sort chapterKeywords
  for keyword in chapterKeywords
    out << "<a href=" + keyword + ">" + keyword + "</a><br>"
  next keyword

  out << "<hr><a href=~>[Index]</a> | <a href=^>[Topics]</a>"
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
        out << "<a href=~>[Index]</a> | <a href=^>[Topics]</a>"
        out << "<a href=!" + chapter + ">[Group]</a>"
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
    out << "<hr><a href=~>[Index]</a> | <a href=^>[Topics]</a>"
  fi
    
  tsave getHelpOutputFilename, out
end

#
# program entry
#
sub main
  if len(command) = 0 or left(command, 1) = "^" then
    showChapters
  elif left(command, 1) = "~" then
    showIndex
  elif left(command, 1) = "!" then
    showChapter right(command, len(command)-1)
  else
    showContext upper(command)
  fi
end

main
