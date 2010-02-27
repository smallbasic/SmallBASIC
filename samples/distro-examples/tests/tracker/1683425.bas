'
'http://sourceforge.net/tracker/?func=detail&aid=1683425&group_id=22348&atid=375102
'

'The following DRAW statement does not work correctly:

DRAW "BM100,100D100R100U100L100"

'There are two bugs in there:
'1. "B" means draw nothing in the WHOLE string not just the next move "M"
'
'2. If you clear the "B" the move "M" lose one character after coma means
'"M100,100" will go to 100,0
'
'Workaround:
'
'1. Do not use "B"
'2. Use "M100,0100" which work correctly

