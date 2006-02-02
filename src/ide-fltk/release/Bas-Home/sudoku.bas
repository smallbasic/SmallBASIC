'app-plug-in
'menu Su-doku
'$Id: sudoku.bas,v 1.5 2006-02-02 11:38:27 zeeb90au Exp $

const text_size = textwidth("9")
const cell_size = text_size*5
const x1 = 10
const y1 = 10
const x2 = x1+(cell_size*9)
const y2 = y1+(cell_size*9)
const ky1 = y2+10
const ky2 = ky1+cell_size

'sets the text position for the given grid reference
sub at_grid(x,y)
  at x1+(x*cell_size)+(text_size*2), y1+(y*cell_size)+text_size+4
end

' displays the game grid
sub showGrid(byref grid)
  local x,y,i,j
  cls

  'draw line segments
  i = x1
  for x = 0 to 9
    line i,y1,i,y2,0
    i += cell_size
  next x
  j = y1
  for y = 0 to 9
    line x1,j,x2,j,0
    j += cell_size
  next y
  
  'draw heavy black borders
  rect x1+1,y1+1,x2+1,y2+1,0
  i = x1+(cell_size*3)+1
  line i,y1,i,y2,0
  i = x1+(cell_size*6)+1
  line i,y1,i,y2,0
  j = y1+(cell_size*3)+1
  line x1,j,x2,j,0  
  j = y1+(cell_size*6)+1
  line x1,j,x2,j,0  

  for y = 0 to 8
    for x = 0 to 8
      if grid(x,y) > 0 then
        at_grid x,y
        ? cat(1)+grid(x,y)
      fi
    next
  next
  ? cat(-1)
end  

'display the game keypad
sub showKeypad
  local i,x

  i = x1
  for x = 0 to 8
    line i,ky1,i,ky2,0
    at i+(text_size*2),ky1+text_size+4
    ? x+1
    i += cell_size    
  next x
  rect x1,ky1,x2,ky2,0
end

'returns 1 when n is unique in the given column
func uniqueCol(byref grid, x, y, n)
  local yy

  if grid(x,y) != 9 then
    uniqueCol=0     'cell not empty
    exit sub
  fi
  
  y-- 
  for yy = 0 to y
    if grid(x,yy) = n then
      uniqueCol = 0 'same number in previous row
      exit sub
    fi
  next
  uniqueCol = 1
end

'returns 1 when n is unique within its 3x3 cell
func uniqueCell(byref grid, x, y, n)
  local x1,x2,y1,y2,xx,yy
  
  '012 345 678
  if x > 5 then
    x1 = 6
  elif x > 2 then
    x1 = 3
  else
    x1 = 0
  fi
  if y > 5 then
    y1 = 6
  elif y > 2 then
    y1 = 3
  else 
    y1 = 0
  fi

  x2 = x1+2
  y2 = y1+2

  for xx = x1 to x2
    for yy = y1 to y2
      if grid(xx,yy) = n then
        uniqueCell=0
        exit sub
      fi
    next xx
  next yy
  uniqueCell=1
end

'creates the game grid
func makeGrid(byref grid)
  local x,y,i,n
  
  for y = 0 to 8
    for x = 0 to 8
      grid(x,y) = 9
    next
  next
  
  for n = 1 to 8   'calc each number
    for y = 0 to 8 'visit each row
      i = 0        'check for invalid pattern
      while 1
        x = rnd*10000%9
        i++
        if i>40 then
          makeGrid = 0
          cls
          ? "Thinking ..."
          exit sub
        fi
        if uniqueCol(grid,x,y,n) then
          if uniqueCell(grid,x,y,n) then
            exit loop 'break from while - AND short-circuit
          fi
        fi
      wend
      grid(x,y) = n
    next
  next
  makeGrid = 1
end

'randomly hide cells
sub hideCells(byref grid)
  local x,y,n
  for y = 0 to 8
    for x = 0 to 8
      n = int(rnd*10000)%2
      if n = 0 then
        grid(x,y) = -1
      fi
    next
  next
