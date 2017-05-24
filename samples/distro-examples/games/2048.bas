const kUP = 0
const kDOWN = 1
const kLEFT = 2
const kRIGHT = 3
const vecIndex = [kUP, kDOWN, kLEFT, kRIGHT]
const directionVectors = [[-1, 0], [1, 0], [0, -1], [0, 1]]
const inf = maxint
const ninf = -maxint - 1
const timeLimit = 5000
const emptyCellPattern =&
[[0,0,0,0],&
 [1,1,0,0],&
 [1,1,1,0],&
 [1,1,1,0]]
const monotonicityPattern =&
 [[16,  8,  1,  0],&
  [32, 16,  8,  1],&
  [64, 32, 16,  8],&
  [128,64, 32, 16]]
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
const actionDict = {
 0: "UP",
 1: "DOWN",
 2: "LEFT",
 3: "RIGHT"
}
const sz = min(xmax,ymax)/6
const offs = 10
const gap = 3
const w = window()
const bgc = rgb(15,40,15)

func Display()
  sub println(s)
    color 15, bgc
    at offs, self.y_out
    print s + chr(27) + "[K"
  end

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
    self.y_out = y + 10
  end

  w.setFont(sz/4,0,1,0)
  color 0, bgc: cls

  local result = {}
  result.show=@display
  result.println=@println
  return result
end

func GridClass()
  func createGrid()
    local result = {}
    result.clone=@clone
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
    return result
  end

  # Make a Deep Copy of This Object
  func clone()
    local result = self
    result.map = self.map
    return result
  end

  sub insertTile(cell, v)
    local x,y
    (y,x) = cell
    self.map[y][x] = v
  end

  sub setCellValue(cell, v)
    local x,y
    (y,x) = cell
    self.map[y][x] = v
  end

  func getAvailableCells()
    local x,y
    dim cells
    for y = 0 to self.size
      for x = 0 to self.size
        if self.map[y][x] == 0 then
          append cells, [y,x]
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
  func canInsert(cell)
    local x,y
    (y,x)=cell
    return getCellValue(x, y) == 0
  end

  # Move the Grid
  # 0 = "Up", 1 = "Down", 2 = "Left", 3 = "Right".
  func move(dir)
    local result
    select case dir
      case kUP: result = moveUD(True)
      case kDOWN: result = moveUD(False)
      case kLEFT: result = moveLR(True)
      case kRIGHT: result = moveLR(False)
      case else: throw "Invalid move:" + dir
    end select
    return result
  end

  # Move Up or Down
  func moveUD(move_up)
    local x,y,y1,y2,incr,moved,cells,cell,value
    dim cells(self.size)

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
        if self.map[y][x] != value then
          self.map[y][x] = value
          moved = True
        endif
      next y
    next x
    return moved
  end

  # move left or right
  func moveLR(move_left)
    local x,y,x1,x2,incr,moved,cells,cell,value
    dim cells(self.size)

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
        if self.map[y][x] != value then
          moved = True
          self.map[y][x] = value
        endif
      next y
    next y
    return moved
  end

  # Merge Tiles
  sub merge(byref cells)
    local num_cells = len(cells)
    if num_cells <= 1 or num_cells == self.size then
      return
    endif
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
    local i, x, y, xm, ym, checkingMoves, adjCellValue

    # Init Moves to be Checked
    checkingMoves = IFF(len(dirs)==0, vecIndex, dirs)
    for y = 0 to self.size
      for x = 0 to self.size
        # If Current Cell is Filled
        if self.map[y][x] != 0 then
          # Look Ajacent Cell Value
          for i in checkingMoves then
            [ym, xm] = directionVectors[i]
            adjCellValue = getCellValue(x + xm, y + ym)
            # If Value is the Same or Adjacent Cell is Empty
            if adjCellValue == self.map[y][x] or adjCellValue == 0 then
              return True
            endif
          next i
        # Else if Current Cell is Empty
        elif self.map[y][x] == 0 then
          return True
        endif
      next x
    next y
    return False
  end

  # Return All Available Moves
  func getAvailableMoves(dirs)
    local x, gridCopy, availableMoves
    dirs=IFF(len(dirs) == 0, vecindex, dirs)
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
    return (x < 0 or x > self.size or y < 0 or y > self.size)
  end

  func getCellValue(x, y)
    if not crossBound(x, y) then
      return self.map[y][x]
    else
      return Nil
    endif
  end

  local result = createGrid()
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
    local size=state.size

    for y = 0 to size
      for x = 0 to size
        if (state.map[y][x] != 0) then
          result += state.map[y][x] * monotonicityPattern[y][x]
          if state.map[y][x] > highest then
            highest = state.map[y][x]
          else
            numEmpty += 1
          endif
        endif
      next x
    next y
    if (state.map[0][3] != highest) then
      result += highest * 100
    endif
    if (numEmpty == 0) then
      result = 0
    elif (result != 0) then
      result = 1 / result
    endif
    return result
  end

  # whether to stop evaluating lower depths
  func isTerminal(move, depth)
    if move == Nil then return True
    if depth >= self.maxDepth then
      return True
    endif
    if ((ticks - self.startTime) > self.timeLimit) then
      self.timeout = True
      return True
    endif
    return False
  end

  func maximise(state, a, b, move, depth)
    local availableMoves, child, maxMove, maxUtility, nextMove

    if (isTerminal(move, depth)) then return [move, getUtility(state)]

    availableMoves = False
    [maxMove, maxUtility] = [Nil, ninf]
    for nextMove in [0,3,1,2]
      child = state.clone()
      if child.move(nextMove) then
        availableMoves = True
        # get the estimated attack/response move
        [_, utility] = minimise(child, a, b, nextMove, depth + 1)
        if (utility > maxUtility) then
          [maxMove, maxUtility] = [nextMove, utility]
        endif
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
      return [move, getUtility(state)]
    endif
    return [maxMove, maxUtility]
  end

  func minimise(state, a, b, move, depth)
    local child,x,y,minMove,utility,emptyPos,nextMove

    local availableCells = state.getAvailableCells()
    if (isTerminal(move, depth) or len(availableCells) == 0) then
      return [move, getUtility(state)]
    endif

    local minimisingMoves = []
    if len(availableCells) > 2 then
      # filter to cells in the top right area
      for emptyPos in availableCells:
        [x,y] = emptyPos
        if (emptyCellPattern[y][x] == 0) then
          append minimisingMoves, emptyPos
        endif
      next emptypos
    else
      # try the remaining 1 or 2 cells
      minimisingMoves = availableCells
    endif
    if len(minimisingMoves) == 0 then return [move, getUtility(state)]

    [minMove, minUtility] = [Nil, inf]
    for nextMove in minimisingMoves
      child = state.clone()
      child.insertTile(nextMove, 2)
      [_, utility] = maximise(child, a, b, move, depth + 1)
      if (utility < minUtility) then [minMove, minUtility] = [move, utility]
      if (minUtility <= a) then exit for
      if (minUtility < b) then b = minUtility
    next nextmove
    return [minMove, minUtility]
  end

  func getMove(byref grid)
    local move, utility
    self.startTime = ticks
    self.timeout = False

    [move, utility] = maximise(grid, ninf, inf, 0, 0)
    if (self.tuned == False) then
      if self.timeout == True then
        self.tuned = True
      else
        self.maxDepth += 1
      endif
    endif
    return move
  end

  local result = {}
  result.getMove=@getMove
  result.getUtility=@getUtility
  result.startTime = 0
  result.startMaxScore = 0
  result.timeLimit = 150
  result.maxDepth = 10
  result.timeout = False
  result.tuned = False
  return result
