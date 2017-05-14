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

  local result
  result = {}
  result.show=@display
  return result
end

func Grid()
  const kUP = 0
  const kDOWN = 1
  const kLEFT = 2
  const kRIGHT = 3
  const vecIndex = [kUP, kDOWN, kLEFT, kRIGHT]
  const directionVectors = [[-1, 0], [1, 0], [0, -1], [0, 1]]

  # Make a Deep Copy of This Object
  func clone()
    local result
    result = self
    result.map = self.map
    result.size = self.size
    return result
  end

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
      case kUP: result = moveUD(False)
      case kDOWN: result = moveUD(True)
      case kLEFT: result = moveLR(False)
      case kRIGHT: result = moveLR(True)
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

  func canMove(dirs)
    local i,x,y, checkingMoves, adjCellValue

    # Init Moves to be Checked
    checkingMoves = IFF(len(dirs)==0, vecIndex, dirs)
    for y = 0 to self.size
      for x = 0 to self.size
        # If Current Cell is Filled
        if self.map[x][y] != 0 then
          # Look Ajacent Cell Value
          for i in checkingMoves then
            move = directionVectors[i]
            adjCellValue = getCellValue((x + move[0], y + move[1]))
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
    local x, gridCopy
    dirs=IFF(len(dirs) == 0, vecindex,dirs)
    dim availableMoves
    for x in dirs
      gridCopy = clone()
      if gridCopy.move(x) then
        append availableMoves, x
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

  local result = {}
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

func GameManager()
  sub updateAlarm(currTime)
    if currTime - self.prevTime > timeLimit + allowance then
      self.over = True
    else
      'while time.clock() - self.prevTime < timeLimit + allowance
      '  pass
      'self.prevTime = time.clock()
    endif
  end

  sub start()
    local i, playerTurn, maxTile, gridCopy, move

    for i = 0 to self.initTiles
      insertRandonTile()
    next i

    self.displayer.display(self.grid)

    # Player AI Goes First
    playerTurn = true
    maxTile = 0

    self.prevTime = time.clock()
    while not self.isGameOver() and not self.over
      # Copy to Ensure AI Cannot Change the Real Grid to Cheat
      gridCopy = self.grid.clone()

      move = None

      if playerTurn then
        print "Player's Turn:",
        move = self.playerAI.getMove(gridCopy)
        'print actionDic[move]

        # Validate Move
        if move >= 0 and move < 4 then
          if self.grid.canMove([move]) then
            self.grid.move(move)

            # Update maxTile
            maxTile = self.grid.getMaxTile()
          else
            print "Invalid PlayerAI Move"
            self.over = True
          endif
        else
          print "Invalid PlayerAI Move - 1"
          self.over = True
        endif
      else
        print "Computer's turn:"
        move = self.computerAI.getMove(gridCopy)

        # Validate Move
        if move and self.grid.canInsert(move) then
          self.grid.setCellValue(move, self.getNewTileValue())
        else
          print "Invalid Computer AI Move"
          self.over = True
        endif
      endif
      if not self.over then
        self.displayer.display(self.grid)
      endif
      # Exceeding the Time Allotted for Any Turn Terminates the Game
      'self.updateAlarm(time.clock())
      playerTurn = IFF(playerTurn, false, true)
    wend
    print maxTile
  end

  func isGameOver()
    return not self.grid.canMove()
  end

  func getNewTileValue()
    if randint(0,99) < 100 * self.probability then
      return self.possibleNewTiles[0]
    else
      return self.possibleNewTiles[1]
    endif
  end

  sub insertRandonTile()
    local tileValue, cells, cell
    tileValue = self.getNewTileValue()
    cells = self.grid.getAvailableCells()
    cell = cells[randint(0, len(cells) - 1)]
    self.grid.setCellValue(cell, tileValue)
  end

  local result = {}
  return result
end

sub Game
  g = Grid()
  d = Display()
  g.insertTile(2,1,4)
  g.insertTile(2,2,4)
  d.show(g)
  logprint g.canMove([kDOWN])
  g.move(g.kDOWN)
  logprint g.canMove([kDOWN])
  d.show(g)
  pause
end

Game()
