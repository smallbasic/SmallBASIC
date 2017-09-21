const t_lr = 1
const t_ud = 2

# A container with a first-in-first-out (FIFO) queuing policy.
# push(3,2,1) -> pop(3)
# 1
# 2           1
# 3           2
func Queue(initial)
  func contains(item)
    return item in self.items > 0
  end
  func is_empty()
    return len(self.items) == 0
  end
  sub push(item)
    self.items << item
  end
  func pop()
    local result = self.items[0]
    delete self.items, 0, 1
    return result
  end
  func size()
    return len(self.items)
  end

  local result = {}
  result.items = []
  result.contains=@contains
  result.is_empty=@is_empty
  result.push=@push
  result.pop=@pop
  result.size=@size
  result.push(initial)
  return result
end

# A container with a last-in-first-out (LIFO) queuing policy.
# push(3,2,1) -> pop(1)
# 1
# 2           2
# 3           3
func Stack(initial)
  func contains(item)
    return item in self.items > 0
  end
  func is_empty()
    return len(self.items) == 0
  end
  sub push(item)
    self.items << item
  end
  func pop()
    local idx = len(self.items) - 1
    local result = self.items[idx]
    delete self.items, idx, 1
    return result
  end
  func size(self)
    return len(self.items)
  end

  local result = {}
  result.items = []
  result.contains=@contains
  result.is_empty=@is_empty
  result.push=@push
  result.pop=@pop
  result.size=@size
  result.push(initial)
  return result
end

func Set
  sub add(item)
    local v = item.to_str()
    self.items[v] = 1
  end
  func contains(item)
    local v = item.to_str()
    return self.items[v] == 1
  end
  func size()
    return len(self.items)
  end
  local result = {}
  result.items = {}
  result.add = @add
  result.size = @size
  result.contains= @contains
  return result
end

# Represents the state of the game at a given turn, including
# parent and child nodes.
func KlotskiState(grid, e1, e2)
  func init(depth)
    local state = {}
    state.grid = []
    state.e1 = [0, 0]
    state.e2 = [0, 0]
    state.depth = 0
    state.parent = 0
    state.children = []
    state.moves = @moves
    state.hash = @hash
    state.to_str = @to_str
    return state
  end

  # create a child state from the parent
  func child(i, e1, e2, t)
    local state = init(self.depth + 1)
    self.children << state
    state.grid = self.grid
    state.parent.grid = self.grid
    state.parent.e1 = self.e1
    state.parent.e2 = self.e2
    if (isarray(e1)) then
      state.e1 = state.grid[i]
      state.grid[i] += e1
      state.e2 = self.e2
      if (e1[1] < 0 and i in [0,2,3,4,5]) then
        state.e1[1]++ # double height moving up
      elif (e1[0] < 0 and i in [0,1]) then
        state.e1[0]++ # double width moving left
      endif
    else
      state.e2 = state.grid[i]
      state.grid[i] += e2
      state.e1 = self.e1
      if (e2[1] < 0 and i in [0,2,3,4,5]) then
        state.e2[1]++  # double height moving up
      elif (e2[0] < 0 and i in [0,1]) then
        state.e2[0]++  # double width moving left
      endif
    endif
    if (t == t_lr) then
      # vertical double left-right
      state.e2 = [state.e1[0], state.e1[1] + 1]
    elseif (t == t_ud) then
      # horizontal double up-down
      state.e2 = [state.e1[0] + 1, state.e1[1]]
    endif
    if (state.e1[0] == state.e2[0] and state.e1[1] > state.e2[1]) then
      # vertical pair - ensure e1 < e2
      swap state.e1, state.e2
    elseif (state.e1[1] == state.e2[1] and state.e1[0] > state.e2[0]) then
      # horizonal pair - ensure e1 < e2
      swap state.e1, state.e2
    endif

    # validate moves
    local j
    for j = 0 to 9
      if (state.grid[j]==state.e1) then
        show_grid(self)
        pause
        show_grid(state)
        pause
        throw "e1"
      endif
      if (state.grid[j]==state.e2) then
        show_grid(self)
        pause
        show_grid(state)
        pause
        throw "e2"
      endif
    next j

    if (i== 0) then
      'logprint self.to_str()
      'logprint "self : " + str(self.e1) + " " + str(self.e2)
      'logprint "empty: " + str(state.e1) + " " + str(state.e2)
      'logprint "e1="+str(e1)
      'logprint "e2="+str(e2)
      'logprint state.grid[i]
      'logprint ""
      'show_grid(self)
      'pause
      'show_grid(state)
      'pause
    endif
    return state
  end

  # returns the hash value of the grid
  func hash
    local result = len(self.grid)
    local i,x,y
    for i = 0 to 9
      [x,y] = self.grid[i]
      result += x+y
      result = result lshift 4
      result = result xor (result rshift 2)
    next i
    return result
  end

  # returns the current state as a string
  func to_str()
    local result = ""
    local i,x,y
    for i = 0 to 9
      [x,y] = self.grid[i]
      result += str(x) + str(y)
    next i
    return result
  end

  # returns the successor states
  func moves()
    local result = []
    local i,x,y
    for i = 0 to 9
      [x,y] = self.grid[i]
      select case i
      case 0
        m0(result, i, x, y)
      case 1
        m1(result, i, x, y)
      case 2,3,4,5
        m2(result, i, x, y)
      case else
        m_lr(result, i, x, y, 1, 0)
        m_ud(result, i, x, y, 1, 0)
      end select
    next i
    return result
  end
  sub m_lr(byref r, i, x, y, w, t)
    if (x - 1 == self.e1[0] && y == self.e1[1]) then
      r << child(i, [-1, 0], 0, t)
    elseif (x + w == self.e1[0] && y == self.e1[1]) then
      r << child(i, [1, 0], 0, t)
    endif
    if (x - 1 == self.e2[0] && y == self.e2[1]) then
      r << child(i, 0, [-1, 0], t)
    elseif (x + w == self.e2[0] && y == self.e2[1]) then
      r << child(i, 0, [1, 0], t)
    endif
  end
  sub m_ud(byref r, i, x, y, h, t)
    if (y - 1 == self.e1[1] && x == self.e1[0]) then
      r << child(i, [0, -1], 0, t)
    elseif (y + h == self.e1[1] && x == self.e1[0]) then
      r << child(i, [0, 1], 0, t)
    endif
    if (y - 1 == self.e2[1] && x == self.e2[0]) then
      r << child(i, 0, [0, -1], t)
    elseif (y + h == self.e2[1] && x == self.e2[0]) then
      r << child(i, 0, [0, 1], t)
    endif
  end
  ' 2,2
  ' 2,2
  sub m0(byref r, i, x, y)
    if (self.e1[1] == self.e2[1] && x == self.e1[0] && x + 1 == self.e2[0]) then
      m_ud(r, i, x, y, 2, t_ud)
    endif
    if (self.e1[0] == self.e2[0] && y == self.e1[1] && y + 1 == self.e2[1]) then
      m_lr(r, i, x, y, 2, t_lr)
    endif
  end
  ' 3,3
  sub m1(byref r, i, x, y)
    if (self.e1[1] == self.e2[1] && x == self.e1[0] && x + 1 == self.e2[0]) then
      m_ud(r, i, x, y, 1, t_ud)
    endif
    m_lr(r, i, x, y, 2, 0)
  end
  ' 1
  ' 1
  sub m2(byref r, i, x, y)
    if (self.e1[0] == self.e2[0] && y == self.e1[1] && y + 1 == self.e2[1]) then
      m_lr(r, i, x, y, 1, t_lr)
    endif
    m_ud(r, i, x, y, 2, 0)
  end

  local state = init(0)
  state.e1 = e1
  state.e2 = e2
  state.grid = grid
  return state
