import teensy

const ADDRESS = 0x23

Print "Open I2C"
const I2C = teensy.OpenI2C(1)   ' use I2C pins 16 and 17
Print "Opened"

' Power down
I2C.Write(ADDRESS, 0x00)
' Power on
I2C.Write(ADDRESS, 0x01)
delay(500)

' Send "Continuously H-resolution mode" instruction
I2C.write(ADDRESS, 0b00010000)
delay(200)

while(1)
  ' Read H-resolution measurement result
  d = I2C.read(ADDRESS, 2)
  ValueHighRes = ((d[0] lshift 8) BOR d[1]) / 1.2

  ii++  
  print ii, "High resolution: " + valueHighRes + " lx"

  delay(500)
wend
