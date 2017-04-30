const colorMap = {
   0 	: 0,
   2  : 1,
   4  : 2,
   8  : 3,
   16 : 4,
   32 : 5,
   64 : 6,
  128 : 7,
  256 : 8,
  512 : 9,
 1024 : 10,
 2048 : 11,
 4096 : 12,
 8192 : 13,
16384 : 14,
32768 : 15
}

const sz=xmax/8
const offs=10
const gap=3

func display(grid)
  local i,j,x,y,v,c
  y = 0
  x = 0
  for i = 0 to grid.size
    for j = 0 to grid.size
      v = grid.map[i][j]
      c = colorMap[v]
      color rgb(45,100,40), c
      rect x+offs, y+offs, x+offs+sz-gap, y+offs+sz-gap, c filled
      at x+offs+((sz-txtw(v))/2), y+offs+((sz-txth(v))/2)
      print v
      x += sz
    next j
    y += sz
    x = 0
  next i
end

w=window()
w.setFont(sz/4,0,0,0)

grid.size = 3
grid.map = [[0,2,4,8],[16,32,128,64],[1024,2048,512,128],[2,2,2,2]]
display(grid)

