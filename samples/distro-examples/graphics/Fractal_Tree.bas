#!/usr/local/bin/sbasic -g
' tree

randomize timer

dim si(10),sx(10),sy(10),st(10)
dim shf(10),sa(10),sb(10),sh(10)
dim sd(10)

' TREE DATA
rand=1
if ymax > 320 then
   ' UNIX
   brafan=ymax/5
   inpfac=.75
   inphf=.8
   depth=6
   density=5
else
  ' PalmOS
  brafan=xmax/3.2
  inpfac=1
  inphf=1
  depth=4
  density=3
fi

'
l=pi/180
a=90:h=ymax/4:nx=xmax/2:ny=1

cls
if bpp>4
  flowercolor=12
else
  flowercolor=10
fi
color 2
pset xmax/2, ymax
gosub tree
end

LABEL tree
if depth=0
  ' flower
  color flowercolor
  pset nx-2,ymax-ny
  pset nx,ymax-(ny+3)
  pset nx+2,ymax-ny
  pset nx,ymax-ny
  color 2
else
  si(sp)=i:sx(sp)=x:sy(sp)=y
  st(sp)=th:shf(sp)=hf
  sp=sp+1
  start=a-brafan/2
  th=brafan/density
  if depth<=2
    hf=inphf/2
  else
    hf=inphf
  fi
  x=h*cos(a*l):y=h*sin(a*l)
  a=start
  i=1
500
  nx=nx+x:ny=ny+y
  line nx,ymax-ny
  gosub brrand
  gosub tree
  a=sa(sp):brafan=sb(sp)
  h=sh(sp):depth=sd(sp)
  a=a+th
  nx=nx-x:ny=ny-y
  pset nx,ymax-ny
  i=i+1
  if i<=density+1 then 500
  sp=sp-1
  i=si(sp):x=sx(sp):y=sy(sp)
  th=st(sp):hf=shf(sp)
  brafan=sb(sp)
fi
return
 
LABEL brrand
sa(sp)=a:sb(sp)=brafan
sh(sp)=h:sd(sp)=depth
if rand=0
  brafan=brafan*inpfac
  h=h*hf
else
  a=a-th/2+rnd*th+1
  brafan=brafan*inpfac
  h=h*hf*rnd
fi
depth=depth-1
return
