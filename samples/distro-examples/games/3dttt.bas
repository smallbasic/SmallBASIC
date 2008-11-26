'app-plug-in
'menu Games/3D TicTacToe

'   *****  3D Tic-Tac-Toe   *****
'   *****    4 x 4 x 4   *****
' SmallBasic program 02.Sept.2004
'      by Keijo Koskinen
'
' The idea of this game is very old
' and propably it has already been rewritten
' thousands of times by others.
' I tried to find out a code for it,
' but I couldn't so I made it for my sons
' mainly because I wanted to prove my kids,
' that a game can be interesting  without
' any fancy "movie" like graphics.
' Therefore this project with its code, data and algorithms
' are assuredly my own creation for this old game.
' And it has no warranty against bugs and crashes.
'
' SmallBasic program 02.Sept.2004

label alku0
 Cls
 Dim pv(76, 4)
 Dim pp(64)
 Dim pa(64)
 Dim pb(64)
 Dim ps(64)
 Dim pz(64)

 ' If you want easier level put  tic=0
 tic=1
 vv=1

' rows
 restore datat
 gosub arvot0
 For i=1 To 76: Read a: pv(i, 1) = a: Read a: pv(i, 2) = a: Read a: pv(i, 3) = a: Read a: pv(i, 4) = a: Next
' Read values
 For i=1 To 64: Read a: pp(i) = a: ps(i) = a: Next

' Draw levels
    Line 129,1,280,1
   Line 117,20,270,20
  Line 105,39,260,39
 Line 93,58,250,58
Line 81,77,240,77
         Line 129,1,81,77
         Line 167,1,120,77
         Line 206,1,160,77
         Line 245,1,200,77
         Line 280,1,240,77
    Line 129, 81,280,81
   Line 117, 100,270,100
  Line 105,119,260,119
 Line 93, 138,250,138
Line 81, 157,240,157
         Line 129,81,81,157
         Line 167,81,120,157
         Line 206,81,160,157
         Line 245,81,200,157
         Line 280,81,240,157
    Line 129, 161,280, 161
   Line 117, 180,270,180
  Line 105,199,260,199
 Line 93, 218,250,218
Line 81, 237,240,237
         Line 129,161,81, 237
         Line 167,161,120,237
         Line 206,161,160,237
         Line 245,161,200,237
         Line 280,161,240,237
    Line 129, 241,280, 241
   Line 117, 260,270,260
  Line 105,279,260,279
 Line 93, 298,250,298
Line 81, 317,240,317
         Line 129,241,81, 317
         Line 167,241,120,317
         Line 206,241,160,317
         Line 245,241,200,317
         Line 280,241,240,317
locate 1,1:?"     3 D"
locate 2,1:?"    TIC"
locate 3,1:?"   TAC"
locate 4,1:?"  TOE"
 rect 1,1,319,319,1
 pen off
 delay 1000

 label main
'c=Column
'r= Row
'l=level

label Mouse
  PEN ON
 label alku
  if pen(0) then
   x=PEN(4)
   y=PEN(5)
   goto jatko
  endif
 goto mouse

label jatko
  r=Int((y-1)/20)+1 'r=Int((y-35)/20)+1
  xr=r
  l=1
 If xr>4 Then xr=xr-4:l=2
  If xr>4 Then xr=xr-4:l=3
   If xr>4 Then xr=xr-4:l=4

  poisto=(4-xr)*12+80
  uusx=x-poisto
 If uusx>1 And uusx<40 Then c=1
  If uusx>40 And uusx<80 Then c=2
   If uusx>80 And uusx<120 Then c=3
    If uusx>120 And uusx<160 Then c=4

m = c + ((xr - 1) * 4) + ((l - 1) * 16)

apux=300
apuy=200
If pz(m) > 0 Then
 locate 8,2: print "  UPS!"
 locate 9,2: print " Spot"
 locate 10,2:print "used"
 Goto main
 FI
 locate 8,2: print "      "
 locate 9,2: print "     "
 locate 10,2:print "    "
pa(m) = 1
 pz(m) = 1

 ' Drawing  o o ***********
y= r*20-10
x=poisto-11+(c*38)
circle x,y,6,.7,2  'draw a circle

locate 9,2:print "hmm..."
For i = 1 To 64
 If pa(i) > 0 Then pa(i) = 1
 If pz(i) < 1 Then pa(i) = 0
Next

'Human win ?
For i = 1 To 76
  bb = pa(pv(i, 1)) + pa(pv(i, 2)) + pa(pv(i, 3)) + pa(pv(i, 4))
 If bb = 4 Then
  locate 8,2: print "  YOU"
  locate 9,2:print " WIN!"
   for iii=1 to 4
     nnn=pv(i,iii)
     r= Int((nnn - 1) / 4) + 1
     c= nnn - ((r - 1) * 4)
     y=r*20-12 'y= (r + 1)*20+4
     x= (10 - r + (Int((nnn-1) / 16) * 4) + (4 * c))*10+4
    rect x-6,y-6,x+6,y+6,12
   next

   delay 2000
  locate 11,2:print"  PLAY"
  locate 12,2:print"AGAIN"
  locate 13,2:input"Yes/No",tt
 if tt="y" or tt="Y" then
  locate 11,2:print"      "
  locate 12,2:print"      "
  locate 13,2:print"      "
   restore datat
   gosub arvot0
  Goto alku0
 fi
  End
 FI
