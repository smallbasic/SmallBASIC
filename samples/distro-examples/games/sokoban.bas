'
' $Id$
'
 
const bl_w = 25
const bl_h = 25
const bl_x = 45
const bl_y = 45

'
' draw the game boundary
'
sub draw_walls(x, y)
  local px = bl_x + (x * bl_w)
  local py = bl_y + (y * bl_h)
  rect px, py, step bl_w, bl_h, 1 filled
  rect px, py, step bl_w, bl_h, 2
end

'
' draw the block target
'
sub draw_target(x, y)
  local px = bl_x + (x * bl_w)
  local py = bl_y + (y * bl_h)
  rect px, py, step bl_w, bl_h, 3 filled
  rect px, py, step bl_w, bl_h, 4
end

'
' draw the movable block
'
sub draw_block(x, y)
  local px = bl_x + (x * bl_w)
  local py = bl_y + (y * bl_h)
  rect px, py, step bl_w, bl_h, 5 filled
  rect px, py, step bl_w, bl_h, 6
end

'
' draw the empty space within the grid
'
sub draw_space(x, y)
  local px = bl_x + (x * bl_w)
  local py = bl_y + (y * bl_h)
  rect px, py, step bl_w, bl_h, 8 filled
end

'
' draw the sokoban man
'
sub draw_soko(x, y)
  local bx = bl_x + (x * bl_w)
  local by = bl_y + (y * bl_h)
  local dx, dy, PolyArray  

  dx = bl_w / 6
  dy = bl_w / 6

  PolyArray = [[bx + 2 * dx, by + 0 * dy], &
               [bx + 2 * dx, by + 3 * dy], &
               [bx + 1 * dx, by + 3 * dy], &
               [bx + 1 * dx, by + 5 * dy], &
               [bx + 2 * dx, by + 5 * dy], &
               [bx + 2 * dx, by + 4 * dy], &
               [bx + 3 * dx, by + 4 * dy], &
               [bx + 3 * dx, by + 5 * dy], &
               [bx + 4 * dx, by + 5 * dy], &
               [bx + 4 * dx, by + 4 * dy], &
               [bx + 5 * dx, by + 4 * dy], &
               [bx + 5 * dx, by + 5 * dy], &
               [bx + 6 * dx, by + 5 * dy], &
               [bx + 6 * dx, by + 3 * dy], &
               [bx + 5 * dx, by + 3 * dy], &
               [bx + 5 * dx, by + 0 * dy] ]

  draw_space x, y
  DrawPoly PolyArray Color 3 Filled  
end

'
' whether there is a block at the x/y location
'
func get_block(byref blocks, x, y)
  local i
  local result = false
  local bl_len = len(game.blocks) - 1
  
  for i = 0 to bl_len
    local block = game.blocks(i)
    if (block(0) = x && block(1) = y) then
      result = true
      i = bl_len      
    fi
  next i
  
  get_block = result
end

'
' draw the game board
'
sub init_game(grid, byref game)
  local row, col, row_len, x, y, ch
  dim blocks
  
  ' cls
  rect 0, 0, xmax, ymax, 8 filled

  y = 0
  for row in grid
    row_len = len(row)
    for x = 1 to row_len
      ch = mid(row, x, 1)
      select case ch
      case "#"' ' border
        draw_walls x, y
      case "@" ' sokoban man
        draw_soko x, y
        game.soko_x = x
        game.soko_y = y
      case "." ' block target
        draw_target x, y
      case "$" ' moveable block
        draw_block x, y
        blocks << [x,y]
      end select 
    next i
    y++
  next row

  game.blocks = blocks
  game.grid = grid
end

'
' return whether the x/y location touches the border
'
func is_border(byref grid, x, y) 
  is_border = mid(grid(y), x, 1) == "#"
end

'
' whether soko is current over a drop target
'
func is_target(byref game)
  local grid = game.grid
  local x = game.soko_x
  local y = game.soko_y  
  is_target = mid(grid(y), x, 1) == "."
end

'
' loads any sokoban games found in filename
' for more games see: http://www.sourcecode.se/sokoban/levels.php
'
func loadGame(filename)
  tload filename, buffer
  local blockFound = false
  local gameName = ""
  local buffLen = len(buffer) - 1
  local nextLine, i, firstChar, games, nextGame

  dim nextGame
  dim games
  
  for i = 0 to buffLen
    nextLine = buffer(i)
    firstChar = left(trim(nextLine), 1)
    if (firstChar == "#") then
      ' now visiting a game block
      blockFound = true
    else if firstChar == ";" then
      ' now visiting a comment
      if blockFound then
        ' store the previously visited game block
        games(gameName) = nextGame
      fi
      blockFound = false
      if (len(nextLine) > 2) then
        ' comment names the next game
        gameName = trim(mid(nextLine, 2))
        erase nextGame
      fi
    fi
    if (blockFound) then
      ' append to the next game block
      nextGame << nextLine
    fi
  next i

  if blockFound then
    ' store the last game block
    games(gameName) = nextGame
  fi
  
  loadGame = games
end

