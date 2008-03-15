' PTB-Time
' 28/11/2001
F=FREEFILE

'OPEN "SOCL:ptbtime1.ptb.de:13" AS #F

OPEN "SOCL:192.53.103.103:13" AS #F
    LINE INPUT #F, timestr
CLOSE #F
PRINT timestr
