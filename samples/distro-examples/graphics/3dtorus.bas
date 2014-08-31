'app-plug-in
'menu Games/3D Torus

' 10/5/04 6:55:40 Relsoft
' 3d torus with backface culling
' Jelly 2004
' Http://rel.betterwebber.com

const Xmax = 640
const Ymax = 480
const Xmid = Xmax/2
const Ymid = Ymax/2
const Lens = 256
const PI = 3.141593
const rings = 14
const bands = 15
const ringradius = 100
const bandradius = 40
'x         0
'y          1
'z          2
'screenx    3
'screeny    4
randomize ticks
MaxPoint = Rings * Bands
dim model(4, MaxPoint-1)   '3d points

'p1         0
'p2         1
'p3         2
'index      3
'color      4
dim poly(4, 1)         'polygons

LoadTorus Rings, Bands, RINGRADIUS, BandRadius, Model()
Tesselate Rings, Bands, Poly()

anglex = int(rnd*360)
angley = int(rnd*360)
anglez = int(rnd*360)

cls

repeat
    anglex = (anglex + 1) mod 360
    angley = (angley + 1) mod 360
    anglez = (anglez + 1) mod 360
    RotateAndProject Model(), anglex, angley, anglez
    DrawModel Model() ,  Poly()
    SHOWPAGE
until

end


'//*********************************************************************
SUB DrawModel (byref Model() , byref Poly())
    cls
    FOR I = 0 TO UBOUND(Poly,2)
        J = Poly(3,I)
        x1 = Model(3,Poly(0,J))       'Get triangles from "projected"
        x2 = Model(3,Poly(1,J))       'X and Y coords since Znormal
        x3 = Model(3,Poly(2,J))       'Does not require a Z coord
        y1 = Model(4,Poly(0,J))       'V1= Point1 connected to V2 then
        y2 = Model(4,Poly(1,J))       'V2 to V3 and so on...
        y3 = Model(4,Poly(2,J))

        'Use the Znormal,the Ray perpendicular(Orthogonal) to the Screen
        'Defined by the Triangle (X1,Y1,X2,Y2,X3,Y3)
        'if Less(<) 0 then its facing in the opposite direction so
        'don't plot. If =>0 then its facing towards you so Plot.
        Znormal = (x2 - x1) * (y1 - y3) - (y2 - y1) * (x1 - x3)
        c = poly(4,J)
        IF Znormal >= 0 THEN
            line x1, y1, x2, y2, color c
            line x2, y2, x3, y3, color c
            line x3, y3, x1, y1, color c
        END IF
    NEXT I
end sub

'//*********************************************************************
SUB LoadTorus (Rings, Bands, RINGRADIUS, BandRadius, byref Model())

MaxPoint = Rings * Bands

A1 = 2 * PI / Rings: A2 = 2 * PI / Bands
i = 0
FOR S2 = 0 TO Bands - 1
    FOR S1 = 0 TO Rings - 1
        X1 = COS(S1 * A1) * RINGRADIUS
        Y1 = SIN(S1 * A1) * RINGRADIUS
        Model(0,i) = X1 + COS(S1 * A1) * COS(S2 * A2) * BandRadius
        Model(1,i) = Y1 + SIN(S1 * A1) * COS(S2 * A2) * BandRadius
        Model(2,i) = SIN(S2 * A2) * BandRadius
        i = i + 1
    NEXT S1
NEXT S2

end sub

'//*********************************************************************
SUB RotateAndProject (byref Model(), AngleX, AngleY, AngleZ)
''Right handed system
''when camera components increase:
''x=goes right
''y=goes down
''z goes into the screen

'''rotation: counter-clockwise of each axis
''ei.  make yourself perpenicular to the axis
''wave your hand from the center of your body to the left.
''That's how it rotates. ;*)


'Precalculate the SIN and COS of each angle
ax= anglex*PI/180
ay= angley*PI/180
az= anglez*PI/180

cx = cos(ax)
sx = sin(ax)
cy = cos(ay)
sy = sin(ay)
cz = cos(az)
sz = sin(az)


xx = cy * cz
xy = sx * sy * cz - cx * sz
xz = cx * sy * cz + sx * sz

yx = cy * sz
yy = cx * cz + sx * sy * sz
yz = -sx * cz + cx * sy * sz

zx = -sy
zy = sx * cy
zz = cx * cy

FOR i = 0 TO UBOUND(Model,2)

        x = Model(0,i)
        y = Model(1,i)
        z = Model(2,i)
       
        RotX = (x * xx + y * xy + z * xz)
        RotY = (x * yx + y * yy + z * yz)
        RotZ = (x * zx + y * zy + z * zz)

        'Project
        Distance = (LENS - RotZ)
        IF Distance > 0 THEN
            Model(3,i) = Xmid + (LENS * RotX / Distance)
            Model(4,i) = Ymid - (LENS * RotY / Distance)
        ELSE

        END IF
NEXT i

END SUB

sub Tesselate(Rings, Bands,Byref Poly())
    I = 0
    MaxTri = 0
    FOR S1 = Bands - 1 TO 0 STEP -1
        FOR S2 = Rings - 1 TO 0 STEP -1
            I = I + 1
            MaxTri = MaxTri + 1
            I = I + 1
            MaxTri = MaxTri + 1
        NEXT S2
    NEXT S1

    REDIM Poly(4,MaxTri)

    I = 0
    FOR S1 = Bands - 1 TO 0 STEP -1
        FOR S2 = Rings - 1 TO 0 STEP -1
            Poly(0,I) = S1 * Rings + S2
            Poly(1,I) = S1 * Rings + (S2 + 1) MOD Rings
            Poly(2,I) = (S1 * Rings + S2 + Rings) MOD MaxPoint
            Poly(3,I) = I
            Poly(4,I) = int(rnd*15)
            
            I = I + 1
            Poly(0,I) = S1 * Rings + (S2 + 1) MOD Rings
            Poly(1,I) = (S1 * Rings + (S2 + 1) MOD Rings + Rings) MOD MaxPoint
            Poly(2,I) = (S1 * Rings + S2 + Rings) MOD MaxPoint
            Poly(3,I) = I
            Poly(4,I) = int(rnd*15)
            
            I = I + 1
        NEXT S2
    NEXT S1
end sub
