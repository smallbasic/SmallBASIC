'
'http://sourceforge.net/tracker/?func=detail&aid=1651510&group_id=22348&atid=375102
'I realized in the t.jpg I could find just the second image, the first one was overwritten.
'

open "t.jpg" for output as #1
image #1,0,0,0,10*width,10*height
image #1,0,10*width,10*height,10*width,10*height
close #1

