REM $Id: memoryTest.bas,v 1.2 2005-08-18 23:18:06 zeeb90au Exp $

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

skillFactor = 5
state = st_init
bn_start = 0
bn_generate = 0

func createGrid(skillFactor)
  local x,y
  dim grid (1 to cols, 1 to rows)
  for y = 1 to rows
    for x = 1 to cols
      grid(x,y) = if(skillFactor=0,0, if(rnd*10 > skillFactor, 1,0))
    next y
  next x
  creategrid=grid
end

func isEqual(byref g1, byref g2)
  cls
  html ""
  local x,y
  for y = 1 to rows
    for x = 1 to cols
      if (g1(x,y) != g2(x,y)) then
        isEqual=0
        ? g1
        ? g2
        pause
        exit func
      fi
    next y
  next x
  exit
  isEqual=1
end

sub drawGrid(byref grid)
  local x,y,x1,y1
  y1 = yoffs
  for y = 1 to rows
    x1 = xoffs
    for x = 1 to cols
      rect x1,y1,x1+size,y1+size, if(grid(x,y),3,2) filled
      x1 += size+gap
    next y
    y1 += size+gap
  next x
end

func clickCell(byref grid, cell)
  local x,y
  clickCell = 0
  for y = 1 to rows
    for x = 1 to cols
      if (x = cell(0) and y = cell(1)) then
        'toggle the cell
        grid(x,y) = 1 'if (grid(x,y)=1,0,1)
        clickCell = 1
      fi
    next y
  next x
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
  s+="   rebuilding the pattern in the lower field by toggling on the<br>"
  s+="   blocks where you think they belong.<br>"
  s+="4. Click outside the pattern to find out the results, and to unhide<br>"
  s+="   the original pattern so you can compare it with your guess.<br>"
  html s, "", 170,1
end

sub showForm
  local x,y,w,h
  x = 5
  y = 1
  w = 70
  h = 20
  button x,y,w,h, bn_opt, "Easy|Medium|Hard", "choice"
  button x,26, w,h, bn_generate, "Generate!"
  if (state = st_ready) then
    button x,48, w,h, bn_start, "Start"
  fi
  doform 1,(cellSize*rows)+5,(cellSize*cols)+3,200,0,0
end  

sub main
  local testgrid,guess
  
  env("TITLE=Memory Test")
  rect 0, 0, xmax, ymax, 0 filled
  randomize timer
  intro
  emptyGrid = createGrid(0)
  drawGrid emptyGrid

  'get the user skillFactor  
  repeat
    showForm
    skillFactor = if (bn_opt = "Easy", 9, if (bn_opt="Medium", 7, 5))
    testgrid = createGrid(skillFactor)
    drawGrid testgrid
    state = st_ready
  until bn_start = 1
  state = st_active

cls
html ""


  'run the memory test
  drawGrid emptyGrid
  guess = createGrid(0)
  repeat
    cellPress = clickCell(guess, getCell)
    drawGrid guess
  until cellPress = 0
  if isEqual(guess,testgrid) then
    ? "You guessed correctly"
  else
    ? "You chose poorly"
  fi
    
end

main
