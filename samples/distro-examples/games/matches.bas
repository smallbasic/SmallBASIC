'app-plug-in
'menu Games/Matches

'Author: Patrick Lucas
'E-Mail: patlucas@btinternet.com

'The traditional game of matches where each player takes away 
'1 2 or 3 matches from a random number. The games goes on until 
'there is only a single match left. The player who has to take
'away this single match looses. When it is your turn to play, 
'press "1" to take one match, "2" to take away two matches or
'"3" to take away three matches. At the end of the game, 
'press "Y" to play again or "N" to quit. Enjoy!

randomize
const maxMatches = 30
const nMatchesPerLine = 10
const cMatchStick = rgb(255, 255, 255)
const cMatchEnd = rgb(255, 0, 0)
const cBorder = rgb(0, 0, 0)
const cBackground = rgb(0, 127, 0)

sub drawMatch(nMatch, wMatch, hMatch, nMatchesPerLine, nMatchesPerColumn, margin, flag)
  y = int((nMatch - 1) / nMatchesPerLine)
  x = nMatch - 1 - y * nMatchesPerLine
  x = x * wMatch
  y = y * hMatch
  if flag = true then
	x = x + wMatch / 2 - margin
	y = y + margin
	rect x, y, x + 2 * margin, y + hMatch - 2 * margin, cMatchStick filled
	rect x, y, x + 2 * margin, y + hMatch / 10, cMatchEnd filled
	rect x, y, x + 2 * margin, y + hMatch / 10, cBorder
	rect x, y, x + 2 * margin, y + hMatch - 2 * margin, cBorder
  else
    rect x, y, x + wMatch, y + hMatch, cBackground filled
  endif
end

sub drawScreen(maxMatches, nMatches, nMatchesPerLine, nMatchesPerColumn)
  wMatch = int(xmax / nMatchesPerLine)
  hMatch = int((ymax - 3 * textheight("M")) / nMatchesPerColumn)
  margin = int(wMatch / 10)
  for i = 1 to maxMatches
	if i > nMatches then flag = false else flag = true
	drawMatch i, wMatch, hMatch, nMatchesPerLine, nMatchesPerColumn, margin, flag
  next i
end

func getKeyPress(allowedKeys$)
  repeat
    repeat
      key$ = inkey
    until len(key$) > 0
    if key$ >= "a" and key$ <= "z" then key$ = chr$(asc(key$) - 32)
  until key$ in allowedKeys$
  getKeyPress = key$
end

func endGame(nMatches, player)
  key$ = " "
  if nMatches < 2 then
	if nMatches = 1 then
	  if player = 1 then name$ = "I" else name$ = "YOU"
	else
	  if player = 1 then name$ = "YOU" else name$ = "I"
	endif
    rect 0, ymax - 3 * textheight("M"), xmax, ymax, 15 filled
    at 0, ymax - 3 * textheight("M") : print name$ + " WIN !!!"
    at 0, ymax - 2 * textheight("M") : print "Do you want to play again?"
    key$ = getkeyPress("YN")
  endif
  endGame = key$
end

nMatches = 2 + int(rnd * (maxMatches - 2))
color 0, 15
cls
rect 0, 0, xmax, ymax - 3 * textheight("M"), cBackground filled
drawScreen maxMatches, nMatches, nMatchesPerLine, maxMatches / nMatchesPerLine
repeat
  rect 0, ymax - 3 * textheight("M"), xmax, ymax, 15 filled
  at 0, ymax - 3 * textheight("M") : print "Plese take away 1, 2 or 3 matches..."
  key$ = getKeyPress("123")
  rect 0, ymax - 3 * textheight("M"), xmax, ymax - 2 * textheight("M"), 15 filled
  at 0, ymax - 3 * textheight("M") : print "You took away " + key$ + " match(es)..."
  nMatches = nMatches - val(key$)
  if nMatches < 0 then nMatches = 0
  drawScreen maxMatches, nMatches, nMatchesPerLine, maxMatches / nMatchesPerLine
  key$ = endGame(nMatches, 0)
  if key$ = "Y" then
    nMatches = 2 + int(rnd * (maxMatches - 2))
    drawScreen maxMatches, nMatches, nMatchesPerLine, maxMatches / nMatchesPerLine
  else
    if key$ <> "N" then
      at 0, ymax - 2 * textheight("M") : print "I am thinking..."
      pause 2
      n = int(nMatches / 4) : n = nMatches - n * 4
      if n = 0 then key$ = "3"
      if n = 3 then key$ = "2"
      if n = 2 then key$ = "1"
      if n = 1 then key$ = str$(1 + int(rnd * 3))
      rect 0, ymax - 2 * textheight("M"), xmax, ymax - textheight("M"), 15 filled
      at 0, ymax - 2 * textheight("M") : print "I took away " + key$ + " match(es)..."
      nMatches = nMatches - val(key$)
      if nMatches < 0 then nMatches = 0
      drawScreen maxMatches, nMatches, nMatchesPerLine, maxMatches / nMatchesPerLine
      pause 2
      key$ = endGame(nMatches, 1)
      if key$ = "Y" then
        nMatches = 2 + int(rnd * (maxMatches - 2))
        drawScreen maxMatches, nMatches, nMatchesPerLine, maxMatches / nMatchesPerLine
      endif
	endif
  endif
until key$ = "N"
