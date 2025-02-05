
import teensy
import ssd1306 as display

display.init()
display.dim(1)
display.print("Hello SmallBASIC!")
display.flush()

out = teensy.openDigitalOutput(13)
print "hello"

while 1
  display.print(sbver)
  display.flush()

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

