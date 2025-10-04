' 12 bit PWM with built-in LED
' For frequency settings see https://www.pjrc.com/teensy/td_pulse.html

import teensy

const BuiltInLED = teensy.openAnalogOutput(13)

BuiltInLED.Frequency(36621.09)
BuiltInLED.Resolution(12)

while(1)
  brightness++
  if(brightness > 4095) then brightness = 0
  print brightness
  BuiltInLED.write(brightness)
  delay(1)
wend