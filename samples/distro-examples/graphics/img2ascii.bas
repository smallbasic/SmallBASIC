#http://mattmik.com/articles/ascii/ascii.html
const chars = "~`!@#$%^&*()+-=[]\;',./_+{}|:"
const block_width = 4
const block_height = 6

func calc_block(byref dat, h, y2, w, x2)
  local y,x,c,r,g,b,n
  
  n = 0
  for y = 0 to h
    for x = 0 to w
      c = -dat[y2 + y, x2 + x]
      r = ((c band 0xff0000) rshift 16)
      g = ((c band 0xff00) rshift 8)
      b = (c band 0xff)
      n += (r + g + b) / 3
    next x
  next y
  return n
end

func calc_color(byref dat, h, y2, w, x2)
  local y,x,c
  local r = 0
  local g = 0
  local b = 0
  local wh = w * h
  
  for y = 0 to h
    for x = 0 to w
      c = -dat[y2 + y, x2 + x]
      rx = ((c band 0xff0000) rshift 16)
      gx = ((c band 0xff00) rshift 8)
      bx= (c band 0xff)

      r += ((c band 0xff0000) rshift 16)
      g += ((c band 0xff00) rshift 8)
      b += (c band 0xff)
    next x
  next y

  r = r / wh
  g = g / wh
  b = b / wh
 
  local minv = min(r,g,b)
  local maxv = max(r,g,b)

  if (minv < maxv) then
    local diff = iff(maxv - minv == 0, 1, maxv - minv)
    r = int(255 * (r - minv) / diff)
    g = int(255 * (g - minv) / diff)
    b = int(255 * (b - minv) / diff)
    return rgb(r,g,b)
  else
    r = min(255, r)
    return rgb(r,r,r) 
  endif

end

func get_char(byref tbl, n)
  local r = tbl[n]
  if r > 0 then return r
  for sn in tbl.sorted
    if (sn > n) then
      ' round up to the nearest value
      tbl[n] = tbl[sn]
      return tbl[sn]
    endif
  next n
  return " "
end

# analyze the graphic data corresponding to each character in the character set.
func create_table
  local img,dat,i,ch,x2,n
  local w = txtw(chars)
  local h = txth(chars)
  local cw = txtw("1")

  cls: print chars
  img = image(0, 0, w, h, 1)
  img.save(dat)
  cls: showpage

  local vals = []
  local minv = maxint
  local maxv = 0
  
  for i = 1 to len(chars)
    ch = mid(chars, i, 1)
    x2 = ((i - 1) * cw)
    n = calc_block(dat, h - 1, 0, cw - 1, x2) / (w * h)
    minv = min(minv, n)
    maxv = max(maxv, n)
    vals << n
  next i
  
  ' scale the values from 0:255
  local tbl = {}
  for i = 1 to len(chars)
    ch = mid(chars, i, 1)
    diff = iff(maxv - minv == 0, 1, maxv - minv)
    n = 255 * (vals[i - 1] - minv) / diff
    vals[i - 1] = n
    tbl[n] = ch
  next i
  
  sort vals
  tbl["sorted"] = vals
  return tbl
end

sub imageToAscii(path)
  local img,dat,pic,bly,blx,y2,x2,n,minv,maxv,row
  local tbl = create_table()

  img = image(path)
  img.save(dat)

  local w = img.width
  local h = img.height
  local blw = w / block_width
  local blh = h / block_height
  local vals = []
  local minv = maxint
  local maxv = 0
  
  dim pic(blh, blw)

  for bly = 0 to blh - 1
    for blx = 0 to blw - 1
      y2 = bly * block_height
      x2 = blx * block_width
      n = calc_block(dat, block_height, y2, block_width - 1, x2) / (w * h)
      minv = min(minv, n)
      maxv = max(maxv, n)
      pic[bly,blx] = n 
    next blx
  next bly

  for bly = 0 to blh - 1
    for blx = 0 to blw - 1
      diff = iff(maxv - minv == 0, 1, maxv - minv)
      n = 255 * (pic[bly, blx] - minv) / diff
      y2 = iff(bly==0,1,bly) * block_height
      x2 = iff(blx==0,1,blx) * block_width
      color calc_color(dat, block_height, y2, block_width - 1, x2)
      print get_char(tbl, n);
    next blx
    print
  next bly
  showpage
end

imageToAscii("tree.png")
