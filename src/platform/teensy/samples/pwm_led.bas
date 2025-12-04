' 8 bit PWM with built-in LED

import teensy

const BuiltInLED = teensy.openAnalogOutput(13)

while(1)
  brightness++
  if(brightness > 255) then brightness = 0
  print brightness
  BuiltInLED.write(brightness)
  delay(10)
wend