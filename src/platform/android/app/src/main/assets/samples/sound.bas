import android

const SensorLight = 3
const ymax2 = ymax / 2
const pi2 = PI * 2
const incr1 = pi2 / (xmax / 9)
const incr2 = pi2 / (xmax / 3)
const fade = xmax / 3

phase1 = 0
phase2 = 0

android.sensor_on(SensorLight)

sub setpx(i, y, c)
  if (i < fade) then
    y *= i / fade
  elseif (xmax - i < fade)
    y *= (xmax - i) / fade
  endif
  pset i, ymax2 - y color c
end

while 1
  s = android.sensor()
  l = s.light
  if (l = 0) then l = 1

  cls
  for i = 1 to xmax
    sy = sin(phase1) * l
    phase1 = (phase1 + incr1) mdl pi2
    setpx(i, sy, 14)

    sy = sin(phase2) * (l * 1.5)
    phase2 = (phase2 + incr2) mdl pi2
    setpx(i, sy, 15)
  next
  showpage

  if l > 5 then
    sound l * 10, l, 100 BG
  else
    delay 5
  endif
wend
