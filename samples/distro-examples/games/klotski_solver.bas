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
func KlotskiState()
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
  func child(e1, e2, i)
    local state = init(self.depth + 1)
    self.children << state
    state.grid = self.grid
    state.parent = self
    if (isarray(e1)) then
      state.e1 = state.grid[i][1]
      state.grid[i][1] += e1
      state.e2 = self.e2
    else
      state.e2 = state.grid[i][1]
      state.grid[i][1] += e2
      state.e1 = self.e1
    endif
    local j
    for j = 0 to 9
      if (state.e1 == state.grid[j][1]) then throw "e1"
      if (state.e2 == state.grid[j][1]) then throw "e2"
    next j
    return state
  end

  # returns the hash value of the grid
  func hash
    local result = len(self.grid)
    local i
    for i = 0 to 9
      local p = self.grid[i][1]
      result += p[0] + p[1]
      result = result lshift 4
      result = result xor (result rshift 2)
    next i
    return result
  end

  # returns the current state as a string
  func to_str()
    local result = ""
    local i
    for i = 0 to 9
      local p = self.grid[i][1]
      result += str(p[0]) + str(p[1])
    next i
    return result
  end

  # returns the successor states
  func moves()
    local result = []
    local i
    for i = 0 to 9
      local p = self.grid[i]
      call p[0], result, p[1][0], p[1][1], i
    next p
    return result
  end

  sub m_lr(byref r, x, y, w, i)
    if (x - 1 == self.e1[0] && y == self.e1[1]) then
      r << child([-1, 0], 0, i)
      elseif (x + w == self.e1[0] && y == self.e1[1]) then
      r << child([1, 0], 0, i)
    elseif (x - 1 == self.e2[0] && y == self.e2[1]) then
      r << child(0, [-1, 0], i)
    elseif (x + w == self.e2[0] && y == self.e2[1]) then
      r << child(0, [1, 0], i)
    endif
  end
  sub m_ud(byref r, x, y, h, i)
    if (y - 1 == self.e1[1] && x == self.e1[0]) then
      r << child([0, -1], 0, i)
    elseif (y + h == self.e1[1] && x == self.e1[0]) then
      r << child([0, 1], 0, i)
    elseif (y - 1 == self.e2[1] && x == self.e2[0]) then
      r << child(0, [0, -1], i)
    elseif (y + h == self.e2[1] && x == self.e2[0]) then
      r << child(0, [0, 1], i)
    endif
  end
  ' 1
  ' 1
  sub m1(byref r, x, y, i)
    if (self.e1[0] == self.e2[0] && 1 == abs(self.e1[1] - self.e2[1])) then
      m_lr(r, x, y, 1, i)
    endif
    m_ud(r, x, y, 2, i)
  end
  ' 2,2
  ' 2,2
  sub m2(byref r, x, y, i)
    if (self.e1[1] == self.e2[1] && 1 == abs(self.e1[0] - self.e2[0])) then
      m_ud(r, x, y, 2, i)
    elif (self.e1[0] == self.e2[1] && 1 == abs(self.e1[1] - self.e2[1])) then
      m_lr(r, x, y, 2, i)
    endif
  end
  ' 3,3
  sub m3(byref r, x, y, i)
    if (self.e1[1] == self.e2[1] && 1 == abs(self.e1[0] - self.e2[0])) then
      m_ud(r, x, y, 1, i)
    endif
    m_lr(r, x, y, 2, i)
  end
  ' 4
  sub m4(byref r, x, y, i)
    m_lr(r, x, y, 1, i)
    m_ud(r, x, y, 1, i)
  end

  # [1, 2, 2, 3]
  # [1, 2, 2, 3]
  # [4, 5, 5, 6]
  # [4, 7, 8, 6]
  # [9, 0, 0, 10]
  local state = init(0)
  state.e1 = [1,4]
  state.e2 = [2,4]
  state.grid << [@m1, [0, 0]]
  state.grid << [@m2, [1, 0]]
  state.grid << [@m1, [3, 0]]
  state.grid << [@m1, [0, 2]]
  state.grid << [@m3, [1, 2]]
  state.grid << [@m1, [3, 2]]
  state.grid << [@m4, [1, 3]]
  state.grid << [@m4, [2, 3]]
  state.grid << [@m4, [0, 4]]
  state.grid << [@m4, [3, 4]]
  return state
end

func getInitialState()
  return KlotskiState()
end

func isGoal(state)
  'local goal = [0,1,2,3,4,5,6,7,8]
  'return goal == state.grid
  show_grid(state)
  showpage
  return false
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

# uses BreadthFirstSearch
sub process()
  local initialState = getInitialState()
  local fringe = Queue(initialState)
  local explored = Set()
  local state, nextState

  explored.add(initialState)
  while (not fringe.is_empty())
    state = fringe.pop()
    if isGoal(state) then
      local p = getPath(state)
      return
    endif
    for nextState in state.moves()
      if (nextState != Nil and not explored.contains(nextState)) then
        fringe.push(nextState)
        explored.add(nextState)
      endif
    next nextState
  wend
  return
end

sub show_grid(s)
  local i,p,x,y,w,h
  local bs = min(xmax,ymax)/10
  local offs = 10
  cls
  for i = 0 to 9
    p = s.grid[i]
    [x,y] = p[1]
    w = 1
    h = 1
    select case i
    case 0,2,3,5
      h = 2
    case 1
      w = 2
      h = 2
    case 4
      w = 2
    end select
    rx = offs + (x * bs)
    ry = offs + (y * bs)
    rw = rx + (w * bs)
    rh = ry + (h * bs)
    rect rx, ry, rw, rh, i + 1 filled
  next i
end

's = getInitialState()
'print s.to_str()
'print s.hash()
'show_grid(s)
'pause
'moves = s.moves()
'for m in moves
'  show_grid(m)
'  pause
'  moves2 = m.moves()
'  for m2 in moves2
'    show_grid(m2)
'    pause
'  next m2
'next m

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

