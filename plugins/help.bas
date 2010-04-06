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
# create a html button
#
func navButton(caption, event)
  local s = "<input type='button' value='" + caption + &
            "' onclick='" + event + "'>&nbsp;"
  navButton = s
end

#
# show the available chapter names
#
sub showChapters(keywordNotFound)
  local out
  out << "<h1>Help Topics</h1>"
  out << "<a href=!Console>Console</a><br>"
  out << "<a href=!Data>Data</a><br>"
  out << "<a href=!Date>Date</a><br>"
  out << "<a href=!File>File</a><br>"
  out << "<a href=!Graphics>Graphics</a><br>"
  out << "<a href=!Language>Language</a><br>"
  out << "<a href=!Math>Math</a><br>"
  out << "<a href=!String>String</a><br>"
  out << "<a href=!System>System</a><br>"
 
  'show not found error
  if (keywordNotFound != 0 && trim(keywordNotFound) != "") then
    out << "<hr><b>Keyword not found: \"" + keywordNotFound + "\"</b>"
  fi
 
  'show additional index information
  out << "<hr> " + navButton("Index", "~") + "<br>"  
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

  out << "<hr>" + navButton("Topics", "^")
  tsave getHelpOutputFilename, out
end

#
# returns files in the given directory and below
#
sub get_files(dir, byref result)
  local list, f
  chdir dir
  list = files("*")
  for f in list
    if (isdir(f)) then
     get_files dir + "/" + f, result
     chdir dir
   else
     if (right(f, 4) == ".bas") then
       result << dir + "/" + f
     fi
   fi
  next i
end

#
# search for the given keyword in the samples dir
#
sub searchKeyword(keyword)
  local f_name, f_len, b_len, b, buffer, out, i, b, found

  ' append to the context output
  showContext keyword
  tload getHelpOutputFilename, out
  out <<  "<br>"

  dim result
  get_files env("PKG_HOME"), result
  f_len = len(result) -1
  
  keyword = upper(keyword)
  found = false

  for i = 0 to f_len
    f_name = result(i)
    tload f_name, buffer
    b_len = len(buffer) - 1
    for b = 0 to b_len
      if instr(upper(buffer(b)), keyword) != 0 then
        out << "<br><a href=" + f_name + ">" + f_name + "</a>"
        b = b_len ' only show first occurence
        found = true
      fi
    next b
  next i

  if (found == false) then
    out << "<b>No examples found</b>"
  fi

  ' save the updated context page
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

  out << "<hr>" + navButton("Index", "~") + navButton("Topics", "^")
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
        rem simple help in the log window
        logprint cat(1) + unquote(cols(4))
        logprint cat(0) + unquote(cols(5))
        
        chapter = unquote(cols(0))
        
        out << "<h1>" + chapter + " - " + keyword + "</h1>"
        out << unquote(cols(4)) 'synopsis
        out << "<br><br>"
        out << unquote(cols(5)) 'help information
        out << "<br><br><i>See also: <a href=http://smallbasic.sf.net/?q=node/" + cols(3) + ">"
        out << cols(2) + " home page on smallbasic.sf.net</a>"
        out << " - CTRL+F1 = log window help</i><hr>"

        rem draw next and previous links
        out << navButton("Index", "~")  
        out << navButton("Topics", "^")
        out << navButton(chapter, "!" + chapter) 
        if (i > 0) then
          split contents(i - 1), ",", cols
          kw = unquote(cols(2))
          out << navButton(kw, kw)
        fi
        if (i < max_contents - 1) then
          split contents(i + 1), ",", cols
          kw = unquote(cols(2))
          out << navButton(kw, kw)          
        fi
        out << navButton("Find Examples", "#" + keyword)
        exit for
      fi
    fi
  next i

  if (isarray(out) == 0) then
    showChapters keyword
  else    
    tsave getHelpOutputFilename, out
  endif
end

#
# program entry
#
sub main
  option base 0
  if len(command) = 0 or left(command, 1) = "^" then
    showChapters 0
  elif left(command, 1) = "~" then
    showIndex
  elif left(command, 1) = "!" then
    showChapter right(command, len(command)-1)
  elif left(command, 1) = "#" then
    searchKeyword right(command, len(command)-1)
  else
    showContext upper(command)
  fi
end

main
