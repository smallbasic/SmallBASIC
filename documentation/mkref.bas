#
# Generate the input file using the following SQL query:
#
# 1. import site data into mysql
#   mysql -u <username> -p -h localhost smallbasic < backup_xxx.sql
#
# 2. export from mysql
#
# select FDB.body_value, FDCB.comment_body_value, FDB.entity_id
# from d7_field_data_body as FDB
# left outer join d7_comment as C on C.nid = FDB.entity_id
# left outer join d7_field_data_comment_body as FDCB on FDCB.entity_id = C.cid
# where FDB.body_value like '%sbasic reference%'
# order by FDB.body_value, FDB.entity_id
#
# 3. Select the JSON export format from mysql-workbench
#

split trim(command), " ", args
tload args(0), in_str, 1

# cleanup the mysql json
in_str = translate(in_str, "\'", "'")
in_str = translate(in_str, "'entity_id' :", "\"entity_id\":")
in_str = translate(in_str, "'body_value' :", "\"body_value\":")
in_str = translate(in_str, "'comment_body_value' :", "\"comment_body_value\":")
in_map = array(in_str)

if (len(args) == 2 && args(1) == "txt") then
  mk_text_reference(in_map)
elif (len(args) == 2 && args(1) == "bas") then
  mk_bas(in_map)
elif (len(args) == 2 && args(1) == "jekyll") then
  mk_jekyll(in_map)
elif (len(args) == 2 && args(1) == "markdown") then
  mk_markdown(in_map)
elif (len(args) == 2 && args(1) == "test") then
  mk_test(in_map)
else
  mk_help(in_map)
fi

func get_field(row, key, escq)
  local n1, n2, result
  n1 = instr(row, key) + len(key)
  n2 = instr(n1, row, "\\r") - n1
  result = mid(row, n1, n2)
  if (escq) then
    result = translate(result, "\\\"", "\"\"")
  else
    result = translate(result, "\\\"", "\"")
  endif
  get_field = result
end

func fix_comments(comments, keyword)
  comments = translate(comments, "p. ", "")
  comments = translate(comments, "bq. ", "")
  comments = translate(comments, "bc. ", "")
  comments = translate(comments, "bc.. ", "")
  comments = translate(comments, "__", "")
  comments = translate(comments, "**", "")
  comments = translate(comments, "&nbsp;", " ")
  comments = translate(comments, "\\\"", "\"")
  comments = translate(comments, "<code>", "----[ " + keyword + " example. cut here ]--->" + chr(10))
  comments = translate(comments, "</code>", chr(10) + "<---[ cut here ]----" + chr(10))
  comments = translate(comments, "\\r\\n\\r\\n", chr(10))
  comments = translate(comments, "\\r\\n", chr(10))
  fix_comments = comments
end

func fix_comments_jekyll(comments, keyword)
  comments = translate(comments, "<code>", "<pre>" + chr(10))
  comments = translate(comments, "</code>", chr(10) + "</pre>" + chr(10))
  comments = translate(comments, "p. ", "<p>")
  comments = translate(comments, "bc. ", "<pre>")
  comments = fix_comments(comments, keyword)
  fix_comments_jekyll = comments
end

func fix_comments_pandoc(comments, keyword)
  comments = translate(comments, "<code>", chr(10) + "~~~" + chr(10))
  comments = translate(comments, "<?code>", chr(10) + "~~~" + chr(10))
  comments = translate(comments, "</code>", chr(10) + "~~~" + chr(10))
  comments = translate(comments, "<strong>", "**")
  comments = translate(comments, "</strong", "**")
  comments = translate(comments, "<cite>", "")
  comments = translate(comments, "</cite", "")
  comments = translate(comments, "<blockquote>", "> ")
  comments = translate(comments, "</blockquote>", "")
  comments = translate(comments, "</blockqoute>", "")
  comments = translate(comments, "p. ", "")
  comments = translate(comments, "bc. ", "> ")
  comments = fix_comments(comments, keyword)
  return comments
end

sub mk_bas(byref in_map)
  local i, row, group, keyword
  local in_map_len = len(in_map) - 1
  for i = 0 to in_map_len
    row = in_map(i).body_value
    keyword = get_field(row, "keyword=", true)
    group = get_field(row, "group=", true)
    ? group + " " + keyword
    while (i + 1 < in_map_len && in_map(i).entity_id == in_map(i + 1).entity_id)
      i++
    wend
  next i
