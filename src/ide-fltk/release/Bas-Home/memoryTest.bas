REM $Id: memoryTest.bas,v 1.1 2005-08-17 23:25:55 zeeb90au Exp $

const rows = 5
const cols = 5
const gap = 3
const size=30
const cellSize=gap+size
dim grid (1 to cols, 1 to rows)
skillFactor = 5

sub creategrid
  for y = 1 to rows
    for x = 1 to cols
      grid(x,y) = if(rnd*10 > skillFactor, 1,0)
    next y
  next x
end

sub emptyGrid
  for y = 1 to rows
    for x = 1 to cols
      grid(x,y) = 0
    next y
  next x
end

sub drawGrid(xoffs, yoffs, size, gap)
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

# Convert mouse coordinates into grid coordinates
func getCell()
  pen on
  repeat
  until pen(0)
  getCell = [1+Int(Pen(1)/cellSize), 1+Int(Pen(2)/cellSize)]
  pen off
end

sub intro
  s= "<b>Objective</b><br>"
  s+="To view a pattern and reproduce it entirely from memory.<br>"
  s+="Instructions<br>"
  s+="1. Choose a difficulty-level from the menu.<br>"
  s+="2. Click on 'Generate!' to generate a random pattern.<br>"
  s+="   If you're not happy with the result, simply click it again.<br>"
  s+="3. Click on 'Start!' to hide the generated pattern, then start<br>"
  s+="   rebuilding the pattern in the lower field by toggling on the<br>"
  s+="   blocks where you think they belong.<br>"
  s+="4. Click on 'Check!' to find out the results, and to unhide<br>"
  s+="   the original pattern so you can compare it with your guess.<br>"
  html s, "", 170,1
end

sub showForm
  button 5,1, 70, 20, bn_opt, "Easy|Medium|Hard", "choice"
  button 5,26, 70, 20, bn_generate, "Generate!"
  button 5,46, 70, 20, bn_cancel, "Cancel"
  doform 1,(cellSize*rows)+5,(cellSize*cols)+3,200,0,0
end  

sub main
  env("TITLE=Memory Test")
  rect 0, 0, xmax, ymax, 0 filled
  randomize timer
  intro
  emptyGrid
  drawGrid 0, 0, size, gap
  # getCell

  showForm
  if (bn_cancel = 1) then
    exit
  fi
  skillFactor = if (bn_opt = "Easy", 9, if (bn_opt="Medium", 7, 5))
  createGrid
  drawGrid 0, 0, size, gap
 

end

main

