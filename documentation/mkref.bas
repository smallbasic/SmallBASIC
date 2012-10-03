#
# Generate the input file using the following SQL query:
#
# select FDB.body_value, FDCB.comment_body_value
# from d7_field_data_body as FDB
# left outer join d7_comment as C on C.nid = FDB.entity_id
# left outer join d7_field_data_comment_body as FDCB on FDCB.entity_id = C.cid
# where FDB.body_value like '%sbasic reference%'
# order by FDB.body_value
#

tload trim(command), rows

in_block = false
more_text = false
comment_text = false
item_id = 1
group_id = ""

? "SmallBASIC Language reference"
? "See: http://smallbasic.sourceforge.net/?q=node/201"
? 

for i in rows
  if left(i, 4) == "bc. " then
     i = mid(i, 4)
  elif left(i, 5) == "bc.. " then
     i = mid(i, 5)
  elif left(i, 3) == "p. " then
     i = mid(i, 3)
  elif left(i, 4) == "bq. " then
     i = mid(i, 4)
  endif

  if i == "\"<!-- sbasic reference -->" then
     in_block = true
  elif instr(i, "<!-- end heading block -->") == 1 then
     in_block = false
     if i == "<!-- end heading block -->" then
        more_text = true
     elif instr(i, "\",NULL") == 0 then
        comment_text = true
        ? mid(i, 30)
     else 
        rem line separator after brief
        ?
     endif
  elif in_block == true then
     if instr(i, "keyword=") == 1 then
       ? item_id + ". (" + group_id + ") " + mid(i, 9)
       ?
       item_id ++
     elif instr(i, "brief=") == 1 then
       ? mid(i, 7)
     elif instr(i, "group=") == 1 then
       group_id = mid(i, 7)
     endif
  elif comment_text == true then
     if len(i) = 1 then
        comment_text = false
        ?
     else 
       ? i
     endif
  elif more_text == true then
     if instr(i, "\",NULL") == 1 then
        more_text = false
        ?
     else 
       ? i
     endif
  endif
next i

