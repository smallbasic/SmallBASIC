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

  DrawPoly PolyArray Color 3 Filled  
end

'
' returns any block at the x/y location
'
func get_block(blocks, x, y)
end

'
' draw the game board
'
func init_game(grid)
  local row, col, row_len, x, y, ch, game
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
  
  init_game = game
end

'
' return whether the x/y location touches the border
'
func is_border(grid, x, y) 
  
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
  local filename, games, sel_game, game, i, form_var
  
  filename = "sokoban.levels" 'openFile
  games = loadGame(filename)

  dim game_names
  for i in games
    game_names << i
  next i
  
  sort game_names

  button 5,  1, 100, 20, game_names, "", "choice"
  button -1, 1, -1,  20, ok_bn,      "View", "button"
  button -1, 1, -1,  20, ok_play,    "Play", "button"

  sel_game = game_names(0)
  game = init_game(games(sel_game))

  while 1
    doform form_var
    select case form_var
    case "View"
      game = init_game(games(sel_game))
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
' move up
'
sub move_up(is_push)
  
end

'
' move down
'
sub move_down(is_push)
  
end

'
' move left
'
sub move_left(is_push)
  
end

'
' move right
'
sub move_right(is_push)
  
end

'
' play the game
'
sub play_game(game)
  local k, ch
  local game_over = false
  while game_over = false
    delay 250
    k = inkey
    if len(k) = 2 then
      ch = asc(right(k,1))
      select case ch
      case "4" 'left arrow"
        if (is_border(game.grid, game.soko_x-1, game.soko_y) = false) then
          if (get_block(game.blocks, game.soko_x-1, game.soko_y) = false) then
            move_left false
          elif (is_border(game.grid, game.soko_x-2, game.soko_y) = false && &
                get_block(game.blocks, game.soko_x-2, game.soko_y) = false) then
            move_left true
          fi
        fi
      case "5" 'right arrow"
        if (is_border(game.grid, game.soko_x+1, game.soko_y) = false) then
          if (get_block(game.blocks, game.soko_x+1, game.soko_y) = false) then
            move_right false
          elif (is_border(game.grid, game.soko_x+2, game.soko_y) = false && &
                get_block(game.blocks, game.soko_x+2, game.soko_y) = false) then
            move_right true
          fi
        fi
      case "9" 'up arrow"
        if (is_border(game.grid, game.soko_x, game.soko_y-1) = false) then
          if (get_block(game.blocks, game.soko_x, game.soko_y-1) = false) then
            move_up false
          elif (is_border(game.grid, game.soko_x, game.soko_y-2) = false && &
                get_block(game.blocks, game.soko_x, game.soko_y-2) = false) then
            move_up true
          fi
        fi
      case "10" 'down arrow"
        if (is_border(game.grid, game.soko_x, game.soko_y+1) = false) then
          if (get_block(game.blocks, game.soko_x, game.soko_y+1) = false) then
            move_down false
          elif (is_border(game.grid, game.soko_x, game.soko_y+2) = false && &
                get_block(game.blocks, game.soko_x, game.soko_y+2) = false) then
            move_down true
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
