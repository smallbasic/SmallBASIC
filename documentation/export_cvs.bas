'
' convert the CSV user guide into input for drupal using
' the HTML input filter
'

tload trim(command), lines

func unquote(s)
  if (left(s, 1) == "\"") then
    ' remove leading/trailing quotes
    s = mid(s, 2, len(s) - 2)
  fi

  ' unescape escaped quotes for CVS export
  unquote = translate(s, "\"\"", "\"")
end

for line_next in lines
  split line_next, ",", cols, "\"\""

  if (len(cols) > 0) then
    ? "<h2>" + unquote(cols(2)) + "</h2>"
    ? "<!-- sbasic reference -->"
    ? "group=" + unquote(cols(0))
    ? "type=" + unquote(cols(1))
    ? "keyword=" + unquote(cols(2))
    ? "syntax=" + unquote(cols(3))
    if (len(cols) > 4) then
      ? "brief=" + unquote(cols(4))
    fi
    ? "<!-- end heading block -->"
    ?
  fi

next line


