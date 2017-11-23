rem recherche
Rem reg1s roum1gu1eres 2017
view (10,470,10,600)
X=xmax/2
y=ymax/2
c=36
p=2
d=xmax/4
d_dir=-1
while 1
  cls
  for a=1 to 1200 'originally 360
    pset (0.06) (x+d*cos((a/150) -345)*sin(a/c)),  (y+d*sin(360-(a/p))*cos(a/180)) color 3
    pset (0.06) (x+d*cos((a/147.27)-345)*sin(a/c)),(y+d*sin(360-(a/p))*cos(a/150)) color 3
    pset (0.06) (x+d*cos((a/144)-345)*sin(a/c)),   (y+d*sin(360-(a/p))*cos(a/147.27)) color 3
    pset (0.06) (x+d*cos((a/140)-345)*sin(a/c)),   (y+d*sin(360-(a/p))*cos(a/144)) color 3
    pset (0.06) (x+d*cos((a/135)-345)*sin(a/c)),   (y+d*sin(360-(a/p))*cos(a/140)) color 3
    pset (0.06) (x+d*cos((a/128.5)-345)*sin(a/c)), (y+d*sin(360-(a/p))*cos(a/135)) color 3
    'pset (0.06) (x+d*cos((a/120)-345)*sin(a/c)),   (y+d*sin(360-(a/c))*cos(a/120)) color 6
    pset (0.06) (x+d*cos((a/90)-345)*sin(a/c)),    (y+d*sin(360-(a/p))*cos(a/108)) color 3
    pset (0.06) (x+d*cos((a/60)-345)*sin(a/c)),    (y+d*sin(360-(a/p))*cos(a/90)) color 3
    pset x+(d*cos(a/359)*sin(a/c)),  y+(d*sin(a)*cos(a/p)*sin(a/359)) color 7
    pset x-(d*cos(a/359)),           y+(d*sin(a)*cos(a/359)) color 7
    'pset x+d*cos(a/359),             y-d*sin(a/c)*cos(a/359) color 2
    pset x-(d*cos(a/359)*sin(a/32)), y+(d*sin(a)*cos(a/c+c)*sin(a/359)) color 7
    pset x-d*cos(a/24)*sin(a/32),    y+d*sin(a)*cos(a/8)*sin(a) color 7
  next a
  showpage
  p = 1 + (p + 1) mod 400
  c = 1 + (c + 1) mod 200
  d += d_dir
  if (d < -1200) then 
    d_dir = 1
  else if (d > 1200) then
    d_dir = -1
  endif
wend  
pause