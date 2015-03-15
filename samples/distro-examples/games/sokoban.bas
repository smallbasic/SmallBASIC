'
' $Id$
'

option predef grmode 700x500

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
  local px = bl_x + (x * bl_w) + 1
  local py = bl_y + (y * bl_h) + 1
  rect px, py, step bl_w - 1, bl_h - 1, 5 filled
  rect px, py, step bl_w - 2, bl_h - 2, 6
end

'
' draw the empty space within the grid
'
sub draw_space(x, y)
  local px = bl_x + (x * bl_w) + 1
  local py = bl_y + (y * bl_h) + 1
  rect px, py, step bl_w - 1, bl_h - 1, 8 filled
end

'
' draw the sokoban man
'
sub draw_soko(x, y, direction)
  local bx = bl_x + (x * bl_w)
  local by = bl_y + (y * bl_h)
  local dx, dy, PolyArray

  dx = bl_w / 5
  dy = bl_w / 5

  PolyArray = [[3 * dx, 1 * dy], &
               [5 * dx, 2 * dy], &
               [4 * dx, 2 * dy], &
               [4 * dx, 4 * dy], &
               [3 * dx, 3 * dy], &
               [2 * dx, 4 * dy], &
               [2 * dx, 2 * dy], &
               [1 * dx, 2 * dy], &
               [3 * dx, 1 * dy]]

  DrawPoly PolyArray, bx, by Color 9 filled
  Circle dx * 3 + bx, by + 5, 3 color 9 filled
end

'
' show the game status
'
sub game_status(byref game)
  local help, num_over

  ' count the number of blocks over the targts
  local bl_len = len(game.blocks) - 1

  num_over = 0

  for i = 0 to bl_len
    local block = game.blocks(i)
    local x = block(0)
    local y = block(1)
    if (mid(game.grid(y), x, 1) == ".") then
      num_over++
    fi
  next i

  if (num_over == bl_len + 1) then
    local y_loc = 5 + txth("T")
    for i = 0 to 10
      at 10, y_loc
      if (i mod 2 == 0) then
        ? "*** GAME OVER ***"
      else
        ? spc(20)
      fi
      delay 300
    next i
    game.game_over = true
  fi

  color 1,8
  help = "  [e]=exit, [u]=undo"
  at 10, 5: ? cat(1); "Moves: "; game.moves; " Pushes: "; game.pushes; cat(-1); help ; spc(20)
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

  erase game

  y = 0
  for row in grid
    row_len = len(row)
    for x = 1 to row_len
      ch = mid(row, x, 1)
      select case ch
      case "#"' ' border
        draw_walls x, y
      case "@" ' sokoban man
        draw_soko x, y, 1
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
  game.undo_top = 0

  dim game.undo
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
func is_target(byref game, x, y)
  local grid = game.grid
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

sub mk_button(byref f, fgc, bgc, x, y, w, h, type)
  local button
  button.foreground = fgc
  button.background = bgc
  button.x = x
  button.y = y
  button.width = w
  button.height = h
  button.type = type
  f.inputs << button
end

'
' returns a selected file name
'
func openFile
  local dir_list, file_list, frm, form_var, selected_file

  selected_file = ""

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

  sub refresh_form
    get_files
    frm.inputs(0).label = cwd
    frm.inputs(1).value = dir_list
    frm.inputs(2).value = file_list
    frm.refresh()
  end

  ' prime the initial files arrays
  get_files

  ' define the interface
  mk_button(frm, 1, 7, 35,  10, 600,  -1, "label")
  mk_button(frm, 1, 7, 35,  35, 150, 200, "listbox")
  mk_button(frm, 1, 7, -5,  35, 150, 200, "listbox")
  mk_button(frm, 1, 7, -5,  35,  -5,  -5, "button")
  mk_button(frm, 1, 7, -5,  35,  -5,  -5, "button")
  frm.inputs(0).label = cwd
  frm.inputs(1).value = dir_list
  frm.inputs(2).value = file_list
  frm.inputs(3).label = "OK"
  frm.inputs(4).label = "Cancel"
  frm = form(frm)

  ' run the interface
  while 1
    frm.doEvents()
    form_var = frm.value

    select case form_var
    case "OK"
      exit loop
    case "Cancel"
      selected_file = ""
      exit loop
    case ".."
      chdir cwd + "/.."
      refresh_form
    case else
      if (isdir(form_var)) then
        chdir form_var
        refresh_form
      else
        selected_file = form_var
      fi
    end select
  wend

  ' close the form
  frm.close()

  ' apply result
  openFile = selected_file
end

'
' main game loop
'
sub main
  local filename, games, sel_game, game, game_names, i, frm, game_file, start_dir

  ' remember the starting directory
  start_dir = cwd

  sub open_game
    games = loadGame(game_file)

    if (len(games) = 0 && game_file != "sokoban.levels") then
      ' revert back to the default game file
      ? " Levels file was empty - reverting to 'sokoban.levels' (Press any key...)"
      pause
      game_file = start_dir + "sokoban.levels"
      games = loadGame(game_file)
    fi

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
    erase frm
    mk_button(frm, 1, 7, 5,  1, 100, 20, "choice")
    mk_button(frm, 1, 7, -1, 1, -1,  20, "button")
    mk_button(frm, 1, 7, -1, 1, -1,  20, "button")
    frm.inputs(0).value = game_names
    frm.inputs(1).label = "Open"
    frm.inputs(2).label = "Play"
    frm = form(frm)
  end

  cls
  game_file = "sokoban.levels"
  open_game

  while 1
    frm.doEvents()
    select case frm.value
    case "Open"
      frm.close()
      rect 0, 0, xmax, ymax, 8 filled
      new_file = openFile
      if (len(new_file) > 0 && exist(cwd + new_file)) then
        game_file = cwd + new_file
      end if
      open_game
    case "Play"
      frm.close()
      play_game game
      open_game
    case else
      sel_game = frm.value
      init_game games(sel_game), game
    end select
  wend

