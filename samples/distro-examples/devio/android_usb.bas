import android

usb = android.openUsbSerial(0x16C0)

while 1
  usb.send("hello");
  print usb.receive()
  delay 1000
wend

