'app-plug-in
'menu Games/Bolmo

label start
dim map1(-10 to 90)
for rtm=1 to 90
read map1(rtm)
next rtm

randomize
read obp
read obx:read oby



color 14,7
cls

sound 700,250,50 bg
sound 900,250,50 bg
sound 800,250,50 bg
sound 700,125,50 bg
sound 800,125,50 bg
sound 900,250,50 bg
sound 800,125,50 bg
sound 700,250,50 bg

print " Bolmo: Takin Hunter"
print "By Trevor Goodwin"

repeat
line 4,15,line1x,15
line1x=line1x+6
delay 100
until line1x=180

circle line1x,14,10 filled
circle line1x,14,2 color 9 filled

print
print "Press any key to continue..."
repeat
p=inkey$
until p<>""


1 color 14,7
cls
print " Bolmo: Takin Hunter"
print "Main Menu"
print "1. Play the game"
print "2. Story & Instructions"
print "3. About"
print "4. Info on map making"
print "5. Map Maker"
print "6. Quit"
print

input "What is your choice?",ch
if ch="2" then
goto choice1
elif ch="3" then
goto choice2
elif ch="4" then
goto choice3
elif ch="5" then
goto mak
elif ch="6" then
goto finish
elif ch="1" then
goto thegame
fi
goto 1

label choice1
cls
print "Bolmo: Takin Hunter"
print "Story & Instructions"
print
print " Bolmo is a creature of the ground. He lives in his little burrow."
print "Once a week, he must leave to get food. You will control Bolmo"
print "in this game. But be careful, the takins do not like being disturbed."
print "It is a 1 out of 6 chance that a takin will attack you."
print "ICONS: Y=tree, ==Takin, X=Bolmo, o=burrow, &=food."
print "CONTROLS: w=up, s=down, d=right, a=left, q=quit."
print "Get the food and head back home."
print

print "Press any key to continue..."
repeat
p=inkey$
until p<>""
goto 1

label choice2
cls
print "Bolmo: Takin Hunter"
print "About"
print
print " This game was made by Trevor Goodwin. As you've probably noticed,"
print "I'm quite a beginner at programming, but my focus is having nice,"
print "clean code. This is the second version of Bolmo and it includes"
print "many new features."
print "This game is slightly inspired by Jocke the Beast's: Dark Woods,"
print "which was written in QBasic. I don't have any plans on"
print "including any more features to the game or map maker because I"
print "really want to make the SmallBasic community larger so more people"
print "will play my game and start their own projects."
print "If you are looking for a very easy programming language to learn"
print "the 'BASIC's' then try SB out!"
print "I hope you enjoy it and you can hand me an email @:"
print "trevor1@joinme.com if you want."
print
print "Press any key to continue..."
repeat
p=inkey$
? p
until p<>""
goto 1

label choice3
cls
print "Bolmo: Takin Hunter"
print "Info on map making"
print
print " Making maps is very simple. At the bottom of the game code, I have"
print "9 lines of data. Each number represents a tile. 0=blank spot,"
print "1=tree(obstacle), 2=takin(obs.), 3=burrow(obs.) and 4=food."
print "Below the map data is the absolute and the x & y starting position."
print "You'll need to make the last line of map all obstacles and don't"
print "make the starting position an obstacle."
print "I reccomend that you make a notepad file to store all your maps and"
print "I obviously don't mind you going over the original. You can use the"
print "Map Maker or do it manually at the bottom of the code."
print "Notes:"
print "Each map is 10X9 tiles big. That's 90 tiles but you can't use the"
print "last 10. In order to finish the game, you'll need to have at least"
print "one food and one burrow. If you find any bugs in the map maker,"
print "please email me. After you have finished the 80'th tile, it will"
print "prompt you for the starting position."
print "Press any key to continue..."
repeat
p=inkey$
until p<>""
goto 1

label finish
print "Thankyou for playing my game. I really hope enjoyed it."
print "Trevor Goodwin"
print "trevor1@joinme.com"
end

label mak
lp=0
sn=0
nlp=1
if sn=0 then
goto mak2
fi
label mak4
sn=1
input "Which tile";nlp
if nlp<1 or nlp>79 then
goto mak4
fi
label mak2
if sn=1 then
lp=nlp-1
fi
sn=0
cls
wt=1
lp++

