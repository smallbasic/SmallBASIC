' Digital IO - Push Button
' =====================================================
'
' This example demonstrates how to use a push button.
'
' ----------         ----------
'  TEENSY   |       | Button
'  PIN 0    |-------| Pin 1 
'  GND      |-------| Pin 2
' ----------         ---------

import teensy

const Pushbutton = teensy.openDigitalInput(0)

for ii = 1 to 20
  print ii, Pushbutton.read()
  delay(500)
next
