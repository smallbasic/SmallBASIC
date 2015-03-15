'app-plug-in
'menu Memory Test
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
const lineHeight = 15 + txth("Q")

numCorrect = 0

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
  at 170, 200
  print chr(27)+"[K" + s
end

sub intro
  local h = txtw("Z") + 10
  local x = 170
  local y = 0

  at x, y: y += h: ? cat(3) + "Objective:" + cat(0)
  at x, y: y += h: ? "To view a pattern and reproduce it entirely from memory."
  at x, y: y += h: ? cat(3) + "Instructions:" + cat(0)
  at x, y: y += h: ? "1. Choose a difficulty-level from the menu."
  at x, y: y += h: ? "2. Click on 'Generate!' to generate a random pattern."
  at x, y: y += h: ? "   If you're not happy with the result, simply click it again."
  at x, y: y += h: ? "3. Click on 'Start!' to hide the generated pattern, then start"
  at x, y: y += h: ? "   rebuilding the pattern by clicking on the"
  at x, y: y += h: ? "   blocks where you think they belong."
  at x, y: y += h: ? "4. Click 'Check' to find out the results, and to unhide"
  at x, y: y += h: ? "   the original pattern so you can compare it with your guess."
end

func button(x, y, w, h)
  button.x = x
  button.y = y
  button.width = w
  button.height = h
  button.type = "button"
  button.color = "blue"
  button.backgroundColor = "green"
end

sub getOptions
  local x = 5
  local w = 10
  local h = -1
  local y = (cellSize*rows)+3
  local frm, bn_opt
  local skillFactor
  local selected

  color 11,0
  frm.inputs << button(x, y, 80, 20)
  frm.inputs << button(x, -lineHeight, -1, -5)
  frm.inputs(0).value = "Easy|Medium|Hard"
  frm.inputs(0).type = "choice"
  frm.inputs(1).label = "Generate"

  local start_ready = false
  frm = form(frm)

  ' display the form until either generate or start has been clicked
  repeat
    frm.doEvents()

    if (frm.value == "Generate") then
      selected = frm.inputs(0).selectedIndex 
      skillFactor = iff(selected==0, 9, iff(selected==1, 7, 5))
      test = createGrid(skillFactor)
      drawGrid test
      if (start_ready == false)
        frm.close()
        frm.inputs << button(x, -lineHeight, -1, -5)
        frm.inputs(2).label = "Start"
        frm = form(frm)
        start_ready = true
      fi
    fi
  until frm.value = "Start"
  frm.close()
end

sub init
  local emptyGrid
  rect 0, 0, xmax, ymax, 0 filled
  randomize timer
  intro
  emptyGrid = createGrid(0)
  drawGrid emptyGrid
end

sub play_game
  local test, guess, emptyGrid, frm, clicked

  'get the user skillFactor
  emptyGrid = createGrid(0)

  getOptions

  'run the memory test
  drawGrid emptyGrid
  guess = createGrid(0)
  drawGrid guess

  color 8,0
  local y = (cellSize*rows)+3

  local is_check_click = false
  sub check_click
    is_check_click = true
  end

  local is_clear_click = false
  sub clear_click
    is_clear_click = true
  end

  ' this shows how to create a combine pen/form loop
  frm.inputs << button(10, y, -1, -5)
  frm.inputs << button(10, -lineHeight, -1, -5)
  frm.inputs(0).label = "Check"
  frm.inputs(1).label = "Clear"

  frm.inputs(0).onclick = @check_click
  frm.inputs(1).onclick = @clear_click
  frm = form(frm)

  pen on
  while is_check_click == false
    repeat
    until pen(0) || is_check_click || is_clear_click
    if (is_clear_click) then
      guess = createGrid(0)
      drawGrid guess
      is_clear_click = false
    else if (is_check_click == false) then
      clicked = [1+Int(Pen(1)/cellSize), 1+Int(Pen(2)/cellSize)]
      clickCell guess, clicked
      drawGrid guess
    end if
  wend
  pen off
  frm.close()

  'test completed
  if isEqual(guess, test) then
    numCorrect++
    display, "Correct [" + numCorrect + "]"
  else
    matches = numMatches(guess, test)
    cells = numCells(test)
    display "You guessed " + matches + " of the " +cells + " cells."
  fi

  local x,y,x1,y1
  x1 = xoffs
  for x = 1 to cols
    y1 = yoffs
    for y = 1 to rows
      if (test(x,y) == 1) then
        rect x1,y1,x1+size,y1+size, 3 filled
      else if (guess(x,y) == 1) then
        rect x1,y1,x1+size,y1+size, 4 filled
      end if
      y1 += size+gap
    next y
    x1 += size+gap
  next x
end

init
while 1
  play_game
wend

