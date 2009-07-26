'app-plug-in
'menu Memory Test
REM $Id: memoryTest.bas,v 1.4 2005/09/02 06:54:52 zeeb90au Exp $
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
  repeat
  until pen(0)
  getCell = [1+Int(Pen(1)/cellSize), 1+Int(Pen(2)/cellSize)]
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

sub getOptions
  local x = 5
  local w = 10
  local h = -1
  local y = (cellSize*rows)+3
  
  color 11,0
  button x, y, 80, 20, bn_opt, "Easy|Medium|Hard", "choice"
  button x, -3, -1, -5, bn_generate, "Generate @->"    

  local start_ready = false

  ' display the form until either generate or start has been clicked
  repeat
    bn_generate = 0
    bn_start = 0
    
    doform form_var

    if (form_var == bn_generate) then
      skillFactor = if (bn_opt = "Easy", 9, if (bn_opt="Medium", 7, 5))
      test = createGrid(skillFactor)
      drawGrid test
      if (start_ready == false)
        button x, -3, -1, -5, bn_start, "Start @->"
        start_ready = true
      fi
    fi
  until form_var = bn_start
 
  ' close the form
  doform 0
end  

sub init
  local emptyGrid
  rect 0, 0, xmax, ymax, 0 filled
  randomize timer
  intro
  emptyGrid = createGrid(0)
  drawGrid emptyGrid
end

sub main
  local test,guess,emptyGrid

  'get the user skillFactor  
  emptyGrid = createGrid(0)

  getOptions
 
  'run the memory test
  drawGrid emptyGrid
  guess = createGrid(0)
  drawGrid guess
  
  color 8,0
  local y = (cellSize*rows)+3

  ' this shows how to create a combine pen/form loop
  bn_check = 0
  bn_reset = 0
  
  button 10, y, -1, -5, bn_check, "Check @->"
  button 10, -3, -1, -5, bn_reset, "Clear"

  pen on
  repeat
    ' handle mouse actions
    clickCell guess, getCell
    drawGrid guess
   
   ' handle form actions
   doform form_var
    select case form_var
    case bn_check
      exit loop
    case bn_reset
      guess = createGrid(0)
      drawGrid guess
      bn_reset = 0
    end select

  until bn_check != ""
  pen off

  ' close the form
  doform 0

  'test completed
  if isEqual(guess, test) then
    display, "<br><font color=green>Correct!</font>"
  else
    matches = numMatches(guess,test)
    cells = numCells(test)
    display "<br>You guessed "+matches+" of the "+cells+" cells."
  fi

  drawGrid test
end

init
while 1
  main
wend