end

func ComputerAI()
  func getMove(grid)
    local cells = grid.getAvailableCells()
    if len(cells) = 0 then
      return -1
    endif
    return cells[rnd * 1000 mod len(cells)]
  end

  local result = {}
  result.getMove=@getMove
  return result
end

func Game()
  sub updateAlarm(currTime)
    if currTime - self.prevTime > timeLimit then
      logprint "timeout!"
      'self.over = True
    else
      self.prevTime = ticks
    endif
  end

  func isGameOver()
    return not self.grid.canMove([])
  end

  func getNewTileValue()
    if rnd < self.probability then
      return self.possibleNewTiles[0]
    else
      return self.possibleNewTiles[1]
    endif
  end

  sub insertRandomTile()
    local tileValue = getNewTileValue()
    local cells = self.grid.getAvailableCells()
    local cell = cells[rnd * 1000 mod len(cells)]
    self.grid.setCellValue(cell, tileValue)
  end

  sub start()
    local i, playerTurn, maxTile, prevMaxTile, gridCopy, move
    local computerAI, playerAI, displayer, moveStr

    computerAI = ComputerAI()
    playerAI   = PlayerAI()
    displayer = Display()

    for i = 1 to self.initTiles
      insertRandomTile()
    next i

    displayer.show(self.grid)

    # Player AI Goes First
    playerTurn = true
    maxTile = 0

    self.prevTime = ticks

    while not isGameOver() and not self.over
      # Copy to Ensure AI Cannot Change the Real Grid to Cheat
      gridCopy = self.grid.clone()
      'delay 200
      move = Nil

      if playerTurn then
        move = playerAI.getMove(gridCopy)
        moveStr = actionDict[move]
        displayer.println("Player's Turn:" + moveStr)
logprint moveStr
        # Validate and set Move
        if move != Nil and move >= 0 and move < 4 then
          if self.grid.canMove([move]) then
        displayer.show(self.grid)
        pause
            self.grid.move(move)
        displayer.show(self.grid)
        pause
            # Update maxTile
            maxTile = self.grid.getMaxTile()
            if maxTile < prevMaxTile then
              throw "maxTile now less"
            endif
            prevMaxTile = maxTile
          else
            displayer.println("Invalid PlayerAI Move")
            self.over = True
          endif
        else
          displayer.println("Invalid PlayerAI Move")
          self.over = True
        endif
      else
        displayer.println("Computers's Turn")
        move = computerAI.getMove(gridCopy)

        # Validate Move
        if isarray(move) and self.grid.canInsert(move) then
          self.grid.setCellValue(move, getNewTileValue())
        else
          displayer.println("Invalid Computer AI Move")
          self.over = True
        endif
      endif
      if not self.over then
        displayer.show(self.grid)
      endif
      # Exceeding the Time Allotted for Any Turn Terminates the Game
      updateAlarm(ticks)
      playerTurn = IFF(playerTurn, false, true)
    wend
    displayer.println("Game over. (max tile: " + self.grid.getMaxTile() + ")")
  end

  randomize
  local result = {}
  result.start = @start
  result.grid = GridClass()
  result.possibleNewTiles = [2, 4]
  result.probability = 0.9
  result.initTiles = 2
  result.over = False
  return result
end

g = Game()
g.start()
pause