Next

'  Even ?
 loppu = 0
For i = 1 To 64
   loppu = loppu + pz(i)
Next
 If loppu=64 Then
  locate 8,2: print " BOTH"
  locate 9,2:print " WIN!"
   delay 2000
  locate 11,2:print"  PLAY"
  locate 12,2:print"AGAIN"
  locate 13,2:input"Yes/No",tt
 if tt="y" or tt="Y" then
  locate 11,2:print"      "
  locate 12,2:print"      "
  locate 13,2:print"      "
   restore  datat
   gosub arvot0
  Goto alku0
 fi
  End
 fi

For i = 1 To 64: ps(i) = pp(i): Next
 vert = 0
 aaa=Rnd(1)*2
 If aaa=0 Then vv=vv*(-1)
 If vv=1 Then sss1=1:sss2=76:vv=vv*(-1)
  If vv=-1 Then sss1=76:sss2=1:vv=vv*(-1)
   If sss1=1 Then Goto eka
    If sss1=76 Then Goto toka

label eka
For i = sss1 To sss2
  nn = 0
  bv = pa(pv(i, 1)) + pa(pv(i, 2)) + pa(pv(i, 3)) + pa(pv(i, 4)) - pb(pv(i, 1)) - pb(pv(i, 2)) - pb(pv(i, 3)) - pb(pv(i, 4))
 If bv > 1 Then nn = i
 If nn = i And pz(pv(i, 1)) = 0 Then ps(pv(i, 1)) = (ps(pv(i, 1)) * tic) + pp(pv(i, 1)) ' ELSE pa(pv(i, 1)) = 1
  If nn = i And pz(pv(i, 2)) = 0 Then ps(pv(i, 2)) = (ps(pv(i, 2)) * tic) + pp(pv(i, 2)) ' ELSE pa(pv(i, 2)) = 1
   If nn = i And pz(pv(i, 3)) = 0 Then ps(pv(i, 3)) = (ps(pv(i, 3)) * tic) + pp(pv(i, 3)) ' ELSE pa(pv(i, 3)) = 1
    If nn = i And pz(pv(i, 4)) = 0 Then ps(pv(i, 4)) = (ps(pv(i, 4)) * tic) + pp(pv(i, 4)) ' ELSE pa(pv(i, 4)) = 1
Next
Goto noniin

label toka
For i = sss1 To sss2 Step -1:
  nn = 0
  bv = pa(pv(i, 1)) + pa(pv(i, 2)) + pa(pv(i, 3)) + pa(pv(i, 4)) - pb(pv(i, 1)) - pb(pv(i, 2)) - pb(pv(i, 3)) - pb(pv(i, 4))
 If bv > 1 Then nn = i
 If nn = i And pz(pv(i, 1)) = 0 Then ps(pv(i, 1)) = (ps(pv(i, 1)) * tic) + pp(pv(i, 1)): ' ELSE pa(pv(i, 1)) = 1
  If nn = i And pz(pv(i, 2)) = 0 Then ps(pv(i, 2)) = (ps(pv(i, 2)) * tic) + pp(pv(i, 2)): ' ELSE pa(pv(i, 2)) = 1
   If nn = i And pz(pv(i, 3)) = 0 Then ps(pv(i, 3)) = (ps(pv(i, 3)) * tic) + pp(pv(i, 3)): ' ELSE pa(pv(i, 3)) = 1
    If nn = i And pz(pv(i, 4)) = 0 Then ps(pv(i, 4)) = (ps(pv(i, 4)) * tic) + pp(pv(i, 4)): ' ELSE pa(pv(i, 4)) = 1
Next

label noniin
  winvert = 0
For i = 1 To 64
 If ps(i) > winvert And pz(i) = 0 Then nnn = i: winvert = ps(i)
Next

'Is there  3 in row *************************
verk = 2
ne = 0
For iu = 1 To 76
 pab = pa(pv(iu, 1)) + pa(pv(iu, 2)) + pa(pv(iu, 3)) + pa(pv(iu, 4)) - pb(pv(iu, 1)) - pb(pv(iu, 2)) - pb(pv(iu, 3)) - pb(pv(iu, 4))
If pab > 2 Then ne = iu
 Next
 If ne > 0 And pz(pv(ne, 1)) = 0 Then nnn = pv(ne, 1)
  If ne > 0 And pz(pv(ne, 2)) = 0 Then nnn = pv(ne, 2)
   If ne > 0 And pz(pv(ne, 3)) = 0 Then nnn = pv(ne, 3)
    If ne > 0 And pz(pv(ne, 4)) = 0 Then nnn = pv(ne, 4)
 verk = 2
 ne = 0
