const bgc = rgb(0, 0, 0)
const fgc = 7

sub show_ch(x, y, c, ch)
  at x, y
  color c
  print ch;
end

sub text1(message)
  local ch_w = txtw("a")
  local ch_h = txth("a")
  local y_end = (ymax - ch_h) / 2
  local x = (xmax - txtw(message)) / 2
  local i, ch

  sub show_str(ch)
    local y = 0
    while y < y_end
      show_ch(x, y, 1 + int(rnd * 14), ch)
      delay 1
      show_ch(x, y, bgc, ch)
      y += ch_h
    wend
    show_ch(x, y_end, fgc, ch)
  end

  cls
  for i = 1 to len(message)
    ch = mid(message, i, 1)
    if (ch != " ") then
      show_str(ch)
    endif
    x += ch_w
  next i
end

sub text2(message)
  local ch_w = txtw("a")
  local ch_h = txth("a")
  local y = (ymax - ch_h) / 2
  local i, ch, j, x

  cls
  for j = 0 to 4
    x = (xmax - txtw(message)) / 2
    for i = 1 to len(message)
      ch = mid(message, i, 1)
      if (ch != " ") then
        show_ch(x, y, 1 + int(rnd * 14), ch)
        delay 10
      endif
      x += ch_w
    next i

    x = ((xmax - txtw(message)) / 2) + ch_w * (len(message) - 1)
    for i = len(message) to 0 step -1
      ch = mid(message, i, 1)
      if (ch != " ") then
        show_ch(x, y, fgc, ch)
        delay 10
      endif
      x -= ch_w
    next i
  next j
end

sub text3(message)
  local ch_w = txtw("a")
  local ch_h = txth("a")
  local i, j, ch, x, y, ha
  local x1 = (xmax - txtw(message)) / 2
  local y1 = (ymax - ch_h) / 2
  local done = {}
  cls
  repeat
    i = 1 + (rnd * 1000 % len(message))
    ch = mid(message, i, 1)
    if (ch != " " && done[i] == 0) then
      ha = ch_h * iff(rnd > 0.5, 1, 2)
      x = x1 + i * ch_w - ch_w
      y = iff(rnd > 0.5, y1 + ha, y1 - ha)
      show_ch(x, y, fgc, ch)
      delay 50
      show_ch(x, y, bgc, ch)
      delay 50
      show_ch(x, y1, fgc, ch)
    endif
    done[i] = 1
  until (len(done) == len(message))
end

sub text4(message)
  local ch_w = txtw("a")
  local ch_h = txth("a")
  local y_end = (ymax - ch_h) / 2
  local x = (xmax - txtw(message)) / 2
  local i, ch

  sub show_str(ch)
    local y = ymax - ch_h
    while y > y_end
      show_ch(x, y, 1 + int(rnd * 14), ch)
      delay 1
      show_ch(x, y, bgc, ch)
      y -= ch_w
    wend
    show_ch(x, y_end, fgc, ch)
  end

  cls
  for i = 1 to len(message)
    ch = mid(message, i, 1)
    if (ch != " ") then
      show_str(ch)
    endif
    x += ch_w
  next i
end

w = window()
w.setFont(3, "em", false, false)
color fgc, bgc

while 1
  text1("Welcome"):    delay 100
  text4("Welcome"):    delay 100
  text1("to"):         delay 100
  text1("SmallBASIC"): delay 100
  text2("SmallBASIC"): delay 100
  text1("Enjoy!"):     delay 500
  text3("Enjoy!"):     delay 500
  text3("SmallBASIC"): delay 100
wend
