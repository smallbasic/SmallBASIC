' === sin(x)
cls
at xmax/3, ymax/2: print "SIN(X)"
plot 0, 2*pi use sin(x)
pause

' === boles (gravity)
u0=35
th=pi/3

func boles(x)
t=x/(u0*cos(th))
y = u0*sin(th)*t - 0.5*9.7*t^2
boles = y
end

cls
at xmax/3, ymax/2: print "BOLES u0=";u0;"m/sec è=";th
plot 0, 100 use boles(x) 
pause

' === relative
c=300
cls
at xmax/3, ymax/2: print "0-1c light-speed, x=du/2, y=u'"
' mass a had same u as mass b but in reverse direction
' u' = the speed of b as counted from mass a
' the graph is finished when ua=~1c and ub=-~1c 
' on x you have the speed of objects, on y the speed of b as counted from a
' remember that in Newton the final y-value is ~2c
plot 0,0.999999999 use ( (x*c*2) / (1+x^2) ) / c
pause
