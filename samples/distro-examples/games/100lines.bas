rem licence: https://opensource.org/licenses/GPL-3.0
option predef load modules
import android
w = window():w.setFont(2,"em",true,false)
const pop=35
const c1=97
const c2=122
const xm = xmax - txtw("z")*2
const ym = ymax - txth("z")*2
const vowel=["a","e","i","o","u"]
const words =["anyway", "well", "what", "hello", "hmmmm", "today"]
def mkatr = 10 + int(rnd * 140)
func mk_char()
  local r
  if (rnd < .2) then
    r.ch = vowel[rnd*10%5]
  else
    r.ch = chr(c1 + ((rnd*1000) % (c2-c1)))
  endif
  r.x = int(rnd * xmax)
  r.y = int(rnd * ymax)
  r.c = 1 + int(rnd * 14)
  r.atr = mkatr()
  mk_char=r
end
sub create(byref d, size)
  local i
  for i = 0 to size
    d[i] = mk_char()
  next i
end
func update(byref d, size, xo, yo)
  local i,x_chg,y_chg
  local rtn=""
  for i = 0 to size
    if (xo != 0) then d[i].x += (xo * d[i].atr)
    if (yo != 0) then d[i].y += (yo * d[i].atr)
    x_chg = y_chg = true
    if d[i].x < 0 then
      d[i].x = 0
    elif d[i].x > xm then
      d[i].x = xm
    else
      x_chg = false
    endif
    if d[i].y < 0 then
      d[i].y = 0
    elif d[i].y > ym then
      d[i].y = ym
    else
      y_chg = false
    endif
    if (x_chg or y_chg) then
      d[i].atr = mkatr()
    endif
    if ((d[i].y == 0 or d[i].y == ym) and d[i].x > 5) then rtn += d[i].ch
  next i
  update = rtn
end
sub show(byref d, size)
  local i
  for i = 0 to size
    at d[i].x, d[i].y
    color d[i].c
    print d[i].ch
  next i
end
android.sensor_on(0)
android.tts_pitch(1)
randomize timer:cls
dim d(pop)
create(d, pop)
show(d, pop)
spk = ""
android.tts_pitch(2)
android.speak("tilt the screen")
android.tts_pitch(.1)
android.speak("DO It !")
landscape=xmax>ymax
while 1
  s=android.sensor
  xo = iff(s.x < -1 or s.x > 1, s.x, 0)
  yo = iff(s.y < -1 or s.y > 1, -s.y, 0)
  if (landscape) then swap xo,yo
  if (xo != 0 or yo != 0) then
    nspk = update(d, pop, xo, yo)
    if (nspk != spk) then
      spk = nspk
      android.tts_pitch(.5 + 1 * rnd)
      if (rnd < .2) then
        android.speak(words[rnd*100%len(words)])
      else
        android.speak(spk)
      endif
    endif
  endif
  cls:show(d, pop)
  showpage:delay 50
wend