for y=1 to 9
for x=1 to 10
if map1(wt)=0 then
locate y,x:print " "
elif map1(wt)=1 then
locate y,x:print "Y"
elif map1(wt)=2 then
locate y,x:print "="
elif map1(wt)=3 then
locate y,x:print "o"
elif map1(wt)=4 then
locate y,x:print "&"
fi
wt++
next x
next y
print "123456789T"
label mak3
print
print "0=Nothing, 1=Tree, 2=Takin, 3=Burrow, 4=Food, Posistion=";lp;"."
input "q=quit map maker 5=skip tile  6=specific. Place/do what";ch2
if lcase(ch2)="q" then
goto 1
elif ch2<0 or ch2>6 then
goto mak3
fi
if ch2=5 and lp<80 then
goto mak2
elif ch2=6 and lp<80 then
goto mak4
fi
map1(lp)=ch2
if lp=<79 then
goto mak2
fi
input "Absolute starting posistion"; obp
if obp<1 or obp>80 then
goto mak2
fi
input "Horizontal (x) starting position"; obx
if obx<1 or oby>10 then
goto mak2
fi
input "Vertical (y) starting position"; oby
if oby<1 or oby>8 then
goto mak2
fi


print "Press any key to continue..."
repeat
p=inkey$
until p<>""
goto 1

label thegame
bp=obp:bx=obx:by=oby
r1=0
fd=0:win=0:dead=0
color 0,15
cls
wt=1





color 4,10
for y=1 to 9
for x=1 to 10
if map1(wt)=0 then
locate y,x:print " "
elif map1(wt)=1 then
locate y,x:print "Y"
elif map1(wt)=2 then
locate y,x:print "="
elif map1(wt)=3 then
locate y,x:print "o"
elif map1(wt)=4 then
locate y,x:print "&"
fi
wt++
next x
next y

color 12,2
locate by,bx:print "X"
oldx=bx:oldy=by

repeat
p=inkey
if lcase(p)="w" and by-1>0 then
gosub move1
elif lcase(p)="s" and by+1<9 then
gosub move2
elif lcase(p)="d" and bx+1<11 then
gosub move3
elif lcase(p)="a" and bx-1>0 then
gosub move4
fi
until p="q" or win=1 or dead=1

color 12,15
locate 15,1
if win=1 then
print "CONGRADULATIONS you have a full tummy and you are safe!"
elif dead=1 then
print "You have been bashed to death by a Takin's head."
fi
sound 600,250,50 bg
sound 900,250,50 bg
sound 700,250,50 bg
print "Press any key to continue..."
repeat
p=inkey$
until p<>""
goto 1

label move1
if map1(bp-10)=1 or map1(bp-10)=2 or map1(bp-10)=3 then
return:fi
bp=bp-10
by--
goto aftermove

label move2
if map1(bp+10)=1 or map1(bp+10)=2 or map1(bp+10)=3 then
return:fi
bp=bp+10
by++
goto aftermove

label move3
if map1(bp+1)=1 or map1(bp+1)=2 or map1(bp+1)=3 then
return:fi
bp++
bx++
goto aftermove

label move4
if map1(bp-1)=1 or map1(bp-1)=2 or map1(bp-1)=3 then
return:fi
bp--
bx--
goto aftermove

label aftermove
color 4,10:locate oldy,oldx:print " "
color 12,2:locate by,bx:print "X"
oldx=bx:oldy=by

if map1(bp)=4 then
fd=1
fi

if map1(bp-10)=3 or map1(bp+10)=3 or map1(bp+1)=3 or map1(bp-1)=3 and fd=1 then
win=1
fi

r1l=1
r1=1
if map1(bp-10)=2 or map1(bp+10)=2 or map1(bp+1)=2 or map1(bp-1)=2 then
for r1l=1 to 6
r1=r1+round(rnd)
next r1l
if r1<6 then
return:fi

dead=1
fi
return


data 1,2,0,0,0,0,1,0,3,1
data 1,0,0,1,1,0,0,0,1,1
data 1,0,1,0,1,0,1,0,0,0
data 0,0,2,2,0,0,1,1,0,1
data 0,4,0,1,0,1,1,2,0,1
data 2,0,0,1,0,1,1,1,0,1
data 1,1,0,1,0,0,0,0,0,1
data 1,0,0,0,0,1,1,1,0,0
data 1,1,1,1,1,1,1,1,1,1


data 8,8,1

'Default map
'data 1,2,0,0,0,0,1,0,3,1
'data 1,0,0,1,1,0,0,0,1,1
'data 1,0,1,0,1,0,1,0,0,0
'data 0,0,2,2,0,0,1,1,0,1
'data 0,4,0,1,0,1,1,2,0,1
'data 2,0,0,1,0,1,1,1,0,1
'data 1,1,0,1,0,0,0,0,0,1
'data 1,0,0,0,0,1,1,1,0,0
'data 1,1,1,1,1,1,1,1,1,1


'data 8,8,1


