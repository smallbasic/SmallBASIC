import teensy
import ssd1306 as display

const welcome = "Hello SmallBASIC!!"
display.init()
display.dim(1)
display.print(welcome)

const sz = display.getTextSize(welcome)
display.setCursor(0, sz.height + 2)
display.flush()

const out = teensy.openDigitalOutput(13)
const usb = teensy.openSerial()

sub show_data(byref s)
  display.clear()
  display.setCursor(0,0)
  display.print("Received:")
  display.setCursor(0, sz.height + 2)
  display.print(s)
  display.flush()
end

sub blink
  local i
  for i = 0 to 5
    out.write(value)
    value = !value
    delay 10 + (i * 10)
  next
  out.write(0)
end

while 1
  if usb.ready() then
    s = usb.receive()
    if (len(s) > 0) then
      show_data(s)
      usb.send(sbver)
      blink
    endif
  else
    delay 10
  endif
wend
