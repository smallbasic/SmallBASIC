dim poly()
dim m3(2,2)

xpos1 = ymax * 0.2
xpos2 = ymax * 0.4
xpos3 = ymax * 0.6
rscle = ymax * 0.1

poly << [1, 1]
poly << [1, -1]
poly << [-1, -1]
poly << [-1, 1]
poly << [1, 1]

polyext poly, x1, y1, x2, y2
? "PolyExt  ",x1, y1, x2,y2
? "PolyArea ",polyarea(poly)
drawpoly poly, xpos1, xpos1, rscle
drawpoly poly, xpos2, xpos2, rscle filled
m3ident m3
m3rotate m3,rad(45)
m3apply m3, poly
drawpoly poly, xpos3, xpos1, rscle


