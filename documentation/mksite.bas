#
# generate site.bas from sbasic_ref.csv
#
# ../src/platform/console/sbasic mksite.bas  | jq -M
#

tload "sbasic_ref.csv", lines
numLines = len(lines) - 1
ref={}

for i = 0 to numLines
  lineBuffer = lines(i)
  lineLen = len(lineBuffer) + 1
  currentCol = 0
  start = 1
  quoted = false
  package = ""
  item = {}
  
  for j = 1 to lineLen
    if (mid(lineBuffer, j, 1) == "\"" && mid(lineBuffer, j + 1, 1) == "\"") then
      rem change CSV escape to C escape "" -> \"
      lineBuffer = mid(lineBuffer, 1, j - 1) + "\\" + mid(lineBuffer, j + 1)
      j += 1
    else if (mid(lineBuffer, j, 1) == "\"") then
      quoted = !quoted
    else if (!quoted && (j == lineLen || mid(lineBuffer, j, 1) == ",")) then
      fieldLen = j - start
      if (mid(lineBuffer, start, 1) == "\"") then
        start += 1
        fieldLen -= 2
      endif  
      field = mid(lineBuffer, start, fieldLen)
      if (len(field) > 0) then
         select case currentCol
           case 0
             package = field
           case 1
             item["type"] = field
           case 2
             item["keyword"] = field
           case 3
             item["nodeID"] = field           
           case 4
             item["signature"] = field           
           case 5
             item["help"] = field
         end select
      endif
      currentCol++
      start = j + 1
    endif
  next j

  if (len(package) > 0) then
    ref[package] << item
  endif
next i

print ref