'
' returns a selected file name
'
func openFile
  local dir_list, file_list, form_var, selected_file
  
  '
  ' get the file list using the current directory
  '
  sub get_files
    local list, f

    ' empty the arrays
    erase dir_list
    erase file_list
    
    ' re-create as arrays
    dim dir_list
    dim file_list
  
    if (exist(cwd + "/..")) then
      dir_list << ".."
    fi

    list = files("*")
    for f in list
      if (!isdir(f)) then
        file_list << f
      else
        dir_list << f
      fi
    next f

    sort dir_list
    sort file_list
    
    ' sync with first pre-selected file list item
    selected_file = iff(empty(file_list), "", file_list(0))
  end
  
  ' prime the initial files arrays
  get_files
  
  ' define the interface
  color 1,7
  button  35, 10, 345, -1, cwd, "", "label"
  button  35, 35, 150, 200, dir_list, "", "listbox"
  button -5, 35, 150, 200, file_list, "", "listbox"
  button -5, 35, -5,  -5,  up_button, "OK"

  ' run the interface
  while 1
    doform form_var
    select case form_var
    case "OK"
      exit loop
    case ".."
      chdir cwd + "/.."
      get_files
    case else
      if (isdir(form_var)) then
        chdir form_var
        get_files
      else
        selected_file = form_var
      fi
    end select
  wend

  ' close the form
  doform 0
  
  ' apply result
  openFile = cwd + selected_file
end

'
' main game loop
'
sub main
  local filename, games, sel_game, game, game_names, i, form_var
  
  sub open_game(filename)
    games = loadGame(filename)

    if (len(games) > 0) then
      ' get the sorted list of game names
      dim game_names
      for i in games
        game_names << i
      next i
      sort game_names
      
      sel_game = game_names(0)
      init_game games(sel_game), game
    fi
    
    ' build the gui
    button 5,  1, 100, 20, game_names, "", "choice"
    button -1, 1, -1,  20, ok_bn,      "View", "button"    
    button -1, 1, -1,  20, ok_open,    "Open", "button"  
    button -1, 1, -1,  20, ok_play,    "Play", "button"
  end 

  cls
  open_game "sokoban.levels"

  while 1
    doform form_var
    select case form_var
    case "Open"
      doform 0
      rect 0, 0, xmax, ymax, 8 filled      
      open_game openFile
    case "View"
      init_game games(sel_game), game
    case "Play"
      exit loop  
    case else
      sel_game = form_var
    end select
  wend

  doform 0
  
  play_game game
end

'
' move the block covered by soko
'
sub move_push(byref game, xdir, ydir)
  local i
  local x = game.soko_x
  local y = game.soko_y
  local bl_len = len(game.blocks) - 1
  
  for i = 0 to bl_len
    local block = game.blocks(i)
    if (block(0) = x && block(1) = y) then
      block(0) = block(0) + xdir
      block(1) = block(1) + ydir
      draw_block block(0), block(1)
      game.blocks(i) = block
      i = bl_len
    fi
  next i
end

'
' erase soko from the current position before a move
'
sub move_erase(byref game)
  if (is_target(game)) then
    ' redraw the now empty target
    draw_target game.soko_x, game.soko_y
  else
    draw_space game.soko_x, game.soko_y
  fi
end

'
' move up
'
sub move_up(byref game, is_push)
  move_erase game
  game.soko_y--
  draw_soko game.soko_x, game.soko_y
  if is_push then
    move_push game, 0, -1
  fi
end

'
' move down
'
sub move_down(byref game, is_push)
  move_erase game
  game.soko_y++
  draw_soko game.soko_x, game.soko_y
  if is_push then
    move_push game, 0, 1
  fi
end

'
' move left
'
sub move_left(byref game, is_push)
  move_erase game
  game.soko_x--
  draw_soko game.soko_x, game.soko_y
  if is_push then
    move_push game, -1, 0
  fi
end

'
' move right
'
sub move_right(byref game, is_push)
  move_erase game
  game.soko_x++
  draw_soko game.soko_x, game.soko_y
  if is_push then
    move_push game, 1, 0
  fi
end

'
' play the game
'
sub play_game(byref game)
  local k, ch
  local game_over = false
  while game_over = false
    delay 150
    k = inkey
    if len(k) = 2 then
      ch = asc(right(k,1))
      select case ch
      case "4" 'left arrow"
        if (is_border(game.grid, game.soko_x-1, game.soko_y) = false) then
          if (get_block(game.blocks, game.soko_x-1, game.soko_y) = false) then
            move_left game, false
          else if (is_border(game.grid, game.soko_x-2, game.soko_y) = false) then
            if (get_block(game.blocks, game.soko_x-2, game.soko_y) = false) then
              move_left game, true
            fi
          fi
        fi
      case "5" 'right arrow"
        if (is_border(game.grid, game.soko_x+1, game.soko_y) = false) then
          if (get_block(game.blocks, game.soko_x+1, game.soko_y) = false) then
            move_right game, false
          elif (is_border(game.grid, game.soko_x+2, game.soko_y) = false) then
            if (get_block(game.blocks, game.soko_x+2, game.soko_y) = false) then
              move_right game, true
            fi
          fi
        fi
      case "9" 'up arrow"
        if (is_border(game.grid, game.soko_x, game.soko_y-1) = false) then
          if (get_block(game.blocks, game.soko_x, game.soko_y-1) = false) then
            move_up game, false
          elif (is_border(game.grid, game.soko_x, game.soko_y-2) = false) then
            if (get_block(game.blocks, game.soko_x, game.soko_y-2) = false) then
              move_up game, true
            fi
          fi
        fi
      case "10" 'down arrow"
        if (is_border(game.grid, game.soko_x, game.soko_y+1) = false) then
          if (get_block(game.blocks, game.soko_x, game.soko_y+1) = false) then
            move_down game, false
          elif (is_border(game.grid, game.soko_x, game.soko_y+2) = false) then
            if (get_block(game.blocks, game.soko_x, game.soko_y+2) = false) then
              move_down game, true
            fi
          fi
        fi
      end select
    fi
  wend
end

'
' program entry point
'
main
