'
'http://sourceforge.net/tracker/?func=detail&aid=1662591&group_id=22348&atid=375102
'

'The inkey$ and the play commands have problems in SmallBasic version FLTK0.9.7!
'e.g.

cls
while not inkey$
 play "abc"
wend

'In this example, the computer plays a sound only, when a key is holding 
'down! Crying or Very sad

cls
play "mbab" 'MB=play background

'Here, the computer doesn't play any sound!
