' Backtracking maze generator
' https://en.wikipedia.org/wiki/Maze_generation_algorithm
'
' - Starting from a random cell,
' - Selects a random neighbouring cell that has not been visited. 
' - Remove the wall between the two cells and marks the new cell as visited, 
'   and adds it to the stack to facilitate backtracking. 
' - Continues with a cell that has no unvisited neighbours being considered a dead-end. 
'   When at a dead-end it backtracks through the path until it reaches a cell with an 
'   unvisited neighbour, continuing the path generation by visiting this new, 
'   unvisited cell (creating a new junction). 
'   This process continues until every cell has been visited, backtracking all the 
'   way back to the beginning cell. We can be sure every cell is visited.
'
' model consts
const W = 40
const H = 30
dim h_walls(0 to W, 0 to H)
dim v_walls(0 to W, 0 to H)

func init_walls()
  local x,y
  for x = 0 to W
    for y = 0 to H
      v_walls(x, y) = 1
      h_walls(x, y) = 1
    next y
  next x
end

func show_maze() 
  local py, px
  local margin = 25
  local border = margin / 2
  local cellW = (xmax - margin) / W
  local cellH = (ymax - margin) / H
  local wallc, x, y
  
  wallc = rgb(80, 80, 80)
  cls
  py = border
  for y = 0 to H 
    px = border
    for x = 0 to W
      wallc += 255
      if (x < W && h_walls(x, y)) then
        rect px, py, px + cellW, py+2, wallc FILLED
      fi
      if (y < H && v_walls(x, y)) then
        rect px, py, px+2, py + cellH, wallc FILLED
      fi
      px += cellW
    next y
    py += cellH
  next x
end

func rand_cell()
  rand_cell = [rnd * 1000 % W, rnd * 1000 % H]
end

func get_unvisited(byref visited, byref current)
  local n
  dim n
  local x = current(0)
  local y = current(1)
  if (x > 0 && visited(x - 1, y) == false) then
    n << [x - 1, y]
  endif
  if (x < W - 1 && visited(x + 1, y) == false) then
    n << [x + 1, y]
  endif
  if (y > 0 && visited(x, y - 1) == false) then
    n << [x, y - 1]
  endif
  if (y < H -1 && visited(x, y + 1) == false) then
    n << [x, y + 1]
  endif
  get_unvisited = n
end
  
func generate_maze() 
  local curr_cell, next_cell, num_visited, num_cells, visited, stack, cells
  local x, y
  dim visited(W, H)
  dim stack

  curr_cell = rand_cell()
  visited(curr_cell(0), curr_cell(1)) = true
  num_visited = 1
  num_cells = W * H

  while num_visited < num_cells
    cells = get_unvisited(visited, curr_cell)
    if (len(cells) > 0) then
      ' choose randomly one of the current cell's unvisited neighbours
      next_cell = cells((rnd * 100) % len(cells))

      ' push the current cell to the stack
      stack << curr_cell
          
      ' remove the wall between the current cell and the chosen cell
      if (next_cell(0) == curr_cell(0)) then
        x = next_cell(0)
        y = max(next_cell(1), curr_cell(1))
        h_walls(x, y) = 0
      else
        x = max(next_cell(0), curr_cell(0))
        y = next_cell(1)
        v_walls(x, y) = 0
      fi

      ' make the chosen cell the current cell and mark it as visited
      curr_cell = next_cell
      visited(curr_cell(0), curr_cell(1)) = true
      num_visited++
    else if (len(stack) > 0) then
      ' pop a cell from the stack and make it the current cell
      local idx = len(stack) - 1
      curr_cell = stack(idx)
      delete stack, idx, 1
    else
      exit loop
    endif
  wend   
end

randomize
init_walls
generate_maze
show_maze
pause
