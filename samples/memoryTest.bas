'app-plug-in
'menu Memory Test
REM $Id: memoryTest.bas,v 1.4 2005-09-02 06:54:52 zeeb90au Exp $
REM Copyright (c) 2005 Chris Warren-Smith 
REM Game idea taken from this site:
REM http://milov.nl/iambald/20.html

const rows = 5
const cols = 5
const gap = 3
const size=30
const xoffs = 0
const yoffs = 0
const cellSize=gap+size
const st_init = 1
const st_ready = 2
const st_active = 3
const st_done = 4

skillFactor = 5
state = st_init
bn_start = 0
bn_generate = 0
txt = ""

func createGrid(skillFactor)
  local x,y
  dim grid (1 to cols, 1 to rows)
  for x = 1 to cols
    for y = 1 to rows
      grid(x,y) = if(skillFactor=0,0, if(rnd*10 > skillFactor, 1,0))
    next y
  next x
  creategrid=grid
end

func isEqual(byref g1, byref g2)
  local x,y
  for x = 1 to cols
    for y = 1 to rows
      if (g1(x,y) != g2(x,y)) then
        isEqual=0
        exit func
      fi
    next y
  next x
  isEqual=1
end

func numMatches(byref g1, byref g2)
  local x,y,n
  n = 0
  for x = 1 to cols
    for y = 1 to rows
      if (g1(x,y) != 0 and g1(x,y) = g2(x,y)) then
        n++
      fi
    next y
  next x
  numMatches=n
end  

func numCells(byref g1)
  local x,y,n
  n = 0
  for x = 1 to cols
    for y = 1 to rows
      if (g1(x,y) != 0) then
        n++
      fi
    next y
  next x
  numCells=n
end  

sub drawGrid(byref grid)
  local x,y,x1,y1
  x1 = xoffs
  for x = 1 to cols
    y1 = yoffs
    for y = 1 to rows
      rect x1,y1,x1+size,y1+size, if(grid(x,y)=1,3,2) filled
      y1 += size+gap
    next y
    x1 += size+gap
  next x
end

sub clickCell(byref grid, cell)
  local x,y
  for y = 1 to rows
    for x = 1 to cols
      if (x = cell(0) and y = cell(1)) then
        grid(x,y) = 1
        exit func
      fi
    next y
  next x
end

sub display(s)
  txt += s
  html txt, "", 170,1
end

# Convert mouse coordinates into grid coordinates
func getCell
  pen on
  repeat
  until pen(0)
  getCell = [1+Int(Pen(1)/cellSize), 1+Int(Pen(2)/cellSize)]
  pen off
end

sub intro
  local s
  s= "<b>Objective</b><br>"
  s+="To view a pattern and reproduce it entirely from memory.<br>"
  s+="Instructions<br>"
  s+="1. Choose a difficulty-level from the menu.<br>"
  s+="2. Click on 'Generate!' to generate a random pattern.<br>"
  s+="   If you're not happy with the result, simply click it again.<br>"
  s+="3. Click on 'Start!' to hide the generated pattern, then start<br>"
  s+="   rebuilding the pattern by clicking on the<br>"
  s+="   blocks where you think they belong.<br>"
  s+="4. Click 'Check' to find out the results, and to unhide<br>"
  s+="   the original pattern so you can compare it with your guess.<br>"
  display s
end

sub showForm
  local x,y,w,h
  x = 5: y = 1: w = 70: h = 20
  button x,y,w,h, bn_opt, "Easy|Medium|Hard", "choice"
  if (state = st_ready) then
    button x,26, w,h, bn_generate, "Generate"
    button x,48, w,h, bn_start, "Start @->"
  else
    button x,26, w,h, bn_generate, "Generate @->"    
  fi
  doform 1,(cellSize*rows)+5,(cellSize*cols)+3,200
end  

sub init
  local emptyGrid
  env("TITLE=Memory Test")
  rect 0, 0, xmax, ymax, 0 filled
  randomize timer
  intro
  emptyGrid = createGrid(0)
  drawGrid emptyGrid
end

sub main
  local test,guess,emptyGrid

  'get the user skillFactor  
  state = st_init
  bn_start = 0
  emptyGrid = createGrid(0)

  showForm
  while bn_start != 1
    state = st_ready
    skillFactor = if (bn_opt = "Easy", 9, if (bn_opt="Medium", 7, 5))
    test = createGrid(skillFactor)
    drawGrid test
    showForm
  wend
  state = st_active
  
  'run the memory test
  drawGrid emptyGrid
  guess = createGrid(0)
  bn_check = 0
  bn_reset = 0
  
  doform 'enter modeless state
  button 5,(cellSize*cols)+3,70,20, bn_check, "Check @->"
  button 5,(cellSize*cols)+26,70,20, bn_reset, "Clear"
  repeat
    clickCell guess, getCell
    if (bn_reset = 1) then
      guess = createGrid(0)
      bn_reset = 0
    fi
    drawGrid guess
  until bn_check = 1
  doform 'end modeless state

  if isEqual(guess,test) then
    display, "<br><font color=green>Correct!</font>"
  else
    matches = numMatches(guess,test)
    cells = numCells(test)
    display "<br>You guessed "+matches+" of the "+cells+" cells."
  fi
  state = st_done
  drawGrid test
end

init
while 1
  main
wend
