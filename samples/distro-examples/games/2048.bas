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

  sub insertTile(x, y, v)
    self.map[y][x] = v
  end

  sub setCellValue(cell, v)
    local x,y
    (x,y) = cell
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
  result.setCellValue = @setCellValue
  result.map = [[0,0,0,0],[0,0,0,0],[0,0,0,0],[0,0,0,0]]
  result.size = 3
  return result
end

func PlayerAI()
  # https://en.wikipedia.org/wiki/Evaluation_function
  func getUtility(state)
    local x,y
    local numEmpty = 0
    local result = 0.0
    local highest = 0

    for y = 0 to self.size
      for x = 0 to self.size
        if (state.map[y][x] != 0) then
          result += state.map[y][x] * monotonicityPattern[y][x]
        endif
        if state.map[y][x] > highest then
          highest = state.map[y][x]
        else
          numEmpty += 1
          if (state.map[0][3] != highest) then
            result += highest * 100
          endif
          if (numEmpty == 0) then
            result = 0
          elif (result != 0) then
            result = 1 / result
          endif
        endif
      next x
    next y
    return result
  end

  # whether to stop evaluating lower depths
  func isTerminal(move, depth)
    if move == None then return True
    if depth >= self.maxDepth then return True
    'if ((time.clock() - self.startTime) > self.timeLimit) then
    '  self.timeout = True
    '  return True
    'endif
    return False
  end

  func maximise(state, a, b, move, depth)
    local availablemoves, child
    if (isTerminal(move, depth)) then return (move, getUtility(state))

    availableMoves = False
    maxMove = None
    maxUtility = ninf
    # 0 stands for "Up", 1 stands for "Down", 2 stands for "Left", and 3 stands for "Right".
    for nextMove in [0,3,1,2]
      child = cloneGrid(state)
      if child.move(nextMove) then
        availableMoves = True
        # get the estimated attack/response move
        [_, utility] = minimise(child, a, b, nextMove, depth + 1)
        if (utility > maxUtility) then [maxMove, maxUtility] = [nextMove, utility]
        if (maxUtility >= b) then
          # this node will not be selected in ancestor min function
          # we are larger than a sibling node so will be discounted
          exit for
        endif
        if (maxUtility > a) then a = maxUtility
      endif
    next nextmove
    if (availableMoves == False) then
      # terminal state detected
      return (move, self.getUtility(state))
    endif
    return (maxMove, maxUtility)
  end

  func minimise(state, a, b, move, depth)
    local child

    local availableCells = state.getAvailableCells()
    if (isTerminal(move, depth) or len(availableCells) == 0) then
      return (move, self.getUtility(state))
    endif

    local minimisingMoves = []
    if len(availableCells) > 2 then
      # filter to cells in the top right area
      for emptyPos in availableCells:
        [y, x] = emptyPos
        if (emptyCellPattern[y][x] == 0) then minimisingMoves.append(emptyPos)
      next emptypos
    else
      # try the remaining 1 or 2 cells
      minimisingMoves = availableCells
    endif
    if len(minimisingMoves) == 0 then return [move, self.getUtility(state)]

    [minMove, minUtility] = [None, inf]
    for nextMove in minimisingMoves
      child = cloneGrid(state)
      child.insertTile(nextMove, 2)
      [_, utility] = maximise(child, a, b, move, depth + 1)
      if (utility < minUtility) then [minMove, minUtility] = [move, utility]
      if (minUtility <= a) then exit for
      if (minUtility < b) then b = minUtility
    next nextmove
    return (minMove, minUtility)
  end

  func getMove(grid)
    self.startTime = ticks
    self.timeout = False
    [move, utility] = self.maximise(grid, ninf, inf, 0, 0)
    if (self.tuned == False) then
      if self.timeout == True then
        self.tuned = True
      else
        self.maxDepth += 1
      endif
    endif
    return move
  end

  result = {}
  result.startTime = 0
  result.startMaxScore = 0
  result.timeLimit = 0.09
  result.maxDepth = 4
  result.timeout = False
  result.tuned = False
  return result
end

func Game()
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

    for i = 1 to self.initTiles
      insertRandonTile()
    next i

    self.displayer.show(self.grid)

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
        self.displayer.show(self.grid)
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
    if rnd < self.probability then
      return self.possibleNewTiles[0]
    else
      return self.possibleNewTiles[1]
    endif
  end

  sub insertRandonTile()
    local tileValue = getNewTileValue()
    local cells = self.grid.getAvailableCells()
    local cell = cells[rnd * 1000 mod len(cells)]
    self.grid.setCellValue(cell, tileValue)
  end

  local result = {}
  result.start = @start
  result.grid = Grid()
  result.possibleNewTiles = [2, 4]
  result.probability = 0.9
  result.initTiles  = 2
'  result.computerAI = PlayerAI()
'  result.playerAI   = PlayerAI()
  result.displayer = Display()
  result.over       = False
  return result
end

sub _Test
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

g = Game()
g.start()