end

'
' move the block covered by soko
'
sub move_block(byref game, xdir, ydir, x, y)
  local i
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
  if (is_target(game, game.soko_x, game.soko_y)) then
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
  move_erase game
  game.moves++
  draw_soko game.soko_x, game.soko_y, 1
  if is_push then
    move_block game, 0, -1, game.soko_x, game.soko_y
    game.pushes++
  fi
end

'
' move down
'
sub move_down(byref game, is_push)
  move_erase game
  game.soko_y++
  move_erase game
  game.moves++
  draw_soko game.soko_x, game.soko_y, 2
  if is_push then
    move_block game, 0, 1, game.soko_x, game.soko_y
    game.pushes++
  fi
end

'
' move left
'
sub move_left(byref game, is_push)
  move_erase game
  game.soko_x--
  move_erase game
  game.moves++
  draw_soko game.soko_x, game.soko_y, 3
  if is_push then
    move_block game, -1, 0, game.soko_x, game.soko_y
    game.pushes++
  fi
end

'
' move right
'
sub move_right(byref game, is_push)
  move_erase game
  game.soko_x++
  move_erase game
  game.moves++
  draw_soko game.soko_x, game.soko_y, 4
  if is_push then
    move_block game, 1, 0, game.soko_x, game.soko_y
    game.pushes++
  fi
end

'
' play the game
'
sub play_game(byref game)
  local k, ch

  repeat: until len(inkey) = 0

  game.game_over = false
  while game.game_over = false
    pause true: k = inkey
    if len(k) = 2 then
      ch = asc(right(k,1))
      select case ch
      case "4" 'left arrow"
        if (is_border(game.grid, game.soko_x-1, game.soko_y) = false) then
          if (get_block(game.blocks, game.soko_x-1, game.soko_y) = false) then
            move_left game, false
            undo_push game, "R", 0, 0
          elif (is_border(game.grid, game.soko_x-2, game.soko_y) = false) then
            if (get_block(game.blocks, game.soko_x-2, game.soko_y) = false) then
              move_left game, true
              undo_push game, "R", game.soko_x-1, game.soko_y
            fi
          fi
        fi
      case "5" 'right arrow"
        if (is_border(game.grid, game.soko_x+1, game.soko_y) = false) then
          if (get_block(game.blocks, game.soko_x+1, game.soko_y) = false) then
            move_right game, false
            undo_push game, "L", 0, 0
          elif (is_border(game.grid, game.soko_x+2, game.soko_y) = false) then
            if (get_block(game.blocks, game.soko_x+2, game.soko_y) = false) then
              move_right game, true
              undo_push game, "L", game.soko_x+1, game.soko_y
            fi
          fi
        fi
      case "9" 'up arrow"
        if (is_border(game.grid, game.soko_x, game.soko_y-1) = false) then
          if (get_block(game.blocks, game.soko_x, game.soko_y-1) = false) then
            move_up game, false
            undo_push game, "D", 0, 0
          elif (is_border(game.grid, game.soko_x, game.soko_y-2) = false) then
            if (get_block(game.blocks, game.soko_x, game.soko_y-2) = false) then
              move_up game, true
              undo_push game, "D", game.soko_x, game.soko_y-1
            fi
          fi
        fi
      case "10" 'down arrow"
        if (is_border(game.grid, game.soko_x, game.soko_y+1) = false) then
          if (get_block(game.blocks, game.soko_x, game.soko_y+1) = false) then
            move_down game, false
            undo_push game, "U", 0, 0
          elif (is_border(game.grid, game.soko_x, game.soko_y+2) = false) then
            if (get_block(game.blocks, game.soko_x, game.soko_y+2) = false) then
              move_down game, true
              undo_push game, "U", game.soko_x, game.soko_y+1
            fi
          fi
        fi
      end select
    else
      select case k
      case "r"
        restart game
      case "u"
        undo game
      case "e"
        game.game_over = true
      end select
    fi
    game_status game
  wend
end

'
' restart the game
'
sub restart(byref game)
end

'
' undo the last move
'
sub undo(byref game)
  local top = len(game.undo) - 1
  if (top != -1) then
    local undo_el = game.undo(top)
    local soko_dir = undo_el.soko_dir
    local block_x = undo_el.block_x
    local block_y = undo_el.block_y

    delete game.undo, top

    select case soko_dir
    case "U": move_up game, false
    case "D": move_down game, false
    case "L": move_left game, false
    case "R": move_right game, false
    end select

    if (block_x != 0 && block_y != 0) then
      ' erase the previous cell
      if (is_target(game, block_x, block_y)) then
        draw_target block_x, block_y
      else
        draw_space block_x, block_y
      fi

      ' move to the previous position
      select case soko_dir
      case "U": move_block game, 0, -1, block_x, block_y
      case "D": move_block game, 0,  1, block_x, block_y
      case "L": move_block game, -1, 0, block_x, block_y
      case "R": move_block game, 1,  0, block_x, block_y
      end select
    fi
  fi
end

'
' add an element to the undo stack
'
sub undo_push(byref game, soko_dir, block_x, block_y)
  local undo_el
  undo_el.soko_dir = soko_dir
  undo_el.block_x = block_x
  undo_el.block_y = block_y
  game.undo << undo_el
end

'
' program entry point
'
main
