'
' falling blocks - tetris clone
'

option predef grmode 300x300

' cell size in pixels
const c_w = 10
const c_h = 10

' board position
const b_x = 40
const b_y = 10

' board size
const b_w = 12
const b_h = 28

' board color
const b_col = rgb(111,199,222)

' the number of available play blocks
const num_blocks = 5

' holder for board pieces
dim blocks(0 to num_blocks - 1)

' remember used positions
dim board(0 to b_w - 1, 0 to b_h - 1)

' event loop delay factor
drop_speed = 300

' whether the game is over
game_over = false

' number of played shapes
num_shapes = 0

' number of completed rows
num_rows = 0

'
' block, rotation, x+y coordinates on the game board
'
sub draw_block(b, r, pos_x, pos_y, c)
  local x, y, px, py
  for x = 0 to 3
    for y = 0 to 3
      if b(r)(x, y) = 1 then
        px = b_x + ((pos_x + x) * c_w)
        py = b_y + ((pos_y + y) * c_h)
        rect px, py, step c_w, c_h, c filled
      fi
    next y
  next x
  SHOWPAGE
end

'
' show the given block
'
sub show_block(block)
  draw_block block.b, block.r, block.x, block.y, block.col
end

'
' erase the given block
'
sub hide_block(block)
  draw_block block.b, block.r, block.x, block.y, b_col
end

'
' returns the right edge of the given block
'
func right_edge(block)
  local x, y, r
  r = block.r
  for x = 3 to 0 step -1  
    for y = 0 to 3
      if block.b(r)(x, y) = 1 then
        right_edge = x + 1
        exit func
      fi
    next y
  next x
end

'
' create one of the four block rotations
'
func create_cell
  local x, y, result
  dim result(0 to 3, 0 to 3)

  for y = 0 to 3
    for x = 0 to 3
      read result(x, y)
    next x
  next y
  create_cell = result
end 

'
' creates 4 rotations of a 4 x 4 pattern. address like this: b(0)(1,1)
'
func create_block
  local i, result  
  dim result(0 to 3)
  
  for i = 0 to 3
    result(i) = create_cell
  next i

  create_block = result
end

'
' draw the board outline
'
sub draw_board(x1, y1)
  local w = b_w * c_w
  local h = b_h * c_h
  rect x1, y1, step w, h, b_col filled

  local x, y, px, py
  
  for y = b_h -1 to 0 step -1
    for x = 0 to b_w -1
      if (board(x, y) != 0) then
        px = x1 + (x * c_w)
        py = y1 + (y * c_h)
        rect px, py, step c_w, c_h, board(x, y) filled
      fi
    next x
  next y
end

'
' creates a new active block
'
func next_block
  local i = int(rnd * num_blocks)
  result.b = blocks(i)
  result.r = int(rnd * 4)
  result.col = i + 2
  result.x = b_w / 2
  result.y = 0
  show_block result
  num_shapes++
  next_block = result
end

'
' prepare the game board
'
sub init_game
  local i, x, y

  ' create the blocks
  for i = 0 to num_blocks - 1
    blocks(i) = create_block
  next i

  ' setup the bottom boundary holder
  for x = 0 to b_w - 1
    for y = 0 to b_h - 1
      board(x, y) = 0
    next y
  next x
  
  draw_board b_x, b_y
  randomize timer 
end

'
' handle row completion
'
func row_completed
  local x, y, n, x2, y2

  row_completed = 0
  
  for y = b_h -1 to 0 step -1
    n = 0
    for x = 0 to b_w -1
      if (board(x, y) != 0) then
        n++
      fi
    next x

    if (n == b_w) then
      ' erase the completed row
      for y2 = y to 1 step -1
        for x2 = 0 to b_w - 1
          board(x2, y2) = board(x2, y2 - 1)
        next x2
      next y2

      ' clear the top row
      for x2 = 0 to b_w - 1
        board(x2, 0) = 0
      next x2
    
      y += 1 'next row now in y
      row_completed = 1      
    fi
  next y
end

