'
' $Id$
'
' falling blocks - tetris clone
'

' cell size in pixels
const c_w = 8
const c_h = 8

' board position
const b_x = 40
const b_y = 10

' board size
const b_w = 20
const b_h = 20

' board color
const b_col = rgb(111,199,222)

' the number of available play blocks
const num_blocks = 5

' holder for board pieces
dim blocks(0 to num_blocks - 1)

' remember used positions
dim board(0 to b_w - 1, 0 to b_h - 1)

' event loop delay factor
drop_speed = 100

' whether the game is over
game_over = false

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
sub draw_board
  local w = b_w * c_w
  local h = b_h * c_h
  rect b_x, b_y, step w, h, b_col filled
end

'
' creates a new active block
'
func next_block
  local i = int(rnd * num_blocks)
  result.id = rnd
  result.b = blocks(i)
  result.r = int(rnd * 4)
  result.col = i + 2
  result.x = b_w / 2
  result.y = 0
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
  
  draw_board
  randomize timer 
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
      if block.b(br)(x, y) = 1 && &
       (touch || by + y + 1 == b_h || board(bx + x, by + y + 1) != 0) then
       touch = true
       board(bx + x, by + y) = block.col
       if (by == 1) then
         game_over = true
         exit func
       fi
      fi
    next x
  next y

  is_bottom = touch
end

'
' handle user input while block is dropping
'
sub handle_input(block)
  local k = inkey
  if len(k) == 2 then
    k = asc(right(k,1))
    select case k
    case "4" 'left
      if (block.x > 0) then
        block.x--
      fi
    case "5" 'right
      if ((block.x + right_edge(block)) < b_w) then
        block.x++
      fi
    case "10" 'down arrow
      
    case "9" 'up arrow
      block.r = iff(block.r == 3, 0, block.r+1)
    end select
  fi
end

'
' main game loop
'
sub play_game
  local block = next_block
  show_block block

  while game_over = false
    delay drop_speed
        
    ' erase the current block before showing updated position
    hide_block block

    ' drop the block down
    block.y++

    ' check for collision
    if is_bottom(block) then
      show_block block 'show final position
      erase block
      block = next_block
    fi

    ' handle user keyboard input
    handle_input block

    ' redisplay new position
    show_block block
  wend
end

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

