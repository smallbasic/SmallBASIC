'
' generate a local help reference (sbasic_ref.csv) from the drupal database
'

import mysql

'
' extract property value
'
func get_prop(prop, body)
  local s, i, i_end

  i = instr(body, prop+"=")
  i_end = instr(i, body, chr(13))

  if (i != 0 && i_end != 0) then
    i += len(prop) + 1
    s = mid(body, i, i_end - i)
  else
    s = ""
  fi     
  get_prop = s
end

'
' extract reference block
'
func get_ref_block(body)
  local s, i, i_end, ref_begin, ref_end

  ref_begin = "<!-- sbasic reference -->"
  ref_end = "<!-- end heading block -->"

  i = instr(body, ref_begin)
  i_end = instr(body, ref_end)

  if (i != 0 && i_end != 0) then
    i += len(ref_begin)
    s = mid(body, i, i_end - i)
  else
    s = ""
  fi

  get_ref_block = s
end

'
' convert results to CVS
'
sub to_csv(nid, body)
  local s, group, cmd_type, kwd, syntax

  body = get_ref_block(body)
  group = get_prop("group", body)
  cmd_type = get_prop("type", body)
  kwd = get_prop("keyword", body)
  syntax = get_prop("syntax", body)
  brief = get_prop("brief", body)

  ? group; ","; cmd_type; ","; kwd; ","; nid; ",\""; syntax; "\",\""; brief; "\""
end

'
' connect to local instance of mysql
'
h = mysql.connect("localhost", "sbasic", "root", "")
result = mysql.query(h, "SELECT nid, body FROM d5_node_revisions where format=7")
mysql.disconnect h

'
' process results
'
result_len = (len(result) / 2) - 1
for i = 0 to result_len
  nid = result(i,0)
  body = result(i,1)
  to_csv nid, body
next i
