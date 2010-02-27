'
'http://sourceforge.net/tracker/?func=detail&aid=2293396&group_id=22348&atid=375102
'
'SB 10.1 mouse broken
'
'On both XP and Vista the pen command to read the mouse does not function
'correctly when running SmallBASIC v10.1. (note SB9.7 running on XP and
'Vista does not have these problems).
'
'1). The coordinates returned for the pen(1) and pen(2) arereferenced to the
'editor window not the output window.
'A work around for this problem is to position the editor window at the top
'left of the physical screen then run the program and move the output window
'to the same position.
'
'2). The mouse infomation is only transfered to the program when the mouse
'is moving
'the button presses are ignored.
'An unexplainable work around to this problem is to hold the shift key down
'at all
'times tnen the button presses are forwarded to the program.
'Both these problems can be seen by running the demo program sudoku.bas.
'
'3) In both SB9.7 and SB10.1 the pen(1,2,4,5,10,11) commands do not function
'as
'documented. In all cases the pen commands return the current position, I do
'not
'consider this a big problem and it can easily be overcome in the program.
'
'4) The commands for pen(12) and pen(13) return a very large number fro true
'1 would
'be better, again this is not a big problem.
'
'A test program follows
'----------------------------
'tests the mouse position ie pen command

zz=0
qq=0
rr=0

repeat
  pen on
  repeat
    zz = pen(0)
    locate 6,5
    qq=qq+1
    print "loop no ",qq
    locate 7,1
    print "pen 0 1 2 3 4 5 10 11 12 13 14"
    locate 8,5
    print ""
    locate 8,5
    print zz,pen(1),pen(2),pen(3),pen(4),pen(5),pen(10),pen(11),pen(12),pen(13),pen(14)
  until zz

  pen off

  rr=rr+1
  print "running progran out of loop ",rr
  locate pen(2)/15,pen(1)/7
  print "*"
  print
  
until 0

