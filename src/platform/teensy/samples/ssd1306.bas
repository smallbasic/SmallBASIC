rem
rem see: https://www.pjrc.com/teensy/td_libs_SSD1306.html
rem

import teensy
import ssd1306 as display

const screenWidth = 128
const screenHeight = 64
const out = teensy.openAnalogInput(31)

display.init()
display.clear()
display.setCursor(10, 10)
display.setRotation(90)
display.invertDisplay(0)
display.setTextSize(1)
display.print(SBVER)
display.dim(10)
display.flush()
display.drawCircle(screenWidth / 2, screenHeight / 2, 24)
display.drawRect(1, 1, screenWidth - 2, screenHeight - 2)
display.drawRoundRect(2, 2, screenWidth - 4, screenHeight - 4, 20)

display.scrollRight(1, 8)
display.flush()
delay 2000

display.scrollLeft(1, 8)
display.flush()
delay 2000

display.stopScroll()

while 1
  display.setCursor(30, 30)
  display.clear()
  display.print("Temp: " + teensy.getTemp() + "c")
  display.flush()
  delay 1000
wend
