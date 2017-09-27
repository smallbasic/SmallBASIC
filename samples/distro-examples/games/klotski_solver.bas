const gw = 4
const gh = 5

# A Queue where each inserted item has a priority associated with it
# [(prio, count, item), (prio, count, item) ...]
func PriorityQueue()
  #https://en.wikipedia.org/wiki/Heap_%28data_structure%29
  #http://www.comp.dit.ie/rlawlor/Alg_DS/heaps/PQs%20and%20Heaps.pdf
  #https://www.cprogramming.com/tutorial/computersciencetheory/heap.html

  # Key - a type, usually int, describes type of values in heap
  # int N - number of elements in array or heap
  # Key h[ ] - heap array of size N containing items of type Key.
  # The heap array h[] and N will be encapsulated inside the heap object.
  sub push(v, p)
    local key = {}
    key.v = v
    key.p = p
    self.heap << key
    upHeap(len(self.heap) - 1)
  end 

  # Key of node at position k may be less than that of its
  # children and may need to be moved down some levels
  # k is a position in the heap array h
  func pop()
    local key = self.heap[1]
    local i_end = len(self.heap) - 1
    self.heap[1] = self.heap[i_end]
    delete self.heap, i_end, 1
    if (i_end > 1) then
      downHeap(1)
    endif
    return key.v
  end

  func is_empty()
    return len(self.heap) == 1
  end

  # upHeap from position k. The key or node value at position k
  # may be less that that of its parent at k/2
  # k is a position in the heap array h
  sub upHeap(k)
    local key = self.heap[k]
    local pk = int(k / 2)
    while (k > 0 && key.p < self.heap[pk].p)
      self.heap[k] = self.heap[pk]
      k = pk
      pk = int(pk / 2)
    wend
    self.heap[k] = key
  end

  # Key of node at position k may be greater than that of its
  # children and may need to be moved down some levels
  # k is a position in the heap array h 
  sub downHeap(k)
    local key = self.heap[k]
    local N = len(self.heap) - 1
    local levels = int(N / 2)
    local j
    while (k <= levels) 
      # node at pos k has a left child node
      j = k * 2
      # select the highest node from left/right
      if (j < N and self.heap[j].p > self.heap[j + 1].p) then j++
      if (key.p < self.heap[j].p) then exit loop
      self.heap[k] = self.heap[j]
      k = j
    wend      
    self.heap[k] = key
  end

  local h = {}
  h.heap = [0]
  h.push = @push
  h.pop = @pop
  h.is_empty=@is_empty
  return h
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
func KlotskiState(grid)
  func init(depth)
    local state = {}
    state.grid = []
    state.depth = depth
    state.parent = 0
    state.children = []
    state.moves = @moves
    state.hash = @hash
    state.to_str = @to_str
    state.get_empty = @get_empty
    return state
  end

  # create a child state from the parent
  func child(i, mv_offs)
    local state = init(self.depth + 1)
    state.grid = self.grid
    state.grid[i] += mv_offs
    state.parent = self.grid
    self.children << state.grid
    select case i
    case 0
      state.prio = 1
    case 1
      state.prio = 2
    case 2,3,4,5
      state.prio = 3
    case else
      state.prio = 4
    end select
    state.prio += log(state.depth)
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

  func get_used()
    local i,x,y
    dim used(3,4)
    for i = 0 to 9
      [x,y] = self.grid[i]
      select case i
      case 0
        used[x,  y] = 1
        used[x+1,y] = 1
        used[x,  y+1] = 1
        used[x+1,y+1] = 1
      case 1
        used[x,  y] = 1
        used[x+1,y] = 1
      case 2,3,4,5
        used[x,  y] = 1
        used[x,  y+1] = 1
      case else
        used[x,  y] = 1
      end select
    next i
    return used
  end

  func get_empty()
    local used = get_used()
    local w = gw - 1
    local h = gh - 1
    local e1 = 0
    local e2 = 0
    for x = 0 to w
      for y = 0 to h
        if (used[x,y] == 0) then
          if (!isarray(e1)) then
            e1 = [x,y]
          elif (!isarray(e2)) then
            e2 = [x,y]
          else
            throw "e1"
          endif
        endif
      next y
    next x
    if (!isarray(e1) || !isarray(e2)) then throw "e2"
    return [e1,e2]
  end

  # returns the successor states
  func moves()
    local result = []
    local u = get_used()
    local i,x,y

    for i = 0 to 9
      [x,y] = self.grid[i]
      select case i
      case 0
        ' 2,2
        ' 2,2
        m_ud2(result, u, i, x, y, 2)
        m_lr2(result, u, i, x, y, 2)
      case 1
        ' 3,3
        m_ud2(result, u, i, x, y, 1)
        m_lr(result, u, i, x, y, 2)
      case 2,3,4,5
        ' 4
        ' 4
        m_lr2(result, u, i, x, y, 1)
        m_ud(result, u, i, x, y, 2)
      case else
        m_lr(result, u, i, x, y, 1)
        m_ud(result, u, i, x, y, 1)
      end select
    next i
    return result
  end

  sub m_lr(byref r, byref u, i, x, y, w)
    if (x > 0 && u[x-1,y] == 0) then
      r << child(i, [-1, 0])
    else if (x + w < gw && u[x+w,y] == 0) then
      r << child(i, [1, 0])
    endif
  end
  sub m_lr2(byref r, byref u, i, x, y, w)
    if (x > 0 && u[x-1,y] == 0 && u[x-1,y+1] == 0) then
      r << child(i, [-1, 0])
    else if (x + 1 < gw && u[x+w,y] == 0 && u[x+w,y+1] == 0) then
      r << child(i, [1, 0])
    endif
  end
  sub m_ud(byref r, byref u, i, x, y, h)
    if (y > 0 && u[x,y-1] == 0) then
      r << child(i, [0, -1])
    else if (y + h < gh && u[x,y+h] == 0) then
      r << child(i, [0, 1])
    endif
  end
  sub m_ud2(byref r, byref u, i, x, y, h)
    if (y > 0 && u[x,y-1] == 0 && u[x+1,y-1] == 0) then
      r << child(i, [0, -1])
    else if (y + 1 < gh && u[x,y+h] == 0 && u[x+1,y+h] == 0) then
      r << child(i, [0, 1])
    endif
  end

  local state = init(0)
  state.e1 = e1
  state.e2 = e2
  state.grid = grid
  state.prio = 1
  return state
