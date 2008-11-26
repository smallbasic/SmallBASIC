'app-plug-in
'menu Games/Star Field

'10/4/04 3:22:23 PM
'3d projection implemented in starfield with z-axis rotate
'my first ever prog made in small basic
'jelly 2004
'http://rel.betterwebber.com
'x     0           'array subscripts
'y     1
'z     2         
'zvel  3         'velocity
'ox   4         'old x
'oy    5
const MAXSTARS = 200               'number of stars
const LENS = 256               
const XMAX = 640
const YMAX = 480
const XMID = XMAX/2
const YMID = YMAX/2
const PI = 3.141593

dim stars(5,MAXSTARS)
dim colors(2, 255)
for i =0 to 255             'a lil hack I made to simulate 256
   colors(0, i) = i         'colors in 24 bit?
   colors(1, i) = i
   colors(2, i) = i
next i

for i = 0 to MAXSTARS
   stars(0, i) = -XMID/2 + INT(RND * XMID)
   stars(1, i) = -YMID/2 + INT(RND * YMID)
   stars(2, i) = INT(RND * LENS)
   stars(3, i) = 2 ' + INT(RND * 5)
   
next i
 
rect 0, 0, XMAX,YMAX, color 0 filled
repeat

        '///Move the stars
        '//Z=0 is 256 units away from the screen
        '//Adding values to Z moves the pixel towards us
        '//if Z > 256, the star is over our screen so reinitialize
        '//the stars Z value to 0(256 units away).
        angleZ = angleZ + 1
        rangleZ = angleZ * PI / 180            'convert to radians
        cosz = COS(rangleZ)                   'precalc
        sinz = SIN(rangleZ)

        FOR i = 0 TO MAXSTARS
            Stars(2,i) = Stars(2,i) + Stars(3,i)    'move it
            IF stars(2,i) > 255 THEN                'check for camera LENS
                stars(2,i) = 0                     'ReInit Z value
            END IF
           

            ox = stars(4,i)
            oy = stars(5,i)
            if ox>-1 and ox<XMAX-1 then
            if oy>-1 and oy<YMAX-1 then
               rect ox, oy, ox+1, oy+1, color 0      'erase
            end if
         end if
            tsx = Stars(0,i)                        'StarX
            tsy = Stars(1,i)                        'cleans the projectioon
            sz = Stars(2,i)                        'algo. ;*)
           
            sx = (tsx * cosz) - (tsy * sinz)         'Z-axis rotate
            sy = (tsy * cosz) + (tsx * sinz)


            Distance = (LENS - sz)                'get Distance

            IF Distance THEN                       'if dist>0 then
                'Projection formula
                x = XMID + (LENS * sx / Distance)
                y = YMID - (LENS * sy / Distance)
            Stars(4,i) = x
            Stars(5,i) = y
            ELSE
                                        'do nothing
                                        'you wouldn't wan't to
                                        'divide by 0 would ya? :*)

            END IF
         
         if x>-1 and x<XMAX-1 then
            if y>-1 and y<YMAX-1 then
               clr = (int(sz)) & 255
               r = colors(0, clr)
               g = colors(1, clr)
               b = colors(2, clr)
               c = rgb(r, g, b)
                  rect x, y, x+1, y+1, c      
            end if
         end if
        NEXT i
until