end

'returns the current mouse position
func getPen
  pen on
  repeat
  until pen(0)
  getPen = [Pen(1), Pen(2)]
  pen off
end

'return true if the x-y location is in given rect
func ptInRect(byref p, x1,x2,y1,y2)
  local x,y
  x = p(0)
  y = p(1)
  
  if (x < x1) then
    ptInRect=0
    exit sub
  fi
  if (x > x2) then
    ptInRect=0
    exit sub
  fi
  if (y < y1) then
    ptInRect=0
    exit sub
  fi
  if (y > y2) then
    ptInRect=0
    exit sub
  fi
  ptInRect=1
end

'convert the x-y location to game grid coordinates
func ptToGrid(p)
  ptToGrid = [int((p(0)-x1)/cell_size), int((p(1)-y1)/cell_size)]
end

'convert the  x-y location to the keypad key number
func ptToKey(p)
  ptToKey = int((p(0)-x1)/cell_size)
end

'highlight the active grid cell
sub showFocus(p, show, mode)
  local x,y,i,j
  i = p(0)
  j = p(1)
  if (i<9 && j<9) then
    x = x1+(i*cell_size)
    y = y1+(j*cell_size)
    rect x, y, x+cell_size, y+cell_size, if(show, if(mode!=0,12,14), 0)
  fi
end

'set the given cell into the given cell location
sub setKey(grid, focus, key, mode)
  local x,y,xx,yy

  x = focus(0)
  y = focus(1)
  atx = x1+(x*cell_size)+(text_size*2)
  aty = y1+(y*cell_size)+text_size+5
  n = key+1
  select mode
  case 1
    atx += 10
    aty += 8
  case 2
    atx -= 10
    aty += 8
  case 3
    atx -= 10
    aty -= 8
  case 4
    atx += 10
    aty -= 8
  end select    
  
  'validate selection
  if n > 9 then
    exit sub
  fi
  
  if grid(x,y) != -1 then
    'don't change clue cell
    exit sub
  fi
  
  if uniqueCell(grid,x,y,n) = 0 then
    exit sub
  fi
  for xx = 0 to 8
    if grid(xx,y) = n then
      exit sub
    fi
  next
  for yy = 0 to 8
    if grid(x,yy) = n then
      exit sub
    fi
  next
 
  grid(i,j) = -n
  at atx,aty
  ? n
end

'show or hide the selected keypad key
sub showClick(key, show)
  local x,y
  if key < 9 then
    x = x1+(key*cell_size)
    y = ky1
    rect x+1, y+1, x+cell_size, y+cell_size, if(show, 12, 15) filled
    at x+(text_size*2),y+text_size+5
    ? key+1
  fi
end

'main game loop
sub main
  local p,focus,focus_click,key,key_click,mode
  
  dim focus(2)
  dim grid(9,9)

  cls  
  randomize
  focus = [0,0]
  key = 1
  mode = 0

  repeat
    i = makeGrid(grid)
  until i
  hideCells grid
  showGrid grid
  showFocus focus, true, 0
  showKeypad

  while 1
    p = getPen
    if ptInRect(p,x1,x2,y1,y2) then
      focus_click = ptToGrid(p)      
      if focus_click != focus then
        showFocus focus, false, 0
        showFocus focus_click, true, 0
        focus = focus_click
      elif pen(12) = 0 then
        showFocus focus_click, true, mode
        mode = if(mode > 0, 0, 1)
      fi
    elif ptInRect(p,x1,x2,ky1,ky2) then
      key_click = ptToKey(p)
      if (key_click != key) then
        showClick key, false
        showClick key_click, true
        key = key_click
        setKey grid, focus, key, mode
        if mode != 0 then
          mode++
          if mode = 5 then
            mode = 1
          fi
        fi
      fi
    fi
  wend
end

main
