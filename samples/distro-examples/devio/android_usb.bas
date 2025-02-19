import android

usb = android.usbConnect()

while 1
  usb.send("hello");
  print usb.receive()
  delay 1000
wend

