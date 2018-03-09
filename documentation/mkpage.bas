#
# generates a SmallBASIC program from an input template
#
# loosely based on https://jekyllrb.com/docs/variables/
#
# {% REM code %}
# {{ variable-to-print }}
#

rem ----------(%----%)-------
rem ^         ^     ^
rem sIndex    p1    p2

func is_code(c2, c4)
  if (c2 == 0) then return false
  return iff(c4 == 0 or c2 < c4, true, false)
end

func is_var(c2, c4)
  if (c4 == 0) then return false
  return iff(c2 == 0 or c4 < c2, true, false)
end

sub text_lines(s)
  local lines, num_lines, i, semi, st

  split s, chr(10), lines
  num_lines = len(lines) - 1
  for i = 0 to num_lines
    semi = iff(i == num_lines, ";", "")
    st = translate(lines[i], "\"", "\\\"")
    st = translate(st, chr(13), "")
    if (len(trim(st)) > 0) then
      print "print (\"" + st + "\")" + semi
    endif
  next i
end

sub process(filename)
  local s, sEnd, sIndex, c1, c2, c3, c4
  local file, code

  tload filename, s, 1
  sEnd = len(s)
  sIndex = 1

  func get_pair(pair1, pair2)
    local p1, p2
    p1 = instr(sIndex, s, pair1)
    p2 = 0
    if (p1 > 0) then
      p2 = instr(p1 + len(pair1), s, pair2)
    endif
    return [p1, p2]
  end

  while 1
    [c1, c2] = get_pair("{%", "%}")
    [c3, c4] = get_pair("{{", "}}")
    if (is_code(c2, c4)) then
      text_lines(mid(s, sIndex, c1 - sIndex))
      code = trim(mid(s, c1 + 2, c2 - c1 - 2))
      if (instr(code, "include ") == 1) then
        process("_includes/" + rightof(code, "include "))
      else
        print code
      endif
      sIndex = c2 + 2
    else if (is_var(c2, c4)) then
      text_lines(mid(s, sIndex, c3 - sIndex))
      print "print (" + mid(s, c3 + 2, c4 - c3 - 2) + ");"
      sIndex = c4 + 2
    else
      exit loop
    endif
  wend

  if (sIndex < sEnd) then
    text_lines(mid(s, sIndex, sEnd))
  endif
end

print "import site"
print "if (exist(command)) then tload command, content, 1"
process(trim(command))

