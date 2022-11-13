goto TestLabel

print "GOTO failed"

LABEL TestLabel

REM Don't crash when label does not exist
goto 1
