'app-plug-in
'menu Games/Plasma

' 10/4/04 2:54:58 PM
'Plasma
'jelly 2004
'http:/relo.betterwebber.com

cls
const PI= 3.141593

dim cols(2,255)

for i =0 to 255
   cols(0,i)=  abs(INT(128 - 127 * SIN(i * PI / 32)))
   cols(1,i)=  abs(INT(128 - 127 * SIN(i * PI / 64)))
   cols(2,i)=  abs(INT(128 - 127 * SIN(i * PI / 128)))
next i

for y = 0 to 240
   for x =0 to 319
      c = (sin(x/35)*128+sin(y/28)*32 + sin((x+y)/16)*64)
      if c >255 then c = c - 256
      if c < 0 then c = 256 + c
      r = cols(0, c)
      if r >255 then r = r - 256
      if r < 0 then r = 256 + c
      g = cols(1, c)
      if g >255 then g = g - 256
      if g < 0 then g = 256 + c
      b = cols(2, c)
      if b>255 then b = b - 256
      if b < 0 then b = 256 + c
      col = rgb(r, g, b)      
      pset x, y, col
   next x
next y   
end
