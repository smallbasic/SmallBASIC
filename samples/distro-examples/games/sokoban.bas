'
' $Id$
'

const bl_w = 25
const bl_h = 25
const bl_x = 45
const bl_y = 45

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

sub draw_block(x, y, c)
  local px = bl_x + (x * bl_w)
  local py = bl_y + (y * bl_h)
  rect px, py, step bl_w, bl_h, c filled
  rect px, py, step bl_w, bl_h, c+1
end

'
'
'
sub draw_board(game)
  local row, col, row_len, x, y, ch
  
  rect 0, 0, xmax, ymax, 6 filled
  y = 0
  for row in game
    row_len = len(row)
    for x = 1 to row_len
      ch = mid(row, x, 1)
      select case ch
        case "#"' ' border
          draw_block x, y, 3
        case "@" ' sokoban man
        case "." ' block target
        case "$" ' moveable block
      end select 
    next i
    y++
  next row
end

'
' main game loop
'
sub main
  local filename = openFile
  local games = loadGame(filename)

  dim game_names
  for i in games
    game_names << i
  next i
  
  sort game_names

  button 5,  1, 100, 20, game_names, "", "choice"
  button -1, 1, -1,  20, ok_bn,      "View", "button"
  button -1, 1, -1,  20, ok_play,    "Play", "button"

  local sel_game = game_names(0)
  draw_board games(sel_game)  

  local form_var
  
  while 1
    doform form_var
    select case form_var
    case "View"
      draw_board games(sel_game)
    case "Play"
      ? "play!"
      exit loop
    case else
      sel_game = form_var
    end select
  wend
  
  doform 0
end

main
