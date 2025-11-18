import teensy

const usbSerial = teensy.openSerial()
const BuiltInLED = teensy.openDigitalOutput(13)
value = 0

teensy.SetInteractive(1)

while(1)
  if(usbSerial.ready()) then
    s = usbSerial.receive()
    if(len(s) > 0) then
      select case s
        case "quit"
          print "Quit program."
          stop
        case "led"
          value = !value
          print "LED: "; value
          BuiltInLED.write(value)
        case else
          print s
      end select
    endif
  else
    delay(50)
  endif
  
  BuiltInLED.write(1)
  delay(25)
  BuiltInLED.write(0)
  delay(25)
wend
