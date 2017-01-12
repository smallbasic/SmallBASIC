#
# Generate the input file using the following SQL query:
#
# 1. import site data into mysql
#   mysql -u <username> -p -h localhost smallbasic < backup_xxx.sql
#
# 2. export from mysql:
#
# select pn.title as folder, n.title as filename, fdb.body_value as code
# from 
#  d7_field_data_body as fdb 
#     left join d7_node as n on n.nid = fdb.entity_id,
#  d7_node as pn
# where n.title like '%.bas'
#  and pn.nid = (select nid from d7_book
#    where mlid = (
#     select plid from d7_menu_links as m
#     join d7_book as cb on cb.mlid = m.mlid
#     where cb.nid = fdb.entity_id))
# order by pn.title, n.title
#
# 3. Select the JSON export format from mysql-workbench
#

split trim(command), " ", args
tload args(0), in_str, 1

# cleanup the mysql json
in_str = translate(in_str, "\'", "'")
in_str = translate(in_str, "'folder' :", "\"folder\":")
in_str = translate(in_str, "'filename' :", "\"filename\":")
in_str = translate(in_str, "'code' :", "\"code\":")
in_map = array(in_str)

mk_files("samples")

sub mk_files(prefix)
  local in_map_len = len(in_map) - 1
  local folder, filename, code

  if (!exist(prefix)) then
    mkdir prefix
  endif

  for i = 0 to in_map_len
    folder = prefix + "/" + translate(lower(in_map(i).folder), "/", " ")
    filename = folder + "/" + lower(in_map(i).filename)
    code = in_map(i).code
    if (!exist(folder)) then
      mkdir folder
    endif
    tsave filename, code
  next i
end
