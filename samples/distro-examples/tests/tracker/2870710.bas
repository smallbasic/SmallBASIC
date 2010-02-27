'
'http://sourceforge.net/tracker/?func=detail&aid=2870710&group_id=22348&atid=375102
'

'The for next iteration ends sometimes prematurely when using a floating
'point step. For example the next code

for i = 0 to n step .1 : ? i + " "; : next

'ends for each "n" as follows:
'0, 0.9, 1.9, 3, 4, 5, 6, 7, 8, 9, 9.9, 10.9, 11.9
'The n.9 values are not correct when compared to the behaviour of the for
'next iteration when using integer values i.e. the iteration is executed up
'to and including the n value
