'
' http://sourceforge.net/tracker/?func=detail&aid=1651503&group_id=22348&atid=375102
' Pause statement
'

PRINT "Start";timer
PAUSE 5
PRINT "End";timer

'This small program in WIN32 GUI variant working correctly BUT eating 100%
'of CPU!!
'
'In FLTK version not eating the CPU but terminated randomly 8 sec or 20 sec
'
'Alternative sb implementation:

For i=1 to n*50
if len(inkey) then exit
delay 20
Next i

'Where n is the "pause" statement attribute
