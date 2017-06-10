option predef load modules
import android

const pop=35
const c1=97
const c2=122
const xm = xmax - txtw("z")*2
const ym = ymax - txth("z")*2
const vowel=["a","e","i","o","u"]
const words =["anyway", "well", "what", "hello", "hmmmm", "today"]
def mkatr = 10 + int(rnd * 140)

func mk()
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
  mk=r
end
sub create(byref d, size)
  local i
  for i = 0 to size
    d[i] = mk()
  next i
end
func upd(byref d, size, xo, yo)
  local i,cg
  local rtn=""
  for i = 0 to size
    if (xo != 0) then d[i].x += (xo * d[i].atr)
    if (yo != 0) then d[i].y += (yo * d[i].atr)
    cg = true
    if d[i].x < 0 then
      d[i].x = 0
    elif d[i].x > xm then
      d[i].x = xm
    else
      cg = false
    endif
    if d[i].y < 0 then
      d[i].y = 0
    elif d[i].y > ym then
      d[i].y = ym
    else
      cg = false
    endif
    if (cg) then d[i].atr = mkatr()
    if (d[i].y == 0 or d[i].y == ym) then rtn += d[i].ch
  next i
  upd = rtn
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
randomize timer
cls
dim d(pop)
create(d, pop)
show(d, pop)
spk = ""
android.tts_pitch(2)
android.speak("tilt the screen")
android.tts_pitch(.1)
android.speak("do it !")

while 1
  s=android.sensor
  cls
  xo = iff(s.x < -1 or s.x > 1, s.x, 0)
  yo = iff(s.y < -1 or s.y > 1, -s.y, 0)
  if (xo != 0 or yo != 0) then
    nspk = upd(d, pop, xo, yo)
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
  show(d, pop)
  showpage
  delay 50
wend


