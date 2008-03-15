'
func f(t,@x,@y)
  x=t*cos(t)
  y=t*sin(t)
' z=t
end

plot3 seq(0, 50, xmax) use f(x,y)
