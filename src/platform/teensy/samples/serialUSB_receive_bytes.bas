import teensy

const usbSerial = teensy.openSerial()
const BuiltInLED = teensy.openDigitalOutput(13)
value = 0

teensy.SetInteractive(1)

while(1)
  if(usbSerial.ready()) then
    s = usbSerial.read(2)
    print s
    if(len(s) == 2) then
      if(s[0] == 81) then     ' if first Byte is a Q
          print "Quit program."
          stop
      endif
    endif
  else
    delay(50)
  endif

  BuiltInLED.write(1)
  delay(25)
  BuiltInLED.write(0)
  delay(25)
wend


