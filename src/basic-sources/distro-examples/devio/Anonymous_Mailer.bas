#!/usr/local/bin/sbasic
REM 
REM	Anonymous mailer
REM
REM	Using VFS socket-client driver

SUB SVR_REPLY
INPUT #1, errcode
PRINT "<- ";errcode
IF LEFT(errcode,1) = "5"
	STOP
FI
WHILE ( LOF(1) )
	SVR_REPLY
WEND
END

SUB SEND(cmd)
PRINT "-> "; cmd
PRINT #1, cmd
END

SUB SEND_AND_CHECK(cmd)	
SEND cmd
SVR_REPLY
END

DIM MSG(99)

PRINT CAT(1);"* ANONYMOUS MAILER *";CAT(0)
PRINT "SmallBASIC 0.7.1 - Example"
PRINT
PRINT "(*=required)"
INPUT "*SMTP server         "; server
INPUT "*Victim's address    "; victim
INPUT " HELO ID             "; heloid
IF EMPTY(heloid) THEN heloid=LEFTOF(victim,"@")
INPUT " Fake sender's name  "; sndname
IF EMPTY(sndname) THEN sndname="null pointer assignment"
INPUT " Fake sender's addr. "; sender
IF EMPTY(sender) THEN replyto="null@null.gr"
INPUT " Reply-To            "; replyto
IF EMPTY(replyto) THEN replyto="null@null.gr"
INPUT " Subject:            "; subject
IF EMPTY(subject) THEN subject="Just, read me"

PRINT
PRINT "Enter the message:"
PRINT "Press [ENTER] on an empty line to send"
FOR i=0 TO 98
	INPUT "LINE "+(i+1);MSG(i)
	IF EMPTY(MSG(i)) THEN EXIT
NEXT
MSG(i)="."

OPEN "SOCL:"+server+":25" AS #1
SVR_REPLY
SEND_AND_CHECK "HELO "+heloid
SEND_AND_CHECK "MAIL FROM: <"+victim+">"
SEND_AND_CHECK "RCPT TO: <"+victim+">"
SEND_AND_CHECK "DATA"
SEND "From: " + sndname + " <"+sender+">"
SEND "To: <" + victim + ">"
SEND "Reply-To: <"+replyto+">"
SEND "Subject: "+subject
SEND ""
FOR i=0 TO 99
	SEND MSG(i)
	IF MSG(i)="." THEN EXIT
NEXT
SVR_REPLY
SEND "QUIT"
CLOSE #1
END

