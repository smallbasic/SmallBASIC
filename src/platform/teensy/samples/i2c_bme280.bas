rem
rem https://piico.dev/p2
rem https://www.mouser.com/datasheet/2/783/BST-BME280-DS002-1509607.pdf
rem
rem based on:https://raw.githubusercontent.com/CoreElectronics/CE-PiicoDev-BME280-MicroPython-Module/main/PiicoDev_BME280.py
rem Original header:
rem A MicroPython class for the Core Electronics PiicoDev Atmospheric Sensor BME280
rem Ported by Michael Ruppe at Core Electronics
rem MAR 2021
rem Original repo https://bit.ly/2yJwysL
rem
rem wiring:
rem SDA1 -> pin 17
rem CLK1 -> pin 16
rem

import teensy
import ssd1306 as display

const screenWidth = 128
const screenHeight = 64
const address = 0x77
const t_mode = 2
const p_mode = 5
const h_mode = 1
const iir = 1

I2C = teensy.OpenI2C(1, 17, 16)
sdata = bme280_init(t_mode, p_mode, h_mode, iir)

display.init()
display.clear()
display.setCursor(0, 0)
display.setTextSize(1)

while 1
  [temp, pres, humi] = values(sdata)
  display.clear()
  display.setCursor(0, 0):  display.print("Temp: " + temp)
  display.setCursor(0, 10): display.print("Pres: " + pres)
  display.setCursor(0, 20): display.print("Humi: " + humi)
  display.setCursor(0, 30): display.print("Cntr: " + j)
  display.setCursor(0, 40): display.print("Free: " + str(teensy.free()))
  display.flush()
  delay 1
  j++
wend

rem the code below could be moved to a unit
func short(dat)
  if dat > 32767 then
    return dat - 65536
  else
    return dat
  endif
end

func read8(register)
  I2C.write(ADDRESS, register)
  return I2C.read(ADDRESS, 1)
end

func read16(register)
  I2C.write(ADDRESS, register)
  ' local b1 = I2C.read(ADDRESS, 1)
  ' local b2 = I2C.read(ADDRESS, 1)
  ' return (b2 lshift 8) BOR b1
  local buffer = I2C.read(ADDRESS, 2)
  return (buffer[1] lshift 8) BOR buffer[0]
end

sub write8(register, dat)
   I2C.write(ADDRESS, register)
   I2C.write(ADDRESS, dat)
end

func bme280_init(t_mode, p_mode, h_mode, iir)
  local result = {}
  result.t_mode = t_mode
  result.p_mode = p_mode
  result.h_mode = h_mode
  result.iir = iir
  result.t_fine = 0
  result.T1 = read16(0x88)
  result.T2 = short(read16(0x8A))
  result.T3 = short(read16(0x8C))
  result.P1 = read16(0x8E)
  result.P2 = short(read16(0x90))
  result.P3 = short(read16(0x92))
  result.P4 = short(read16(0x94))
  result.P5 = short(read16(0x96))
  result.P6 = short(read16(0x98))
  result.P7 = short(read16(0x9A))
  result.P8 = short(read16(0x9C))
  result.P9 = short(read16(0x9E))
  result.H1 = read8(0xA1)
  result.H2 = short(read16(0xE1))
  result.H3 = read8(0xE3)

  local a = read8(0xE5)
  result.H4 = (read8(0xE4) lshift 4) + (a mod 16)
  result.H5 = (read8(0xE6) lshift 4) + (a rshift 4)
  result.H6 = read8(0xE7)
  if result.H6 > 127 then result.H6 -= 256

  write8(0xF2, h_mode)
  sleep_ms(10)
  write8(0xF4, 0x24)
  sleep_ms(10)
  write8(0xF5, iir lshift 2)
  return result
end

func read_raw_data(sdata)
  write8(0xF4, (sdata.p_mode lshift 5 | sdata.t_mode lshift 2 | 1))
  sleep_time = 1250

  if sdata.t_mode in [1, 2, 3, 4, 5] then
    sleep_time += 2300*(1 lshift  sdata.t_mode)
  endif
  if sdata.p_mode in [1, 2, 3, 4, 5] then
    sleep_time += 575+(2300*(1 lshift sdata.p_mode))
  endif
  if sdata.h_mode in [1, 2, 3, 4, 5] then
    sleep_time += 575+(2300*(1 lshift sdata.h_mode))
  endif

  sleep_ms(1 + sleep_time / 1000)

  while (read16(0xF3) & 0x08)
    sleep_ms(1)
  wend

  local raw_p = ((read8(0xF7) lshift 16) | (read8(0xF8) lshift 8) | read8(0xF9)) rshift 4
  local raw_t = ((read8(0xFA) lshift 16) | (read8(0xFB) lshift 8) | read8(0xFC)) rshift 4
  local raw_h = (read8(0xFD) lshift 8)| read8(0xFE)
  return [raw_t, raw_p, raw_h]
end

rem yikes !!!!
func read_compensated_data(sdata)
  local raw_t, raw_p, raw_h, var1, var2, var3, h

  [raw_t, raw_p, raw_h] = read_raw_data(sdata)

  var1 = ((raw_t rshift 3) - (sdata.T1 lshift 1)) * (sdata.T2 rshift 11)
  var2 = (raw_t  rshift  4) - sdata.T1
  var2 = var2*((raw_t rshift 4) - sdata.T1)
  var2 = ((var2 rshift 12) * sdata.T3) rshift 14
  sdata.t_fine = var1+var2

  local temp = (sdata.t_fine * 5 + 128) rshift 8
  var1 = sdata.t_fine - 128000
  var2 = var1 * var1 * sdata.P6
  var2 = var2 + ((var1*sdata.P5) lshift 17)
  var2 = var2 + (sdata.P4 lshift 35)
  var1 = (((var1 * var1 * sdata.P3) rshift 8) + ((var1 * sdata.P2) lshift 12))
  var1 = (((1 lshift 47)+var1)*sdata.P1) rshift 33
  if var1 == 0 then
    pres = 0
  else
    p = ((((1048576-raw_p) lshift 31)-var2) * 3125) / var1
    var1 = (sdata.P9 * (p rshift 13) * (p  rshift  13)) rshift 25
    var2 = (sdata.P8 * p) rshift 19
    pres = ((p + var1 + var2) rshift 8) + (sdata.P7 lshift 4)
  endif

  h = sdata.t_fine - 76800
  h = (((((raw_h lshift 14) - (sdata.H4 lshift 20) - (sdata.H5 * h)) + 16384) rshift 15) * &
       (((((((h * sdata.H6) rshift 10) * (((h * sdata.H3) rshift 11) + 32768)) rshift 10) + 2097152) * sdata.H2 + 8192) rshift 14))
  h = h - (((((h rshift 15) * (h rshift 15)) rshift 7) * sdata.H1) rshift 4)
  h = iff(h < 0, 0, h)
  h = iff(h > 419430400, 419430400, h)
  humi = h rshift 12
  return [temp, pres, humi]
end

func values(sdata)
  local temp, pres, humi
  [temp, pres, humi] = read_compensated_data(sdata)
  return [temp / 100, pres / 256, humi / 1024]
end

func pressure_precision(sdata)
  local p = read_compensated_data(sdata)[1]
  local pi = p / 256
  local pd = (p % 256) / 256
  return [pi, pd]
end

func altitude(sdata)
  local pi, pd
  local pressure_sea_level=1013.25
  [pi, pd] = pressure_precision(sdata)
  return 44330 * (1 - (((pi + pd) / 100) / pressure_sea_level) * (1 / 5.255))
end

sub sleep_ms(ms)
  delay ms
end
