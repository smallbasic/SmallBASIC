#!/usr/local/bin/sbasic -g
' sound-game
cls:15 s=int(100*rnd)+1
20 ?
sound s*10,500
input "What is your guess?";g
sound g,500:g=g/10
if g=s
	?"You guessed it!"
	? "Let's play again.":?
	goto 15
fi
if g>s
	?"Too big!"
else
	?"Too small!"
fi
goto 20


