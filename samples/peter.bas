' bin.bas
' 02/06/2002
dim sbuf(16)
dim rbuf(16)
etx=5
stx=6
stufi=7
set=21
get=20
OK=1
NOK=0

sbuflen=5
rbuflen=0

sbuf(0)=50
sbuf(1)=40
sbuf(2)=get
sbuf(3)=240
sbuf(4)=230
sbuf(5)=220
sbuf(6)=30
slen=7

OPEN "COM2:9600" AS #1
? "lof (1) ", lof(1)
print #1, "A";
crc
? c
sbuf(i) = c
slen=slen+1
? send
? receive
close #1
end

func send
BPUTC #1,stx
for i=0 to slen -1
 if sbuf(i) = stx then  BPUTC #1,stuff
 if sbuf(i) = etx then  BPUTC #1,stuff
 if sbuf(i) = stuff then  BPUTC #1,stuff
 BPUTC #1,sbuf(i)
next i

BPUTC #1,etx
send = OK
end

func receive
b = 0
a = gets
while a<> etx
  a =gets
  if b=stuff then 
    rbuf (i)=a 
    a=0
    b=0
    i=i+1
  endif
  if a=stuff then 
    b=stuff
  else 
    rbuf(i)= a
    i=i+1
  endif
wend
receive=OK
end

sub crc
c=0
for i =0 to slen-1
c= (c+sbuf(i)) MOD 256
?"c= ",c,"sbuf(i)= ",sbuf(i),"i= ",i
next i
end


func gets
  'while if lof(1)=0:wend
  'ndc
  while lof(1)=0:wend
  gets=BGETC(1)
end
