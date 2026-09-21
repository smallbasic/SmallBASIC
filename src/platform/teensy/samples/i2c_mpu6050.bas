' MPU6050 - I2C accelerometer and gyroscope
'
' Pins:
'   TEENSY      MPU6050
'   16 SCL1 --> SCL
'   17 SDA1 --> SDA
'      3.3V --> VCC
'      GND  --> GND

import teensy

const ADDRESS = 0x68

Init()

currentTime = ticks()
lastUpdate = ticks()

while(1)
  ' Read accelerometer data
  Accel = GetAcceleration()
  AccX = Accel[0]
  AccY = Accel[1]
  AccZ = Accel[2]

  ' Get delta time
  previousTime = currentTime
  currentTime = ticks()
  dt = (currentTime - previousTime) / 1000

  ' Read gyroscope data
  Gyro = GetGyroscope()
  GyroX = Gyro[0]
  GyroY = Gyro[1]
  GyroZ = Gyro[2]

  ' Calc roll and pitch using acceleration sensor
  AccRoll = atan2(AccY, AccZ) * 180 / PI
  AccPitch = atan2(-AccX, sqr(AccY^2 + AccZ^2)) * 180/PI

  ' Calc roll and pitch unsing gyroscope and accelerometer to reduce noise and cancel drift
  roll = 0.98 * (roll + GyroX * dt) + 0.02 * AccRoll
  pitch = 0.98 * (pitch + GyroY * dt) + 0.02 * AccPitch

  ' Display every 500ms
  if(ticks() - lastUpdate > 500)
    print "Roll : "; round(roll, 2), " Pitch : "; round(pitch, 2), " Temperature: "; round(GetTemperature(),1); "°C"
    ' uncomment to see roll and pitch measured with accelerometer only
    ' print "AccRoll : "; round(AccRoll, 2), " AccPitch: "; round(AccPitch, 2)
    lastUpdate = ticks()
  endif
wend

' #### Functions #############################################

sub Init()
  local buffer, Who_am_I

  Print "Open I2C...";
  const I2C = teensy.OpenI2C(1)   ' use I2C pins 16 and 17
  Print "Opened"

  ' Test if sensor is present
  I2C.write(ADDRESS, 0x75)
  Who_am_I = I2C.read(ADDRESS, 1)
  if(Who_am_I != 0x68 AND Who_am_I != 0x69)
    print "Error: MPU6050 WHO_AM_I value wrong"
    stop
  endif

  ' Set SMPLRT_DIV to 0
  ' Sample Rate = Gyroscope Output Rate / (1 + SMPLRT_DIV)
  buffer = [0x19, 0]
  I2C.write(ADDRESS, buffer)

  ' MPU config
  ' external Frame Synchronization disabled
  buffer = [0x1A, 0]
  I2C.write(ADDRESS, buffer)

  GyroscopeConfig(250)
  AccelerationConfig(2)

  ' Turn on
  buffer = [0x6B, 0x01]
  I2C.write(ADDRESS, buffer)
end

func short(dat)
    if dat > 32767 then
        return dat - 65536
    else
        return dat
    endif
end

func GetAcceleration()
  local buffer, AccX, AccY, AccZ
  dim buffer[5]

  I2C.write(ADDRESS, 0x3B)
  buffer = I2C.read(ADDRESS, 6)

  AccX = short((buffer[0] lshift 8) BOR buffer[1]) / AccelerationLSBSensitivity
  AccY = short((buffer[2] lshift 8) BOR buffer[3]) / AccelerationLSBSensitivity
  AccZ = short((buffer[4] lshift 8) BOR buffer[5]) / AccelerationLSBSensitivity

  return [AccX, AccY, AccZ]
end

func GetGyroscope()
  local buffer, GyroX, GyroY, GyroZ
  dim buffer[5]

  I2C.write(ADDRESS, 0x43)
  buffer = I2C.read(ADDRESS, 6)

  ' Correct the outputs with the error values
  GyroX = short((buffer[0] lshift 8) BOR buffer[1]) / GyroscopeLSBSensitivity
  GyroY = short((buffer[2] lshift 8) BOR buffer[3]) / GyroscopeLSBSensitivity
  GyroZ = short((buffer[4] lshift 8) BOR buffer[5]) / GyroscopeLSBSensitivity

  return [GyroX, GyroY, GyroZ]
end

func GetTemperature()
  local buffer, Temp
  dim buffer[1]

  I2C.write(ADDRESS, 0x41)
  buffer = I2C.read(ADDRESS, 2)

  Temp = short((buffer[0] lshift 8) BOR buffer[1]) / 340.0 + 36.53

  return Temp
end

' GyroscopeConfig(range)
' Sets the full scale range of the Gyroscope. Range can have the
' values 250, 500, 1000, or 2000 in °/s.
sub GyroscopeConfig(range)
  local Setting, buffer
  dim buffer[1]

  select case range
    case 250
      Setting = 0x00
      GyroscopeLSBSensitivity = 131.0
    case 500
      Setting = 0x08
      GyroscopeLSBSensitivity = 65.5
    case 1000
      Setting = 0x10
      GyroscopeLSBSensitivity = 32.8
    case 2000
      Setting = 0x18
      GyroscopeLSBSensitivity = 16.4
    case else
      Setting = 0x00
      GyroscopeLSBSensitivity = 131.0
  end select

  buffer[0] = 0x1B
  buffer[1] = Setting
  I2C.write(ADDRESS, buffer)
end

' Accelerometer Configuration
'  AccelerationConfig(range)
'  Sets the full scale range of the Accelerometer. Range can have the
'  values 2, 4, 8, or 16, which corresponds to the following g-force
'  2 ->  2g
'  4 ->  4g
'  8 ->  8g
'  16 -> 16g
sub AccelerationConfig(range)
  local setting, buffer
  dim buffer[1]

  select case range
    case 2
      Setting = 0x00
      AccelerationLSBSensitivity = 16384
    case 4
      Setting = 0x08
      AccelerationLSBSensitivity = 8192
    case 8
      Setting = 0x10
      AccelerationLSBSensitivity = 4096
    case 16
      Setting = 0x18
      AccelerationLSBSensitivity = 2048
    case else
      Setting = 0x00
      AccelerationLSBSensitivity = 16384
  end select

  buffer[0] = 0x1C
  buffer[1] = Setting
  I2C.write(ADDRESS, buffer)
end