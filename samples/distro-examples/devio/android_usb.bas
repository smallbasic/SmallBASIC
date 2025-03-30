import android

usb = android.openUsbSerial(0x16C0)

while 1
  input k
  n = usb.send(k);
  print "sent "; n
  print usb.receive()
wend

