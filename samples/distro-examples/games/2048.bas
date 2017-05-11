func Display()
  colorMap = {
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

  sz = xmax/8
  offs = 10
  gap = 3
  w=window()
  w.setFont(sz/4,0,1,0)
  color 0,rgb(15,40,15)
  cls

  sub display(grid)
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

  result = {}
  result.show=@display
  return result
end

func Grid()
  sub insertTile(x,y,v)
    self.map[y][x] = v
  end

  func getAvailableCells()
    local x,y
    dim cells
    for y = 0 to self.size
      for x = 0 to self.size
        if self.map[y][x] == 0 then
          append cells, [x,y]
        endif
      next x
    next y
    return cells
  end

  # Return the Tile with Maximum Value
  func getMaxTile()
    local x,y
    local maxTile = 0
    for y = 0 to self.size
      for x = 0 to self.size
        maxTile = max(maxTile, self.map[y][x])
      next x
    next y
    return maxTile
  end

  # Check If Able to Insert a Tile in Position
  func canInsert(x, y)
    return getCellValue(x, y) == 0
  end

  # Move the Grid
  sub move(dir)
    local result
    select case dir
      case 0: result = moveUD(False)
      case 1: result = moveUD(True)
      case 2: result = moveLR(False)
      case 3: result = moveLR(True)
      case else: throw "Invalid move:" + dir
    end select
    return result
  end

  # Move Up or Down
  func moveUD(move_up)
    local x,y,y1,y2,incr,moved
    dim cells

    incr = iff(move_up, 1, -1)
    y1 = iff(move_up, 0, self.size)
    y2 = iff(move_up, self.size, 0)

    moved = False
    for x = 0 to self.size
      erase cells
      for y = y1 to y2 step incr
        cell = self.map[y][x]
        if cell != 0 then append cells, cell
      next y

      merge(cells)

      for y = y1 to y2 step incr
        if len(cells) then
          value = cells[0]
          delete cells, 0
        else
          value = 0
        endif
        if self.map[y][x] != value then moved = True
        self.map[y][x] = value
      next y
    next x
    return moved
  end

  # move left or right
  func moveLR(move_left)
    local x,y,x1,x2,incr,moved
    dim cells

    incr = iff(move_left, 1, -1)
    x1 = iff(move_left, 0, self.size)
    x2 = iff(move_left, self.size, 0)

    moved = False
    for y = 0 to self.size
      erase cells
      for x = x1 to x2 step incr
        cell = self.map[y][x]
        if cell != 0 then append cells, cell
      next y
      merge(cells)
      for x = x1 to x2 step incr
        if len(cells) then
          value = cells[0]
          delete cells, 0
        else
          value = 0
        endif
        if self.map[y][x] != value then moved = True
        self.map[y][x] = value
      next y
    next y
    return moved
  end

  # Merge Tiles
  sub merge(byref cells)
    if len(cells) <= 1 then return
    local i = 0
    while i < len(cells) - 1
      if cells[i] == cells[i+1] then
        cells[i] *= 2
        delete cells, i+1
      endif
      i += 1
    wend
  end

  sub canMove(dirs)
    local x,y
    # Init Moves to be Checked
    checkingMoves = Set(dirs)
    for y = 0 to self.size
      for x = 0 to self.size
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
      next x
    next y
    return False
  end

  # Return All Available Moves
  func getAvailableMoves(dirs)
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

  func crossBound(x, y)
    return x < 0 or x >= self.size or y < 0 or y >= self.size
  end

  func getCellValue(x, y)
    if not crossBound(x, y) then
      return self.map[y][x]
    else
      return -1
    endif
  end

  result = {}
  result.getAvailableCells = @getavailablecells
  result.getMaxTile = @getmaxtile
  result.canInsert = @caninsert
  result.canMove = @canmove
  result.move = @move
  result.insertTile = @inserttile
  result.merge = @merge
  result.getAvailableMoves = @getavailablemoves
  result.getCellValue = @getCellValue
  result.map = [[0,0,0,0],[0,0,0,0],[0,0,0,0],[0,0,0,0]]
  result.size = 3
  return result
end

sub Game
  g = Grid()
  d = Display()
  g.insertTile(2,1,4)
  g.insertTile(2,2,4)
  d.show(g)
  pause
  g.move(3)
  d.show(g)
  pause
end

Game()
