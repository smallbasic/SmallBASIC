#!/usr/local/bin/sbasic
' MATRICES
? cat(1);"MATRICES";cat(0)
?
A = [ -6; 2; -7]
B = [  5; 4; -6]
print A; " + "; B; " = "; A + B
print A; " - "; B; " = "; A - B
print A; " * "; .5; " = "; A * .5
print "A = ";A; ", -A = "; -A
a=[1,-1,1;2,-1,2;3,2,-1]
print "A = ";A; ", A^-1 = "; inverse(A)

?
A = [ -2; 3]
B = [  7, 6, -3, 5]
print A; " * "; B; " = "; A * B

?
A = [  2, 4; -1, -2]
B = [ -2, 2, 4; 1, -1, -2]
print A; " * "; B; " = "; A * B

?
A = [ -3,  0; 2, -1]
B = [  4, -2; 3,  5]
print A; " * "; B; " = "; A * B

?
? "Solve this:"
? "  5x - 2y + 3z = -2"
? " -2x + 7y + 5z =  7"
? "  3x + 5y + 6z =  9"
? 
A = [ 5, -2, 3; -2, 7, 5; 3, 5, 6]
B = [-2; 7; 9]
'? A; " * X = "; B
'?
C=LinEqn(a,b)
print "[x; y; z] = "; C
?