end

func getInitialState()
  # [1, 2, 2, 3]
  # [1, 2, 2, 3]
  # [4, 5, 5, 6]
  # [4, 7, 8, 6]
  # [9, 0, 0, 10]
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
  return KlotskiState(grid)
end

func isGoal(state)
  show_grid(state)
  local x,y
  [x,y] = state.grid[0]
  return y == 2
end

# Queue => Breadth first search
# Stack => Depth first search
sub process()
  local initialState = getInitialState()
  local fringe = PriorityQueue()
  local explored = Set()
  local state, nextState,p,h

  fringe.push(initialState, 0)
  explored.add(initialState)

  while (not fringe.is_empty())
    state = fringe.pop()
    if isGoal(state) then
      'p = getPath(state)
      return
    endif
    for nextState in state.moves()
      if (not explored.contains(nextState)) then
        fringe.push(nextState, nextState.prio)
        explored.add(nextState)
      endif
    next nextState
  wend
  return
end

func getPath(state)
  local path = []
  path << state.path
  local parent = state.parent
  while parent != nil and parent.path != 0
    path << parent.path
    parent = parent.parent
  wend
  return path
end

sub show_grid(s)
  local i,x,y,w,h,e1,e2
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

  [e1,e2] = s.get_empty()
  [x,y] = e1
  show_cell(x,y,1,1,-1)
  [x,y] = e2
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

sub TestHeap
  h = Heap()
  h.push("cat", 2)
  h.push("dog", 9)
  h.push("mouse", 1)
  h.push("blah1", 10)
  h.push("blah2", 11)
  h.push("blah3", 12)
  if (h.pop() <> "mouse") then throw "e1"
  if (h.pop() <> "cat") then throw "e2"
  if (h.pop() <> "dog") then throw "e3"
  if (h.pop() <> "blah1") then throw "e4"
  if (h.pop() <> "blah2") then throw "e5"
  if (h.pop() <> "blah3") then throw "e6"
end

'testQueue
'testStack
'testSet

