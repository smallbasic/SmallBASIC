'
' provided by rpnielsen
' https://www.syntaxbomb.com/smallbasic/the-play-string-command-and-the-latest-beta/
'
nrtests=5
tempo=240 '1/4 notes pr min
length=4  'play 1/x notes

expDur=(240000/tempo)/length

play "V10O3T"+tempo+"L"+length

? nrtests;" tests of 10 1/";length;" notes"
? "at tempo = ";tempo;" 1/4 notes pr minute"
? "Expected note duration = ";expDur;" ms."
? "--------------------------"
?

for a=1 to nrtests
  delay 50 'because otherwise nothing is printed to screen until after PLAY is done
  t=ticks
  play "CDEFGGFEDC"
  actDur=(ticks-t)/10
  dev=actDur-expDur
  ? "Test ";a
  ? "Average note duration = ";actDur;" ms."
  ? "Deviaton = ";dev;" ms = ";((actDur/expDur)-1)*100;" %"
  totalDur=totalDur+actDur
  totalDev=totalDev+dev
  ?
next a

? "Overall average:"
? "Duration  = ";totalDur/nrtests;" ms"
? "Deviation = ";totalDev/nrtests;" ms = ";(((totalDur/nrtests)/expDur)-1)*100;" %"

end