For iu = 1 To 76
 pbb = pb(pv(iu, 1)) + pb(pv(iu, 2)) + pb(pv(iu, 3)) + pb(pv(iu, 4)) - pa(pv(iu, 1)) - pa(pv(iu, 2)) - pa(pv(iu, 3)) - pa(pv(iu, 4))
If pbb > 2 Then ne = iu
 Next
 If ne > 0 And pz(pv(ne, 1)) = 0 Then nnn = pv(ne, 1)
  If ne > 0 And pz(pv(ne, 2)) = 0 Then nnn = pv(ne, 2)
   If ne > 0 And pz(pv(ne, 3)) = 0 Then nnn = pv(ne, 3)
    If ne > 0 And pz(pv(ne, 4)) = 0 Then nnn = pv(ne, 4)
 pb(nnn) = 1: pz(nnn) = 1
 l= nnn
 c= nnn
 r= Int((nnn - 1) / 4) + 1
 c= nnn - ((r - 1) * 4)

'Drawing x *******************************************
 y=r*20-10
 x= (10 - r + (Int((nnn-1) / 16) * 4) + (4 * c))*10+4
delay 300
 locate 9,2:print "      "
 line x-4,y-4,x+5,y+5,5:line x-4,y+4,x+5,y-5,5

'Machine ... *****
For i=1 To 64
 If pb(i) > 0 Then pb(i) = 1: pa(i) = 0
Next

'Did machine win ?
For i=1 To 76
  bv = pb(pv(i, 1)) + pb(pv(i, 2)) + pb(pv(i, 3)) + pb(pv(i, 4))
 If bv = 4 Then
  locate 8,2: print "MACHINE"
  locate 9,2:print " WIN!"
   for iii=1 to 4
     nnn=pv(i,iii)
     r= Int((nnn - 1) / 4) + 1
     c= nnn - ((r - 1) * 4)
     y=r*20-12 'y= (r + 1)*20+4
     x= (10 - r + (Int((nnn-1) / 16) * 4) + (4 * c))*10+4
    rect x-6,y-6,x+6,y+6,12
   next
   delay 2000
  locate 11,2:print"  PLAY"
  locate 12,2:print"AGAIN"
  locate 13,2:input"Yes/No",tt
 if tt="y" or tt="Y" then
  locate 11,2:print"         "
  locate 12,2:print"       "
  locate 13,2:print"        "
   restore datat
   gosub arvot0
  Goto alku0
 fi
 end
 FI
Next

Goto main

label arvot0
 for ii=0 To 76
  for i=0 to 4
   pv(ii, i)=0
  next
 next
 for ii=0 to 64
  pp(ii)=0
  pa(ii)=0
  pb(ii)=0
  ps(ii)=0
  pz(ii)=0
  vv=1
 Next
Return
label datat
' Winning rows  6+5+5+6+5+5+6+6+4+4+4+4+4+4+4+4=76
Data 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24
Data 25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44
Data 45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64
Data 1,5,9,13,2,6,10,14,3,7,11,15,4,8,12,16,17,21,25,29,18,22,26,30
Data 19,23,27,31,20,24,28,32,33,37,41,45,34,38,42,46,35,39,43,47
Data 36,40,44,48,49,53,57,61,50,54,58,62,51,55,59,63,52,56,60,64
Data 1,17,33,49,2,18,34,50,3,19,35,51,4,20,36,52,5,21,37,53,6,22,38,54
Data 7,23,39,55,8,24,40,56,9,25,41,57,10,26,42,58,11,27,43,59,12,28,44,60
Data 13,29,45,61,14,30,46,62,15,31,47,63,16,32,48,64
Data 1,21,41,61,2,22,42,62,3,23,43,63,4,24,44,64
Data 13,25,37,49,14,26,38,50,15,27,39,51,16,28,40,52
Data 1,18,35,52,5,22,39,56,9,26,43,60,13,30,47,64
Data 4,19,34,49,8,23,38,53,12,27,42,57,16,31,46,61
Data 1,6,11,16,17,22,27,32,33,38,43,48,49,54,59,64
Data 4,7,10,13,20,23,26,29,36,39,42,45,52,55,58,61
Data 1,22,43,64,4,23,42,61,16,27,38,49,13,26,39,52

' Starting values of the spots
Data 8,5,5,8,5,5,5,5,5,5,5,5,8,5,5,8
Data 5,7,7,5,7,9,9,7,7,9,9,7,5,7,7,5
Data 5,7,7,5,7,9,9,7,7,9,9,7,5,7,7,5
Data 8,5,5,8,5,5,5,5,5,5,5,5,8,5,5,8
' 15/09/2004 11:11:20 AM

