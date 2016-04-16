#
# Generate the input file using the following SQL query:
#
# select FDB.body_value, FDCB.comment_body_value, FDB.entity_id
# from d7_field_data_body as FDB
# left outer join d7_comment as C on C.nid = FDB.entity_id
# left outer join d7_field_data_comment_body as FDCB on FDCB.entity_id = C.cid
# where FDB.body_value like '%sbasic reference%'
# order by FDB.body_value, FDB.entity_id
#
# Then export the results in JSON format
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
