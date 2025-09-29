import teensy

const ADDRESS = 0x77
const Oversampling = 0

I2C = teensy.OpenI2C(1)   ' use I2C pins 16 and 17
I2C.setClock(100000)
Init()

while(1)
  ii++
  m = StartMeasurement()
  print ii, m[0]; "°C", m[1]/100; "hPa" 
  delay(500)
wend

sub Init()
  local buffer
  ' Get chip id
  I2C.write(ADDRESS, 0xD0)
  buffer = I2C.read(ADDRESS, 1)
  if(buffer != 85)
    print "Error Chip ID does not match"
    while(1)
    wend
  endif

  ' Read calibration values
  ac1 = short(Read_2Bytes(0xAA))
  ac2 = short(Read_2Bytes(0xAC))
  ac3 = short(Read_2Bytes(0xAE))
  ac4 = Read_2Bytes(0xB0)
  ac5 = Read_2Bytes(0xB2)
  ac6 = Read_2Bytes(0xB4)
  b1  = short(Read_2Bytes(0xB6))
  b2  = short(Read_2Bytes(0xB8))
  mb  = short(Read_2Bytes(0xBA))
  mc  = short(Read_2Bytes(0xBC))
  md  = short(Read_2Bytes(0xBE))
end

func short(dat)
    if dat > 32767 then
        return dat - 65536
    else
        return dat
    endif
end

func StartMeasurement()

  local buffer, UncompTemp, Temperature, UncompPres, Pressure
  local x1, x2, x3, b3, b5, b6, b4, b7

  ' Read uncompensated temperature
  buffer = [0xF4, 0x2E]
  I2C.write(ADDRESS, buffer)
  delay(5)
  buffer[0] = Read_Byte(0xF6)
  buffer[1] = Read_Byte(0xF7)
  UncompTemp = (buffer[0] lshift 8) BOR buffer[1]
  
  ' Read uncompensated pressure
  buffer[0] = 0xF4
  buffer[1] = 0x34 + (Oversampling lshift 6)
  I2C.write(ADDRESS, buffer)
  delay(2 + (3 lshift Oversampling))
  I2C.write(ADDRESS, 0xF6)
  buffer = I2C.read(ADDRESS, 3)
  UncompPres = ((buffer[0] lshift 16) + (buffer[1] lshift 8) + buffer[0]) rshift (8 - Oversampling)

  ' Calculate true temperature
  x1 = ((UncompTemp - ac6) * ac5) rshift 15
  x2 = (mc lshift 11) / (x1 + md)
  b5 = x1 + x2
  Temperature = ((b5 + 8) rshift 4) / 10.0

  ' Calculate true pressure
  b6 = b5 - 4000
  x1 = (b2 * (b6 * b6) rshift 12) rshift 11
  x2 = (ac2 * b6) rshift 11
  x3 = x1 + x2
  b3 = (((ac1 * 4 + x3) lshift Oversampling) + 2) rshift 2
  x1 = (ac3 * b6) rshift 13
  x2 = (b1 * ((b6 * b6) rshift 12)) rshift 16
  x3 = ((x1 + x2) + 2) rshift 2
  b4 = (ac4 * (x3 + 32768)) rshift 15
  b7 = ((UncompPres - b3) * (50000 rshift Oversampling))
  if (b7 < 0x80000000)
    Pressure = (b7 lshift 1) / b4
  else
    Pressure = (b7 / b4) lshift 1
  endif
  x1 = (Pressure rshift 8) * (Pressure rshift 8)
  x1 = (x1 * 3038) rshift 16
  x2 = (-7357 * Pressure) rshift 16
  Pressure = Pressure + ((x1 + x2 + 3791) rshift 4)

  return [Temperature, Pressure]
end

func Read_2Bytes(Reg)
  local buffer
  I2C.write(ADDRESS, Reg)
  buffer = I2C.read(ADDRESS, 2)
  temp = (buffer[0] lshift 8) BOR buffer[1]
  return temp
end

func Read_Byte(Reg)
  local buffer
  I2C.write(ADDRESS, Reg)
  buffer = I2C.read(ADDRESS, 1)
  return buffer
end

