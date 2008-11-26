'app-plug-in
'menu Games/Mira fractals

' Mira fractals
' Martin Latter
' Converted to SB from my old 1992 version in Archimedes BASIC - SB and modern computers thankfully plot a bit faster.
' (Run/f9 to replot)

dim ca(6)
xoffs = xmax/2
yoffs = ymax/2
dots=12000
b=0.9998

while (1)
    cls
    randomize
    a=rnd

    c=2-2*a
    x=j=0
    y=rnd*12+0.1
    col=2

    for pc=0 to 5
        ca(pc)=round(rnd*15)
    next

    for i=0 to dots
        z=x
        x=b*y+j
        j=a*x+c*(x^2)/(1+x^2)
        y=j-z
        xp=(x*20)+xoffs
        yp=(y*20)+yoffs

        if i>1000 then 
            col=ca(0)
        elif i>3000 then 
            col=ca(1)
        elif i>5000 then 
            col=ca(2)
        elif i>7000 then 
            col=ca(3)
        elif i>9000 then 
            col=ca(4)
        elif i>11000 then 
            col=ca(5)
        fi
        pset xp,yp,col
    next i
wend