'
' whether the block can move any further down
'
func is_bottom(block)
  local x, y
  local br = block.r
  local bx = block.x
  local by = block.y
  local touch = false

  for y = 3 to 0 step -1  
    for x = 0 to 3
      if (block.b(br)(x, y) = 1 && &
          (by + y + 1 == b_h || board(bx + x, by + y + 1) != 0)) then
       touch = true
      fi
    next x
  next y

  if (touch) then
    ' check for end of game
    for y = 3 to 0 step -1  
      for x = 0 to 3
        if (block.b(br)(x, y) = 1) then
          if (by == 0 || (by < 4 && board(bx + x, by + y) != 0)) then
            game_over = true
            exit func
          fi
        fi
      next x
    next y

    ' record the resting position on the board    
    for y = 3 to 0 step -1  
      for x = 0 to 3
        if (block.b(br)(x, y) = 1) then
          board(bx + x, by + y) = block.col          
        fi
      next x
    next y
  fi

  is_bottom = touch
end

'
' update the block position by the given offsets
'
sub moveBlock(x, y)
  hide_block block
  block.x += x
  block.y += y
  show_block block
end

'
' move the block left
'
sub moveLeft
   if (block.x > 0) then
     moveBlock -1, 0
   fi
end

'
' move the block right
'
sub moveRight
  if ((block.x + right_edge(block)) < b_w) then
    moveBlock 1, 0
  fi
end

'
' rotate the block
'
sub moveRotate
  hide_block block
  block.r = iff(block.r == 3, 0, block.r+1)

  ' ensure rotation doesn't extend beyond board
  while ((block.x + right_edge(block)) > b_w)
    block.x --
  wend
  
  show_block block
end

'
' drop the block
'
sub dropBlock
  while !is_bottom(block)
    moveBlock 0, 1
  wend
end

'
' show game statistics
'
sub show_stats
  at b_x + (b_w * c_w) + 10, b_y: ? "Shapes:"; num_shapes
  at b_x + (b_w * c_w) + 10, b_y+20: ? "Rows:"; num_rows
  at b_x + (b_w * c_w) + 10, b_y+40: ? "Speed:"; drop_speed
end

'
' main game loop
'
sub play_game
  block = next_block
  show_stats

  while game_over = false
    delay drop_speed

    ' check for collision
    if is_bottom(block) then
      show_block block 'show final position
      erase block
      if row_completed then
        num_rows++
        draw_board b_x, b_y
        drop_speed -= 20
      fi
      block = next_block
      show_stats
    else
      ' drop the block down one position
      moveBlock 0, 1
    fi
  wend
end

defineKey 0xFF04, moveLeft
defineKey 0xFF05, moveRight
defineKey 0xFF09, moveRotate
defineKey asc(" "), dropBlock

init_game
play_game

'
' define the game pieces
'

'block 1 - bar
data 1,0,0,0, 1,0,0,0, 1,0,0,0, 1,0,0,0
data 1,1,1,1, 0,0,0,0, 0,0,0,0, 0,0,0,0
data 1,0,0,0, 1,0,0,0, 1,0,0,0, 1,0,0,0
data 1,1,1,1, 0,0,0,0, 0,0,0,0, 0,0,0,0

'block 2 - elbow
data 1,0,0,0, 1,0,0,0, 1,1,0,0, 0,0,0,0
data 1,1,1,0, 1,0,0,0, 0,0,0,0, 0,0,0,0
data 0,1,1,0, 0,0,1,0, 0,0,1,0, 0,0,0,0
data 0,0,0,0, 0,0,1,0, 1,1,1,0, 0,0,0,0

'block 3 - tee
data 1,1,1,0, 0,1,0,0, 0,0,0,0, 0,0,0,0
data 0,1,0,0, 1,1,0,0, 0,1,0,0, 0,0,0,0
data 0,1,0,0, 1,1,1,0, 0,0,0,0, 0,0,0,0
data 1,0,0,0, 1,1,0,0, 1,0,0,0, 0,0,0,0

'block 4 - cube
data 1,1,0,0, 1,1,0,0, 0,0,0,0, 0,0,0,0
data 1,1,0,0, 1,1,0,0, 0,0,0,0, 0,0,0,0
data 1,1,0,0, 1,1,0,0, 0,0,0,0, 0,0,0,0
data 1,1,0,0, 1,1,0,0, 0,0,0,0, 0,0,0,0

'block 5 - s-bend
data 1,1,0,0, 0,1,1,0, 0,0,0,0, 0,0,0,0
data 0,1,0,0, 1,1,0,0, 1,0,0,0, 0,0,0,0
data 1,1,0,0, 0,1,1,0, 0,0,0,0, 0,0,0,0
data 0,1,0,0, 1,1,0,0, 1,0,0,0, 0,0,0,0

