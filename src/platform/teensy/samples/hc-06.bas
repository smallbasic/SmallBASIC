import teensy
import ssd1306 as display

const screenWidth = 128
const screenHeight = 64
const bt = teensy.openSerial(1)

display.init()
display.clear()
display.setCursor(0, 0)
display.setTextSize(1)
display.print("BT via HC-06")
display.flush()
delay 2000

' set name and make discoverable
bt.send("AT+NAME=TeensyBT")
bt.send("AT+INQ")

while 1
  if (bt.ready()) then
    display.setCursor(30, 30)
    display.clear()
    display.print("Rx: " + bt.receive())
    display.flush()
  else
    bt.send("hello?")
    delay 1000
  endif  
wend

