#!/usr/bin/sbasic -g

' vectors for wireframe
dim z0(3), z2(3), zc(3)

' vectors at Z=0
z0(0) = [ [0, 0, 0], [2, 0, 0] ]
z0(1) = [ [0, 0, 0], [0, 2, 0] ]
z0(2) = [ [2, 2, 0], [0, 2, 0] ]
z0(3) = [ [2, 2, 0], [2, 0, 0] ]

' vectors at Z=2
z2(0) = [ [0, 0, 2], [2, 0, 2] ]
z2(1) = [ [0, 0, 2], [0, 2, 2] ]
z2(2) = [ [2, 2, 2], [0, 2, 2] ]
z2(3) = [ [2, 2, 2], [2, 0, 2] ]

' connections
zc(0) = [ [0, 0, 0], [0, 0, 2] ]
zc(1) = [ [2, 0, 0], [2, 0, 2] ]
zc(2) = [ [2, 2, 0], [2, 2, 2] ]
zc(3) = [ [0, 2, 0], [0, 2, 2] ]

' setup world
d3world 0,0,0, 2,2,2

' setup viewpoint
d3vpp 10,10,10, 1.2
'd3vpa 10,0,0,0, 1.2

' keys-loop

end

' draw vectors
sub drawvecs(@v)
end

