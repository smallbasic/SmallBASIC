' Analog IO - Potentiometer
' =====================================================
'
' This example demonstrates how to read an analog
' voltage using a potentiometer.
'
' ----------
'  TEENSY   |
'  3.3V     |-------- 1 ---/\/\/\--- 3 --
'           |                 |          |
'  PIN 23   |---------------- 2          |
'           |                            |
'  GND      |----------------------------
' ----------

import teensy

const Potentiometer = teensy.openAnalogInput(23)

for ii = 1 to 50
  print ii, Potentiometer.read()
  delay(500)
next