end

func getInitialState()
  # [1, 2, 2, 3]
  # [1, 2, 2, 3]
  # [4, 5, 5, 6]
  # [4, 7, 8, 6]
  # [9, 0, 0, 10]
  local e1 = [1,4]
  local e2 = [2,4]
  local grid = []
  grid << [1,0] '2x2
  grid << [1,2] '2x1
  grid << [0,0] '1x2
  grid << [3,0] '1x2
  grid << [0,2] '1x2
  grid << [3,2] '1x2
  grid << [1,3] '1x1
  grid << [2,3] '1x1
  grid << [0,4] '1x1
  grid << [3,4] '1x1
  return KlotskiState(grid, e1, e2)
end

func isGoal(state)
  'local goal = [0,1,2,3,4,5,6,7,8]
  'return goal == state.grid
  show_grid(state)
'  pause
  local x,y
  [x,y] = state.grid[0]
  return y == 2
end

func getPath(state)
  local path = []
  'show(state.grid)
  print state.path
  path << state.path
  local parent = state.parent
  while parent != nil and parent.path != 0
    'show(parent.grid)
    '    print parent.path
    path << parent.path
    parent = parent.parent
  wend
  'show(parent.grid)
  'print parent.path
  return path
end

# Queue => Breadth first search
# Stack => Depth first search
sub process()
  local initialState = getInitialState()
  local fringe = Stack(initialState)
  local explored = Set()
  local state, nextState,p

  explored.add(initialState)
  while (not fringe.is_empty())
    state = fringe.pop()
    if isGoal(state) then
      p = getPath(state)
      return
    endif
    for nextState in state.moves()
      if (not explored.contains(nextState)) then
        fringe.push(nextState)
        explored.add(nextState)
      endif
    next nextState
  wend
  return
end

sub show_grid(s)
  local i,x,y,w,h
  local bs = min(xmax,ymax)/7
  local xoffs = 60
  local yoffs = 5

  sub show_cell(x,y,w,h,i)
    local rx = xoffs + (x * bs)
    local ry = yoffs + (y * bs)
    local rw = rx + (w * bs)
    local rh = ry + (h * bs)
    if (i==-1) then
      rect rx+1, ry+1, rw-2, rh-2,1
      at rx+bs/3,ry+bs/3: print [x,y]
    else
      rect rx, ry, rw, rh, i + 1 filled
      at rx,ry: print [x,y]
    endif
  end

  cls
  for i = 0 to 9
    [x,y] = s.grid[i]
    w = 1
    h = 1
    select case i
    case 0
      w = 2
      h = 2
    case 1
      w = 2
    case 2,3,4,5
      h = 2
    end select
    show_cell(x,y,w,h,i)
  next i
  [x,y] = s.e1
  show_cell(x,y,1,1,-1)
  [x,y] = s.e2
  show_cell(x,y,1,1,-1)
  showpage
end

process()

rem -------- TESTS --------
sub testSet
  s = Set()
  s.add({a:2,b:3})
  s.add({a:2,b:3})
  s.add({a:2,b:4})
  s.add({a:2,b:3})
  s.add({a:2,b:4})
  print s
  print s.contains({a:2,b:5})
  pause
end
sub testStack()
  s = Stack("blah")
  s.push("1")
  s.push("2")
  s.push("3")
  print s.pop()
  print s.pop()
  print s.pop()
  print s.pop()
end
func testQueue()
  q = Queue("blah")
  q.push("1")
  q.push("2")
  q.push("3")
  print q.pop()
  print q.pop()
  print q.pop()
  print q.pop()
  print q.is_empty()
end
'testQueue
'testStack
'testSet