end

sub mk_help(byref in_map)
  local i, row, group, type, keyword, syntax, brief
  local in_map_len = len(in_map) - 1
  for i = 0 to in_map_len
    row = in_map(i).body_value
    group = get_field(row, "group=", true)
    type = get_field(row, "type=", true)
    keyword = get_field(row, "keyword=", true)
    syntax = get_field(row, "syntax=", true)
    brief = get_field(row, "brief=", true)
    while (i + 1 < in_map_len && in_map(i).entity_id == in_map(i + 1).entity_id)
      i++
    wend
    ? group + "," + type + "," + keyword + "," + in_map(i).entity_id + ",\"" + syntax + "\",\"" + brief + "\""
  next i
end

sub mk_text_reference(byref in_map)
  local i, row, group, type, keyword, syntax, brief, comments, counter
  local in_map_len = len(in_map) - 1
  local end_block = "<!-- end heading block -->"

  ? "SmallBASIC Language reference"
  ?
  ? "Online:
  ? "http://smallbasic.sourceforge.net/?q=node/201"
  ?
  ? "Generated with:
  ? "https://github.com/smallbasic/SmallBASIC/blob/master/documentation/mkref.bas
  ?
  ? "   _____                 _ _ ____           _____ _____ _____
  ? "  / ____|               | | |  _ \   /\    / ____|_   _/ ____|
  ? " | (___  _ __ ___   __ _| | | |_) | /  \  | (___   | || |
  ? "  \___ \| '_ ` _ \ / _` | | |  _ < / /\ \  \___ \  | || |
  ? "  ____) | | | | | | (_| | | | |_) / ____ \ ____) |_| || |____
  ? " |_____/|_| |_| |_|\__,_|_|_|____/_/    \_\_____/|_____\_____|
  ?
  ?

  counter = 0
  for i = 0 to in_map_len
    row = in_map(i).body_value
    group = get_field(row, "group=", false)
    type = get_field(row, "type=", false)
    keyword = get_field(row, "keyword=", false)
    syntax = get_field(row, "syntax=", false)
    brief = get_field(row, "brief=", false)
    counter++

    ? counter + ". (" + group + ") " + keyword
    ?
    ? syntax
    ?
    ? brief
    ?
    pos = instr(row, end_block) + len(end_block)
    if (pos < len(row)) then
      ? fix_comments(mid(row, pos), keyword)
    endif
    comments = in_map(i).comment_body_value
    if (comments != "NULL") then
      ? fix_comments(comments, keyword)
    endif
    while (i + 1 < in_map_len && in_map(i).entity_id == in_map(i + 1).entity_id)
      i++
      ? fix_comments(in_map(i).comment_body_value, keyword)
    wend
  next i
end

'
' make post files for smallbasic.github.io
'
' local test: $ bundle exec jekyll serve
'
sub mk_jekyll(byref in_map)
  local i, row, group, type, keyword, syntax, brief, comments, buffer, filename
  local in_map_len = len(in_map) - 1
  local end_block = "<!-- end heading block -->"
  dim buffer

  for i = 0 to in_map_len
    erase buffer
    row = in_map(i).body_value
    group = get_field(row, "group=", false)
    type = get_field(row, "type=", false)
    keyword = get_field(row, "keyword=", false)
    syntax = get_field(row, "syntax=", false)
    brief = get_field(row, "brief=", false)
    buffer << "---"
    buffer << "permalink: /" + in_map(i).entity_id
    buffer << "layout: post"
    buffer << "title:  \"" + keyword + "\""
    buffer << "categories: " + lower(group)
    buffer << "---"
    buffer << group
    buffer << ""
    buffer << syntax
    buffer << ""
    buffer << brief
    buffer << ""
    pos = instr(row, end_block) + len(end_block)
    if (pos < len(row)) then
      buffer << fix_comments_jekyll(mid(row, pos), keyword)
    endif
    comments = in_map(i).comment_body_value
    if (comments != "NULL") then
      buffer << fix_comments_jekyll(comments, keyword)
    endif
    while (i + 1 < in_map_len && in_map(i).entity_id == in_map(i + 1).entity_id)
      i++
      buffer << fix_comments_jekyll(in_map(i).comment_body_value, keyword)
    wend
    filename = "2016-06-04-" + lower(group) + "-" + lower(keyword) + ".markdown"
    tsave filename, buffer
  next i
end

