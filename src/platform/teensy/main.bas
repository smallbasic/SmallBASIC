
import teensy
import ssd1306 as display

display.init()
delay(1000)

display.setTextSize(1)
display.setTextColor(1, 2)
display.setCursor(0, 0)
display.print("Hello SmallBASIC!")
display.flush()

print "display done!"

out = teensy.openDigitalOutput(13)

while 1
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

