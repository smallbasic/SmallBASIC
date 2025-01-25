
import teensy

out = teensy.openDigitalOutput(13)

while 1
  print "Hello Teensy"
  print sbver
  value = false
  for i = 0 to 5
    out.write(value)
    value = !value
    delay 1000
  next

  for i = 0 to 5
    out.write(value)
    value = !value
    delay 50
  next

wend

