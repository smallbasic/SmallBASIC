option predef autolocal

glo_x=123

sub sub1()
  sub inner1()
    glo_x=1234
  end
  sub viewFile()
    loc_y = 999
  end
  inner1()
  loc_y = 10
  for loc_j = 0 to 10
    loc_k = loc_j
  next loc_j
  viewFile()
  if loc_y != 10 then throw "err1: " + loc_y
end

sub sub2()
  if loc_y != 0 then throw "err2: " + loc_y
  loc_y = 20
  for loc_j = 0 to 20
    loc_k = loc_j
  next loc_j
end

sub1()
sub2()

if glo_x != 1234 then throw "err3: " + glo_x
if loc_j != 0 then throw "err4: " + loc_j
if loc_k != 0 then throw "err5: " + loc_k
if loc_y != 0 then throw "err6: " + loc_y