'
' must contain at least 3 | chars
'
func is_table(s)
  local z
  z = instr(0, s, "|")
  if (z!=1) then return false

  z = instr(z, s, "|")
  if (z==0) then return false

  z = instr(z, s, "|")
  if (z==0) then return false

  return true
end

'
' generate the table pattern
'
func table_markdown(s)
  local result = ""
  local s_len = len(s)
  local i
  for i = 1 to s_len
    if (mid(s, i, 1) == "|") then
      result += " "
    else
      result += "-"
    endif
  next i
  return result
end

'
' convert textile to markdown
'
func table_row(table_md, s)
  local out = trim(leftoflast(rightof(s, "|"), "|"))
  local j = instr(table_md, " ")
  local x = instr(out, "|")
  local n = max(1, j - x)
  return translate(out, "|", space(n))
end

'
' convert textile to markdown
'
func update_tables(byref in_str)
  local in_table = false
  local table_md = ""
  local out = ""
  local row, rows

  split in_str, chr(10), rows
  for row in rows
    if (is_table(row)) then
      if (!in_table) then
        table_md = ltrim(table_markdown(row))
        in_table = true
        out += table_md
        out += chr(10)
      endif
      out += table_row(table_md, row)
    else
      if (in_table) then
        out += table_md
        out += chr(10)
        in_table = false
      endif
      out += row
    endif
    out += chr(10)
  next row
  return out
end

'
' make post files for smallbasic.github.io
'
sub mk_markdown(byref in_map)
  local i, row, group, type, keyword, syntax, brief, comments, buffer, filename
  local in_map_len = len(in_map) - 1
  local end_block = "<!-- end heading block -->"

  sub append_buf(s)
    buffer += s
    buffer += chr(10)
  end

  for i = 0 to in_map_len
    buffer=""
    row = in_map(i).body_value
    group = get_field(row, "group=", false)
    type = get_field(row, "type=", false)
    keyword = get_field(row, "keyword=", false)
    syntax = get_field(row, "syntax=", false)
    brief = get_field(row, "brief=", false)
    append_buf("# " + keyword)
    append_buf("")
    append_buf("> " + syntax)
    append_buf("")
    append_buf(brief)
    append_buf("")
    pos = instr(row, end_block) + len(end_block)
    if (pos < len(row)) then
      append_buf(fix_comments_pandoc(mid(row, pos), keyword))
    endif
    comments = in_map(i).comment_body_value
    if (comments != "NULL") then
      append_buf(fix_comments_pandoc(comments, keyword))
    endif
    while (i + 1 < in_map_len && in_map(i).entity_id == in_map(i + 1).entity_id)
      i++
      append_buf(fix_comments_pandoc(in_map(i).comment_body_value, keyword))
    wend

    filename = in_map(i).entity_id + "-" + lower(group) + "-" + lower(keyword) + ".markdown"
    filename = translate(filename, " ", "")
    buffer = update_tables(buffer)

    if (len(filename) > 200) then
      filename = in_map(i).entity_id + ".markdown"
    endif
    tsave filename, buffer
  next i
end

func cmpFunc(l, r)
  local f1 = lower(l)
  local f2 = lower(r)
  cmpFunc = IFF(f1 == f2, 0, IFF(f1 > f2, 1, -1))
end

func fname(s)
  local result
  result = trim(leftof(s, " "))
  if (len(result) == 0) then
    result = s
  endif
  fname = result
end

'
' make a test program from the syntax field
'
sub mk_test(byref in_map)
  local i, row, type, prev, syntax, el
  local in_len = len(in_map) - 1
  dim cmds, funcs
  for i = 0 to in_len
    row = in_map(i).body_value
    type = get_field(row, "type=", false)
    syntax = trim(get_field(row, "syntax=", false))
    if (type == "command") then
      cmds << syntax
    else if (type == "function") then
      funcs << syntax
    endif
  next i

  sort cmds use cmpFunc(x,y)
  in_len = len(cmds) - 1
  prev = ""
  for i = 0 to in_len
    row = cmds(i)
    if (row != prev) then
      print "print \"" + fname(row) + ":\" ':" + row
    endif
    prev = row
  next i

  sort funcs use cmpFunc(x,y)
  in_len = len(funcs) - 1
  prev = ""
  for i = 0 to in_len
    row = funcs(i)
    if (row != prev) then
      print "print \"" + fname(row) + ":\" + " + row
    endif
    prev = row
  next i
end
