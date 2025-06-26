import teensy
import ssd1306 as display

'
' Pico pins:
' GND
' 3.3v
' SDA 18
' SCL 19
'
' HC-06 pins:
' RX  1
' TX  0
' GND
' 3.3v
'

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
bt.send("AT+NAME=TeensyBT\r\n")
bt.send("AT+VERSION?\r\n")
bt.send("AT+PSWD?\r\n")

while 1
  if (bt.ready()) then
    rx = bt.receive(100)
    display.setCursor(0, 0)
    display.clear()
    display.print("Rx: " + rx)
    display.flush()
    bt.send(rx)
    print "received "; rx
  else
    delay 1
  endif  
wend

