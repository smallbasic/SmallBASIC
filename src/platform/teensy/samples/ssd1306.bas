rem
rem see: https://www.pjrc.com/teensy/td_libs_SSD1306.html
rem

import ssd1306 as display

display.init()
display.clear()
display.dim(5)
display.setCursor(0, 0)
display.invertDisplay(0)
display.print("It works !")
display.flush()

while 1
  display.print(".")
  display.flush()
  delay 1000
wend
