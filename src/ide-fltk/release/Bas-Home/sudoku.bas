'app-plug-in
'menu Su-doku
'$Id: sudoku.bas,v 1.1 2006-02-01 04:29:04 zeeb90au Exp $

sub showGrid(byref grid)
  local x,y,w,i,j,x1,x2,y1,y2
  cls

  w = textwidth("9")+20
  x1 = 20
  y1 = 20
  x2 = x1+(w*9)
  y2 = y1+(w*9)

  'draw line segments
  i = x1
  for x = 0 to 9
    line i,y1,i,y2,0
    i += w
  next x
  j = y1
  for y = 0 to 9
    line x1,j,x2,j,0
    j += w
  next y
  
  'draw heavy black borders
  rect x1+1,y1+1,x2-1,y2-1,0
  i = x1+(w*3)+1
  line i,y1,i,y2,0
  i = x1+(w*6)+1
  line i,y1,i,y2,0
  j = y1+(w*3)+1
  line x1,j,x2,j,0  
  j = y1+(w*6)+1
  line x1,j,x2,j,0  
  
  for y = 0 to 8
    for x = 0 to 8
      i = x1+(x*w)+10
      j = y1+(y*w)+5
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

sub main
  cls  
  randomize
  dim grid(9,9)
  repeat
    i = makeGrid(grid)
  until i
  showGrid grid
end

main
