'app-plug-in
'menu Su-doku
'$Id: sudoku.bas,v 1.2 2006-02-01 05:41:54 zeeb90au Exp $

const cell_size = textwidth("9")+20
const x1 = 20
const y1 = 20
const x2 = x1+(cell_size*9)
const y2 = y1+(cell_size*9)

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
      i = x1+(x*cell_size)+10
      j = y1+(y*cell_size)+5
      at i,j
      ? grid(x,y)
    next
  next
end  

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

func getPen
  pen on
  repeat
  until pen(0)
  getPen = [Pen(1), Pen(2)]
  pen off
end

'return true if the x-y location is in the game grid
func ptInGrid(p)
  local x,y
  x = p(0)
  y = p(1)
  if (x < x1) then
    ptInGrid=0
    exit sub
  fi
  if (x > x2) then
    ptInGrid=0
    exit sub
  fi
  if (y < y1) then
    ptInGrid=0
    exit sub
  fi
  if (y > y2) then
    ptInGrid=0
    exit sub
  fi
  ptInGrid=1
end

'convert the x-y location to game grid coordinates
func ptToGrid(p)
  ptToGrid = [int((p(0)-x1)/cell_size), int((p(1)-y1)/cell_size)]
end

sub showFocus(p, show) 
  local x,y
  x = x1+(p(0)*cell_size)+3
  y = y1+(p(1)*cell_size)+3
  rect x, y, x+cell_size-5, y+cell_size-5, if(show, 7, 15)
end

sub main
  local focus,p
  
  dim focus(2)
  dim grid(9,9)

  cls  
  randomize
  focus = [0,0]

  repeat
    i = makeGrid(grid)
  until i
  showGrid grid
  showFocus focus, true
  
  while 1
    p = getPen
    if ptInGrid(p) then
      showFocus focus, false
      focus = ptToGrid(p)
      showFocus focus, true
    fi

  wend
  
end

main
