# https://news.ycombinator.com/item?id=15122540
# linear feedback shift register
# https://gist.github.com/anonymous/a42eedfdfc22c469141f8f7d0419fa90

sub fizzle(j)
  rndval = 1
  while 1
    y = rndval band j+0xff    ' Y = low 8 bits
    x = rndval rshift 8       ' X = High 9 bits
    lsb = rndval band 1       ' Get the output bit.
    rndval = rndval rshift 1  ' Shift register
    if (lsb) then             ' If the output is 0, the xor can be skipped.
      rndval = rndval xor 0x00012000
    endif
    pset 10+x,10+y  color 6+rnd*3
    if rndval == 1 then
      exit loop
    endif
  wend
end

while 1
  fizzle(-rnd*3)
wend
