'
'http://sourceforge.net/tracker/?func=detail&aid=2857384&group_id=22348&atid=375102
'

'This defect pertains to version FLTK 0.10.5 ( the Group field doesn't allow
'you to select this value yet)

'The parameters for red and blue are interchanged In the RGB(r, g, b) and
'RGBF(r, g, b) functions

color 15, rgb(128, 0, 0) 

'results in white foreground on blue background,
'this should be white foreground on red background

color 15, rgb(0, 0, 128) 

'results in white foreground on red background,
'this should be white foreground on blue background'

'
' fix this at the same time since it will be a problem with AnsiWidget.cxx:
'
'I get weird results when i use the tab() function inside a print #1
'statement. For example: print x;tab(10);y works fine, but when i do
'this print #1; x;tab(10);y it puts escape codes inbetween x and y
'instead of a space.

