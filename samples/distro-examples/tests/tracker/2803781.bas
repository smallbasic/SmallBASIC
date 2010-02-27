'
'http://sourceforge.net/tracker/?func=detail&aid=2803781&group_id=22348&atid=375102
'

'The "aspect" in the circle command is not working correctly beyond a
'certain value of r.

w=3000 : window -w,w,w,-w : line -w,0,w,0 : line 0,-w,0,w
r = 497 : circle 0,0, r : circle 0,0, r, 3 : circle 0,0, r, 0.333 ' correct shapes
r = 900 : circle 0,0, r, 3 ' incorrect shape
r = 1480 : circle 0,0, r, 0.333 ' incorrect shape

