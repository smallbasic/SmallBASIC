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

func Grid()
  sub insertTile(x,y,v)
    self.map[x][y]=v
  end

  func getAvailableCells()
    local x,y
    dim cells
    for x = 0 to self.size
      for y = 0 to self.size
        if self.map[x][y] == 0 then
          append cells (x,y)
        endif
      next y
    next x
    return cells
  end

  # Return the Tile with Maximum Value
  func getMaxTile()
    local x,y
    local maxTile = 0
    for x = 0 to self.size
      for y = 0 to self.size
        maxTile = max(maxTile, self.map[x][y])
      next y
    next x
    return maxTile
  end

  # Check If Able to Insert a Tile in Position
  func canInsert(self, pos)
    return self.getCellValue(pos) == 0
  end

  # Move the Grid
  func move(self, dir)
    dir = int(dir)
    if dir == kUP then return self.moveUD(False)
    if dir == kDOWN then return self.moveUD(True)
    if dir == kLEFT then return self.moveLR(False)
    return self.moveLR(True)
  end

  # Move Up or Down
  func moveUD(move_down)
    local moved,i,j
    'r = range(self.size -1, -1, -1) if down else range(self.size)
    moved = False
    dim cells

    for j = 0 to self.size
      erase cells
      for i in r
        cell = self.map[i][j]
        if cell != 0 then cells.append(cell)
        self.merge(cells)
      next i
      for i in r
        #value = cells.pop(0) if cells else 0
        if self.map[i][j] != value then moved = True
        self.map[i][j] = value
      next i
    next j
    return moved
  end

  # move left or right
  func moveLR(move_right)
    local moved,i,j
    dim cells
    'r = range(self.size - 1, -1, -1) if right else range(self.size)

    moved = False
    for i = 0 to self.size
      erase cells
      for j in r
        cell = self.map[i][j]
        if cell != 0 then cells.append(cell)
        self.merge(cells)
      next j
      for j in r
        #value = cells.pop(0) if cells else 0
        if self.map[i][j] != value then moved = True
        self.map[i][j] = value
      next j
    next i
    return moved
  end

  # Merge Tiles
  sub merge(cells)
    local i
    if len(cells) <= 1 then return cells
    i = 0
    while i < len(cells) - 1
      if cells[i] == cells[i+1] then
        cells[i] *= 2
        #del cells[i+1]
      endif
      i += 1
    wend
  end

  sub canMove(dirs)
    local x,y
    # Init Moves to be Checked
    checkingMoves = Set(dirs)
    for x = 0 to self.size
      for y = 0 to self.size
        # If Current Cell is Filled
        if self.map[x][y] then
          # Look Ajacent Cell Value
          for i in checkingMoves then
            move = directionVectors[i]
            adjCellValue = self.getCellValue((x + move[0], y + move[1]))
            # If Value is the Same or Adjacent Cell is Empty
            if adjCellValue == self.map[x][y] or adjCellValue == 0 then
              return True
            endif
          next i
        # Else if Current Cell is Empty
        elif self.map[x][y] == 0 then
          return True
        endif
      next y
    next x
    return False
  end

  # Return All Available Moves
  sub getAvailableMoves(dirs)
    local x
    dim availableMoves
    for x in dirs
      gridCopy = self.clone()

      if gridCopy.move(x) then
        availableMoves.append(x)
      endif
    next x
    return availableMoves
  end

  sub crossBound(x, y)
    return x < 0 or x >= self.size or y < 0 or y >= self.size
  end

  sub getCellValue(pos)
    if not self.crossBound(pos) then
      return self.map[pos[0]][pos[1]]
    else
      return false
    endif
  end

  result = {}
  result.getAvailableCells = @getavailablecells
  result.getMaxTile = @getmaxtile
  result.canInsert = @getmaxtile
  result.move = @move
  result.insertTile = @inserttile
  result.merge = @merge
  result.getAvailableMoves = @getavailablemoves
  result.getCellValue = @getCellValue
  result.map = [[0,0,0,0],[0,0,0,0],[0,0,0,0],[0,0,0,0]]
  result.size = 3
  return result
end

func Game()
  local w
  w=window()
  w.setFont(sz/4,0,0,0)


end

g= Grid()
g.insertTile(2,1,1)
? g.map
'grid.size = 3
'grid.map = [[0,2,4,8],[16,32,128,64],[1024,2048,512,128],[2,2,2,2]]
'display(grid)

