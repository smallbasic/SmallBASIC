'
func fz(x,y)
  fz=sin(x)*cos(y)
end

x=seq(-6.75,6.75,xmax)
y=x

plot3 x, y use fz(x,y)
