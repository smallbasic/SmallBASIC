'app-plug-in
'menu Games/Chess

' Chess program

' Written by KC Goldberg and Sons, November, 2004
' Written in SmallBASIC for Windows, 0.9.5.2

' This program was written as a teaching example for me and my sons
' We used SmallBASIC mostly because it is cool and free. For this particular
' application, the array structure (including nested arrays) and the ability
' of functions to return an array make this almost LISP-like, which is perfect
' for turn-based games and the MiniMax algorithm

' In addition to the rich array strucure, there is a simple mechanism to
' capture both console input and mouse clicks, and a very accessible
' graphics primitives package.

' Finally, although I have not done this, the ability to port this to my
' PalmPilot is just too cool!

' The program here will either ask the user to make a move using a
' "click on the board" interface, or generate one itself using the
' MiniMax algorithm.

' Due to performance problems, this game is just too slow to
' use for good computer AI. On my fast system, a level 2 ply depth is
' barely playable

' Also, in many places the code is far from elegant. I would like to blame
' a lot of that on the complications of the rules of chess, making it quite
' difficult to generate generalized rules and routines, and some of it on
' my not spending the time to tighten it up.

' As near as I can tell, the code is correct. However, this has not
' been 'professionally' tested. In about 20 games, I have only seen
' legal moves allowed. You mileage may vary!

' =====================================================================
' Data types:
' Board: Array Squares x Squares of pieces
'   B(1,1) = lower left
'   B(Squares,Squares) = upper right
' BoardSet: Array of 4 arrays
'   1st is a Board
'   2nd is a 6-value array of true-false values:
'       Black Rook/King/Rook, White Rook/King/Rook
'       This value is True if they have not been
'       moved, and False once the appropriate piece has been moved
'       This is used to see if castling is still possible
'   3rd is the last Move made in the board (to check for en passant)
'   4th is a KingPosition array; although we could just search the BOARD for
'       the King, since the King position is used repeatedly to detect the
'       CHECK status, it saves time to keep track of the Kings separately
' KingPosition: Array{Black .. White] of (x,y) : Lists current King positions
' Side: [Black, White] ; -1 for Dark, 1 for light
' Piece: Integer; see constants below for piece each value represents
'   (Side * Piece) denotes color of piece
' Move: Array of 4x1 listing coordinates (from, to) e.g. [x1,y1,x2,y2]
'   Castling is denoted by the KING move (the king ONLY moves 2 squares
'   during castling, although rooks may move two/three moves with or without
'   a castle
' MoveArray: An array of Moves, e.g.[[1,1,2,2],[1,2,1,4],[8,2,8,3]]
' MiniMaxResult: [Score, Move] where score is a numeric board evaluation Score

' ===========================================================================
' Procedures:

' DisplayBoard (BoardSet)   : Display a board
' Main : Main procedure loop

' Functions:

' MakeMove (BoardSet, MoveList, ShowMoves) = BoardSet   : Apply a MoveList to a Board
'   If ShowMoves = TRUE then display the moves as we make them
'   This procedure will also promote pawns to queesn if appropriate, manage castling,
'   and keep the components of the BoardSet updated
'   If does NOT do error checking - be sure the move passed is correct/legal!
' GenerateMoveList (BoardSet, Side) = MoveArray : Generate list of all valid moves for Side
' GetUserMove (BoardSet, Side) = Move : Ask user to move for Side
' InitBoard = BoardSet : Return a new board with pieces in initial position
' Func MiniMax(BoardSet, Side, Depth, DepthMax, Evaluator) = MiniMaxResult
'   : Perform MiniMax algorithm search to find best move for Side (see declaration
'   for description of all parameters)
' EvaluateBoard (BoardSet, Side, Evaluator) = Score : Return a numeric score with positive
'   numbers favoring Side using the evaluation function specified by Evaluator
'   This should only be called from within MiniMax; technically I should probably make
'   it a private function of Minimax, but as it too has private functions, that seemed
'   like a lot of overhead for Minimax, which will be called recursively
' DetermineMove (BoardSet, Side) = Move : Use the method stored in PlayerStrategy(Side)
'   to determine the next move for Side to make
' IsCheck(Boardm KingPosition, Side) = Boolean : Returns True if side SIDE is in check, else false
'

' ===================================================================
' ===================================================================
' Constants

' Size of the board (default = 8 for standard board - don't change this!)
Const Squares = 8


' These constants denote what occupies a piece on any board square
Const Pawn = 1
Const Bishop = 2
Const Knight = 3
Const Rook = 4
Const Queen = 5
Const King = 6

Const Blank = 0

' This defines the sides
Const Black = -1
Const White = 1

' Figure out how big the boxes should be for graphical display, in pixels
' (board will be square)
Const BoxSize = (Min(xmax, ymax - 3 * TextHeight ("TO MOVE")) / Squares)

' Colors for display

' Color of board squares (background)
Const ColorDarkSquare = 8   ' Dark gray
Const ColorLightSquare =7   ' Light gray
Const ColorSelect = 1       ' Deep Blue
Const ColorSelectFinal = 9  ' Bright blue
Const ColorCheck = 12       ' Bright red

' Color of pieces on squares
Const ColorBlack = 0        ' Black
Const ColorWhite = 15       ' White
const thing = ''
' =========================================================================
' =========================================================================

' -------------------------------------------------------------------------
' -------------------------------------------------------------------------
' This will Generate a BoardSet to the game starting values
Func InitBoard

    ' Variables we will be using
    Local x, y, Board, CanCastle, KingPosition

    Dim Board (1 to Squares, 1 to Squares)

    ' Initially, rooks and kings for both sides have not been moved
    '            BR    BK    BR    WR    WK    WR
    CanCastle = [True, True, True, True, True, True]

    ' List the initial position of the Black and White Kings
    Dim KingPosition(Black to White,1)
    KingPosition(Black,0) = 5
    KingPosition(Black,1) = 8

    KingPosition(White,0) = 5
    KingPosition(White,1) = 1

    For x = 1 to Squares

        ' Second, second-to-last rows are Pawns
        Board (x, 2) = Pawn * White
        Board (x, 7) = Pawn * Black

        ' Middle squares are Blank
        For y = 3 to 6
            Board (x, y) = Blank
        Next y
    Next x

    ' Now, hand-code in first and last row
    Board (1,1) = Rook * White      :   Board (1,8) = Rook * Black
    Board (2,1) = Knight * White    :   Board (2,8) = Knight * Black
    Board (3,1) = Bishop * White    :   Board (3,8) = Bishop * Black
    Board (4,1) = Queen * White     :   Board (4,8) = Queen * Black
    Board (5,1) = King * White      :   Board (5,8) = King * Black
    Board (6,1) = Bishop * White    :   Board (6,8) = Bishop * Black
    Board (7,1) = Knight * White    :   Board (7,8) = Knight * Black
    Board (8,1) = Rook * White      :   Board (8,8) = Rook * Black

    ' Return type BoardSet
    InitBoard = [Board, CanCastle, [0,0,0,0], KingPosition]
End

' -------------------------------------------------------------------------
' -------------------------------------------------------------------------

' This routine will display a graphic square with the checkers pieces
' kept in board B
Sub DisplayBoard (BoardSet)

    ' Variables we will be using
    ' x and y for board coordinates
    Local x, y, Board, PieceColor


    ' ----------------------------------------------------------------
    ' Subroutines to draw specific pieces at coordinates (x,y) for
    ' Side

    Sub DrawPawn (x, y, FillColor)
        ' This is a triangle on top of a circle

        Local bx, by, PolyArray

        bx = BoxSize * (x - 1)
        by = BoxSize * (Squares + 1 - y)

        ' The triangle base will be 1/3 square width
        ' the triangle height will be 1/2 square height

        PolyArray = [[bx + BoxSize / 3, by], &
        [bx + BoxSize / 2, by - BoxSize / 2], &
        [bx + 2 * BoxSize / 3, by]]

        DrawPoly PolyArray Color FillColor Filled

        ' Draw Circle on top of triangle, with radius = 1/8 window height
        Circle (bx + BoxSize / 2), (by - BoxSize / 2), BoxSize / 8, &
        1, FillColor Filled

    End ' Sub DrawPawn

    Sub DrawRook (x, y, FillColor)
        Local bx, by, dx, dy, PolyArray

        bx = BoxSize * (x - 1)
        by = BoxSize * (Squares + 1 - y)

        dx = BoxSize / 7
        dy = BoxSize / 6

        PolyArray = [ [bx + 2 * dx, by - 0 * dy], &
        [bx + 2 * dx, by - 3 * dy], &
        [bx + 1 * dx, by - 3 * dy], &
        [bx + 1 * dx, by - 5 * dy], &
        [bx + 2 * dx, by - 5 * dy], &
        [bx + 2 * dx, by - 4 * dy], &
        [bx + 3 * dx, by - 4 * dy], &
        [bx + 3 * dx, by - 5 * dy], &
        [bx + 4 * dx, by - 5 * dy], &
        [bx + 4 * dx, by - 4 * dy], &
        [bx + 5 * dx, by - 4 * dy], &
        [bx + 5 * dx, by - 5 * dy], &
        [bx + 6 * dx, by - 5 * dy], &
        [bx + 6 * dx, by - 3 * dy], &
        [bx + 5 * dx, by - 3 * dy], &
        [bx + 5 * dx, by - 0 * dy] ]

        DrawPoly PolyArray Color FillColor Filled

    End ' Sub DrawRook

    Sub DrawBishop (x, y, FillColor)
        ' This is a triangle on top of a circle, but bigger than a pawn
        ' and with a visor slit in the circle

        Local bx, by, PolyArray

        bx = BoxSize * (x - 1)
        by = BoxSize * (Squares + 1 - y)

        ' The triangle base will be 1/3 square width
        ' the triangle height will be 2/3 square height

        PolyArray = [[bx + BoxSize / 3, by], &
        [bx + BoxSize / 2, by - 2 * BoxSize / 3], &
        [bx + 2 * BoxSize / 3, by]]

        DrawPoly PolyArray Color FillColor Filled

        ' Draw Circle on top of triangle, with radius = 1/6 window height
        Circle (bx + BoxSize / 2), (by - 2 * BoxSize / 3), BoxSize / 6, &
        1, FillColor Filled

        ' Use a polygon to make the slit
        ' Now, add a slit for the visor
        PolyArray = [[bx, by -  BoxSize + BoxSize * 1/8], &
        [bx + BoxSize / 2, by - 2 * BoxSize /3], &
        [bx + BoxSize * 1/8, by - BoxSize]]

        ' Check Background color of square
        If (x+y) Mod 2 = 1 Then
            FillColor = ColorLightSquare
        Else
            FillColor = ColorDarkSquare
        EndIf

        DrawPoly PolyArray Color FillColor Filled

    End ' Sub DrawBishop

    Sub DrawKnight (x, y, FillColor)
        Local bx, by, PolyArray

        bx = BoxSize * (x - 1)
        by = BoxSize * (Squares + 1 - y)

        ' Draw triangle as the base of the knight

        PolyArray = [ [bx + 1 * BoxSize / 3, by], &
        [bx + 3 *BoxSize / 4, by], &
        [bx + 3 * BoxSize / 4, by -  5 * BoxSize / 6] ]

        DrawPoly PolyArray Color FillColor Filled

        ' Draw Rectangle for the snout

        PolyArray = &
        [[bx + 1*BoxSize/3, by - 2*BoxSize/3 - 2*BoxSize/20], &
        [bx + 3*BoxSize/4, by - 2*BoxSize/3 - 2*BoxSize/20], &
        [bx + 3*BoxSize/4, by - 2*BoxSize/3 + 3*BoxSize/20], &
        [bx + 1*BoxSize/3, by - 2*BoxSize/3 + 3*BoxSize/20]]

        DrawPoly PolyArray Color FillColor Filled

        ' Place an eye in the center (circle)
        If (x+y) Mod 2 = 0 Then
            FillColor = ColorDarkSquare
        Else
            FillColor = ColorLightSquare
        EndIf
        Circle (bx + 3*BoxSize/4 - 2* BoxSize / 20),(by - 2*BoxSize/3), BoxSize / 20, &
        1, FillColor Filled

    End ' Sub DrawKnight

    Sub DrawKing (x, y, FillColor)
        ' This basically draws a cross on top of a triangular base
        ' Triangle is same dimensions as queen

        Local bx, by, PolyArray

        bx = BoxSize * (x - 1)
        by = BoxSize * (Squares + 1 - y)

        ' The triangle base will be 1/2 square width
        ' the triangle height will be 2/3 square height

        PolyArray = [[bx + BoxSize / 4, by], &
        [bx + BoxSize / 2, by - BoxSize + BoxSize / 3], &
        [bx + 3 * BoxSize / 4, by]]

        DrawPoly PolyArray Color FillColor Filled

        ' Now, draw two rectangles crossing the center
        Rect (bx + BoxSize / 4), (by - BoxSize + BoxSize / 3 - BoxSize / 15), &
        (bx + 3 * BoxSize / 4), (by - BoxSize + BoxSize / 3 + BoxSize / 15), &
        Color FillColor Filled
        Rect (bx + BoxSize / 2 - BoxSize / 15), (by - BoxSize + BoxSize / 15), &
        (bx + BoxSize / 2 + BoxSize / 15), (by - BoxSize + 5 * BoxSize / 8), &
        Color FillColor Filled

    End ' Sub DrawKing

    Sub DrawQueen (x, y, FillColor)

        ' Triangle with a circle on top, but bigger than pawn or
        ' bishop

        Local bx, by, PolyArray

        bx = BoxSize * (x - 1)
        by = BoxSize * (Squares + 1 - y)

        ' The triangle base will be 1/2 square width
        ' the triangle height will be 2/3 square height

        PolyArray = [[bx + BoxSize / 4, by], &
        [bx + BoxSize / 2, by - BoxSize + BoxSize / 3], &
        [bx + 3 * BoxSize / 4, by]]

        DrawPoly PolyArray Color FillColor Filled

        ' Draw Circle on top of triangle, with radius = 1/8 window height
        Circle (bx + BoxSize / 2), (by - BoxSize + BoxSize / 4), BoxSize / 5, &
        1, FillColor Filled

    End ' Sub DrawQueen

    ' ----------------------------------------------------------------
    ' Function begins here

    ' Clear screen
    Cls

    ' Extract the board
    Board = BoardSet(0)

    ' Draw the background squares
    For x = 1 to Squares
        For y = 1 to Squares
            Rect (x-1)*BoxSize, (Squares-y)*BoxSize, &
            x*BoxSize, ((Squares + 1)-y)*BoxSize, &
            Color IF((x+y) Mod 2 = 0, ColorDarkSquare, ColorLightSquare) &
            Filled
        Next y
    Next x

    For x = 1 to Squares
        For y = 1 to Squares

            If Sgn(Board(x,y)) = Black Then
                PieceColor = ColorBlack
            Else
                PieceColor = ColorWhite
            EndIf

            If Abs(Board(x,y)) = Pawn then
                DrawPawn x, y, PieceColor
            ElseIf Abs(Board(x,y)) = Rook then
                DrawRook x, y, PieceColor
            ElseIf Abs(Board(x,y)) = Knight then
                DrawKnight x, y, PieceColor
            ElseIf Abs(Board(x,y)) = Bishop then
                DrawBishop x, y, PieceColor
            ElseIf Abs(Board(x,y)) = King then
                DrawKing x, y, PieceColor
            ElseIf Abs(Board(x,y)) = Queen then
                DrawQueen x, y, PieceColor
            EndIf
        Next y
    Next x  
End ' Sub DisplayBoard

' -------------------------------------------------------------------------
' -------------------------------------------------------------------------

' This function is passed a BoardSet and a Move and returns the
' BoardSet after that move is made
' If ShowMoves = True, then display intermediate moves and play
' appropriate tunes, allowing the user to follow along
Func MakeMove(BoardSet, Move, ShowMoves)

    ' Piece is the type of piece we are making
    Local Piece, Board, CanCastle, KingPosition

    Board = BoardSet(0)

    ' Used to see if castling is still allowed after this move
    CanCastle = BoardSet(1)

    ' Keep track of King Positions
    KingPosition = BoardSet(3)

    ' Record initial piece
    Piece = Board(Move(0), Move(1))

    ' Check for en passant - have to capture the pawn left behind
    If Abs(Piece) = Pawn Then
        ' Show that we are moving diagnonally - only happens with a capture
        If Abs(Move(2) - Move(0)) = 1 Then
            If Board(Move(2), Move(3)) = Blank Then
                ' There SHOULD be a pawn to delete one square toward center -
                ' pawns should not be capturing a blank square except during
                ' en passant
                Board(Move(2), Move(3) - Sgn(Piece)) = Blank
            EndIf
        EndIf
    EndIf

    ' Move that piece to the target
    Board (Move(2), Move(3)) = Piece

    ' Blank out original score
    Board (Move(0), Move(1)) = Blank

    ' Check for promotion - here, we will only allow promotion to
    ' a Queen - consider revision in the future
    If Abs(Piece) = Pawn And (Move(3)=1 Or Move(3)=Squares) Then
        Board(Move(2), Move(3)) = Queen * Sgn (Piece)
    EndIf

    ' If we are moving a Rook, then we cannot castle on that side
    ' in the future
    If Abs(Piece) = Rook Then
        If Sgn(Piece) = White Then
            If Move(0)=1 And Move(1)=1 Then
                ' Queen rook
                CanCastle(3) = False
            ElseIf Move(0)=8 And Move(1)=1 Then
                ' King rook
                CanCastle(5) = False
            EndIf
        ElseIf Sgn(Piece) = Black Then
            If Move(0)=1 And Move(1)=8 Then
                ' Queen rook
                CanCastle(0) = False
            ElseIf Move(0)=8 And Move(1)=8 Then
                ' King Rook
                CanCastle(2) = False
            EndIf
        EndIf
    EndIf

    ' Check for castle - we'll have to assume here the move is legal
    ' And, if we castle, then we cannot castle again ...
    ' Actually, if we just move the King (castling or not), we cannot
    ' castle again
    If Abs(Piece) = King Then
        If Sgn(Piece)=White Then
            ' White king
            CanCastle(4) = False

            ' Update KingPosition for White
            KingPosition(White,0)=Move(2)
            KingPosition(White,1)=Move(3)

            ' Check to see if moving from initial square, (5,1)
            If Move(0)=5 And Move(1)=1 Then
                ' Check if castling toward queenside
                If Move(2)=7 And Move(3)=1 Then
                    ' Kingside Castle - Move Rook as well
                    Board(6,1) = Board(8,1)
                    Board(8,1) = Blank
                    CanCastle(5) = False
                    ' Check to see if castling toward kingside
                ElseIf Move(2)=3 And Move(3) = 1 Then
                    ' Queenside Castle - Move Rook as well
                    Board(4,1) = Board(1,1)
                    Board(1,1) = Blank
                    CanCastle(3) = False
                EndIf
            EndIf

        ElseIf Sgn(Piece)=Black Then
            ' Black king
            CanCastle(1) = False

            ' Update KingPosition for Black
            KingPosition(Black,0)=Move(2)
            KingPosition(Black,1)=Move(3)

            ' See if moving from initial square (5,8)
            If Move(0)=5 And Move(1)=8 Then

                ' Check if castling toward queenside
                If Move(2)=7 And Move(3)=8 Then
                    ' Kingside Castle - Move Rook as well
                    Board(6,8) = Board(8,8)
                    Board(8,8) = Blank
                    CanCastle(2) = False
                    ' Check to see if castling toward kingside
                ElseIf Move(2)=3 And Move(3) = 8 Then
                    ' Queenside Castle - Move Rook as well
                    Board(4,8) = Board(1,8)
                    Board(1,8) = Blank
                    CanCastle(0) = False
                EndIf
            EndIf
        EndIf
    EndIf

    If ShowMoves Then
        ' Highlight original square

        ' Paint a rectangle inside the box to bound the fill
        Rect (Move(0)-1)*BoxSize+1, (Squares-Move(1))*BoxSize+1, &
        Move(0)*BoxSize-1, ((Squares + 1)-Move(1))*BoxSize-1, &
        Color ColorSelectFinal
        Paint (Move(0)-1)*BoxSize + 2, (Squares-Move(1))*BoxSize + 2, &
        ColorSelectFinal
        Play "V025O3C"

        ' Highlight the square that is being moved to, pause
        ' Paint a rectangle inside the box to bound the fill
        Rect (Move(2)-1)*BoxSize+1, (Squares-Move(3))*BoxSize+1, &
        Move(2)*BoxSize-1, ((Squares + 1)-Move(3))*BoxSize-1, &
        Color ColorSelectFinal
        Paint (Move(2)-1)*BoxSize + 2, (Squares-Move(3))*BoxSize + 2, &
        ColorSelectFinal
        Play "V025O3C"

        ' Update board, play terminal sound
        DisplayBoard BoardSet
        Play "O3G"
    EndIf

    ' Return the new BoardSet
    MakeMove = [Board, CanCastle, Move, KingPosition]
End ' Func MakeMove

' -------------------------------------------------------------------------
' -------------------------------------------------------------------------

' This will return TRUE if, in the given board, side SIDE
' is in check. Otherwise it will return false
Func IsCheck(Board, KingPosition, Side)

    ' This function will move off down the vector defined by (dx, dy)
    ' for a maximum of Limit squares. If it encounters an enemy piece
    ' of a type in the array PieceArray, then we return True
    Func WalkLine (dx, dy, PieceArray, Limit)

        Local Counter, NewX, NewY

        WalkLine = False

        For Counter = 1 to Limit
            NewX = KingPosition(Side,0) + Counter * dx
            NewY = KingPosition(Side,1) + Counter * dy

            If (NewX >=1) And (NewX <= Squares) And &
                (NewY >= 1) And (NewY <= Squares) Then

                If Board(NewX, NewY) <> Blank Then
                    If Abs(Board(NewX, NewY)) in PieceArray Then
                        If Sgn(Board(NewX,Newy)) = -Side Then
                            WalkLine = True
                        EndIf
                    EndIf

                    ' If we get here, we're done one way or another-
                    ' exit function
                    Exit Func
                EndIf
            Else
                ' If we are off the board, stop looking
                Exit Func
            EndIf

        Next Counter

    End ' Func WalkLine

    ' ----------------------------------------------------------------
    ' Main function starts here
    IsCheck = False

    ' Check on diagonals for bishop, queen
    ' (pawns only going forward)
    If WalkLine (-1, -1, [Queen, Bishop], Squares) Then
        IsCheck = True
        Exit Func
    EndIf
    If WalkLine (-1, 1, [Queen, Bishop], Squares) Then
        IsCheck = True
        Exit Func
    EndIf
    If WalkLine (1, -1, [Queen, Bishop], Squares) Then
        IsCheck = True
        Exit Func
    EndIf
    If WalkLine (1, 1, [Queen, Bishop], Squares) Then
        IsCheck = True
        Exit Func
    EndIf

    ' Check pawns
    If WalkLine (-1, Side, [Pawn], 1) Then
        IsCheck = True
        Exit Func
    EndIf
    If WalkLine (1, Side, [Pawn], 1) Then
        IsCheck = True
        Exit Func
    EndIf

    ' Check on orthagonals for rook, queen
    If WalkLine (-1, 0, [Queen, Rook], Squares) Then
        IsCheck = True
        Exit Func
    EndIf
    If WalkLine (1, 0, [Queen, Rook], Squares) Then
        IsCheck = True
        Exit Func
    EndIf
    If WalkLine (0, -1, [Queen, Rook], Squares) Then
        IsCheck = True
        Exit Func
    EndIf
    If WalkLine (0, 1, [Queen, Rook], Squares) Then
        IsCheck = True
        Exit Func
    EndIf

    ' Check for knights
    If WalkLine (-2, 1, [Knight], 1) Then
        IsCheck = True
        Exit Func
    EndIf
    If WalkLine (-2, -1, [Knight], 1) Then
        IsCheck = True
        Exit Func
    EndIf
    If WalkLine (2, 1, [Knight], 1) Then
        IsCheck = True
        Exit Func
    EndIf
    If WalkLine (2, -1, [Knight], 1) Then
        IsCheck = True
        Exit Func
    EndIf
    If WalkLine (-1, 2, [Knight], 1) Then
        IsCheck = True
        Exit Func
    EndIf
    If WalkLine (-1, -2, [Knight], 1) Then
        IsCheck = True
        Exit Func
    EndIf
    If WalkLine (1, 2, [Knight], 1) Then
        IsCheck = True
        Exit Func
    EndIf
    If WalkLine (1, -2, [Knight], 1) Then
        IsCheck = True
        Exit Func
    EndIf

    ' Check for Kings
    If WalkLine (-1, -1, [King], 1) Then
        IsCheck = True
        Exit Func
    EndIf
    If WalkLine (-1, 0, [King], 1) Then
        IsCheck = True
        Exit Func
    EndIf
    If WalkLine (-1, 1, [King], 1) Then
        IsCheck = True
        Exit Func
    EndIf
    If WalkLine (0, -1, [King], 1) Then
        IsCheck = True
        Exit Func
    EndIf
    If WalkLine (0, 1, [King], 1) Then
        IsCheck = True
        Exit Func
    EndIf
    If WalkLine (1, -1, [King], 1) Then
        IsCheck = True
        Exit Func
    EndIf
    If WalkLine (1, 0, [King], 1) Then
        IsCheck = True
        Exit Func
    EndIf
    If WalkLine (1, 1, [King], 1) Then
        IsCheck = True
        Exit Func
    EndIf

End ' Func IsCheck

' -------------------------------------------------------------------

' This function is passed a board and a color and returns a MoveArray listing
' all valid moves that color Side can make
Func GenerateMoveList (BoardSet, Side)

    ' MoveArray is an array of MoveLists
    Local MoveArray

    ' Coordinates to move through the board
    Local x, y, Board

    ' --------------------------------------------------------------------
    ' This routine will take the piece starting at (x,y) in Board
    ' and walk off in vector (dx, dy) until it jumps off the board,
    ' captures another piece in that vector, runs into one of its
    ' own pieces in that vector, or exceeds the number of squares in
    ' limit (1 for King, infinity for rook, queen, bishop).
    ' For each change to board, if making this move does NOT result
    ' in a check situation for Side (the color of the piece at x, y)
    ' then the postulated move will be added into MoveArray
    Sub WalkLine(x, y, dx, dy, Limit)

        Local Counter, NewBoard, Piece, NewX, NewY, MustCheck, KingPosition

        ' Grab current position of the Kings - needed for IsCheck
        KingPosition = BoardSet(3)

        ' This is TRUE if we must check to see if this
        ' movement results in a check before adding it to the list
        ' We must check if:
        '   It is the first move (if we are in check)
        '   The piece is a king (any move can result in check)
        '   If, after the first move, the piece is not a king, and
        '   we don't start out in check, then we don't need to check again
        '   It's worth trying to avoid the check "check" to save time
        MustCheck = ((Abs(Board(x,y)) = King) Or IsCheck(Board, KingPosition, Side))

        For Counter = 1 to Limit
            NewX = x + Counter * dx
            NewY = y + Counter * dy

            ' Make sure the proposed move is still on the board
            If (NewX<1) or (NewY<1) or (NewX>Squares) or (NewY>Squares) Then
                Exit For
            Else
                ' See if piece is either empty or occupied by piece from
                ' the other side
                ' If so, this is a valid move
                If Sgn(Board(NewX, NewY)) <> Side Then

                    ' Make the move on a "virtual" board
                    NewBoard = Board
                    Piece = NewBoard(x,y)
                    NewBoard(x,y) = Blank
                    NewBoard(NewX, NewY) = Piece

                    ' If that move does not result in check, then
                    ' add it to possible move list

                    ' If the piece is a King, then update KingPosition
                    If Abs(Piece) = King Then
                        KingPosition(Side,0) = NewX
                        KingPosition(Side,1) = NewY
                    EndIf

                    If MustCheck Or (Counter <= 2) Then
                        If Not IsCheck(NewBoard, KingPosition, Side) Then
                            MoveArray << [x, y, NewX, NewY]
                        EndIf
                    Else
                        MoveArray << [x, y, NewX, NewY]
                    EndIf

                    ' If this was a capture, then end the loop as we
                    ' can't advance any further in this direction
                    If Sgn(Board(NewX, NewY)) = -Side Then
                        Exit For
                    EndIf
                ElseIf Sgn(Board(NewX, NewY)) = Side Then
                    ' We're done - can't move over our own piece; exit
                    ' the loop
                    Exit For
                EndIf
            EndIf

        Next Counter

    End ' WalkLine

    ' -------------------------------------------------------------------------
    ' Specific piece checks start here

    Sub CheckPawn(x, y)
        Local NewBoard, Piece, Counter, LastMove, KingPosition

        ' Grab current positions of Kings, for IsCheck
        KingPosition = BoardSet(3)

        ' Check for single move
        If Board(x, y + Side) = Blank Then
            NewBoard = Board
            Piece = NewBoard (x,y)
            NewBoard (x,y) = Blank
            NewBoard(x, y + Side) = Piece
            If Not IsCheck(NewBoard, KingPosition, Side) Then
                MoveArray << [x, y, x, y + side]
            EndIf
        EndIf

        ' Check for double move, if starting in 2/7 row
        If ((Side = White) And (y=2)) or ((Side=Black) And (y=7)) Then
            If (Board(x,y+Side)=Blank) And (Board(x,y+2*Side)=Blank) Then
                NewBoard = Board
                Piece = NewBoard (x,y)
                NewBoard (x,y) = Blank
                NewBoard(x, y + 2*Side) = Piece
                If Not IsCheck(NewBoard, KingPosition, Side) Then
                    MoveArray << [x, y, x, y + 2*Side]
                EndIf
            EndIf
        EndIf

        ' Check for capture on diagonals
        For Counter in [-1, 1]
            If ((x+Counter) >= 1) And ((x+Counter)<= Squares) And &
                ((y+Side)>=1) And ((y+Side)<=Squares) Then
                If Sgn(Board(x+Counter,y+Side)) = -Side Then
                    NewBoard = Board
                    Piece = NewBoard (x,y)
                    NewBoard (x,y) = Blank
                    NewBoard(x + Counter, y + Side) = Piece
                    If Not IsCheck(NewBoard, KingPosition, Side) Then
                        MoveArray << [x, y, x + Counter, y + Side]
                    EndIf
                EndIf
            EndIf
        Next Counter

        ' Check for en-passant (look at previous move - BoardSet(2) and BoardSet(3))

        ' Grab this for convenience
        LastMove = BoardSet(2)
        If LastMove <> [0,0,0,0] Then
            If Board(LastMove(2), LastMove(3)) = -Side * Pawn Then
                ' Check to see if last move was one column away from current pawn
                ' and the last move was a double advance; In theory,
                ' Abs(LastMove(2)-LastMove(0)) = 0, so we won't check to save time
                If Abs(LastMove(0) - x) = 1 And (Abs(LastMove(3)-LastMove(1))=2) Then
                    ' See if we are in the correct Row
                    If ((Side=White) and (y=5)) Or ((Side=Black) And (y=4)) Then
                        ' If we get here, en passant is possible!
                        NewBoard = Board
                        Piece = NewBoard (x,y)
                        NewBoard (x,y) = Blank
                        NewBoard(LastMove(0), y + Side) = Piece
                        If Not IsCheck(NewBoard, KingPosition, Side) Then
                            MoveArray << [x, y, LastMove(0), y + Side]
                        EndIf
                    EndIf
                EndIf
            EndIf
        EndIf

    End ' Func CheckPawn

    Sub CheckRook (x,y)
        ' Rooks move orthogonally
        WalkLine x, y, 1, 0, Squares
        WalkLine x, y, 0, 1, Squares
        WalkLine x, y, -1, 0, Squares
        WalkLine x, y, 0, -1, Squares
    End ' Func CheckRook

    Sub CheckKnight (x,y)
        ' Check all the squares a Knight could go to
        WalkLine x, y, -1, 2, 1
        WalkLine x, y, 1, 2, 1
        WalkLine x, y, -1, -2, 1
        WalkLine x, y, 1, -2, 1
        WalkLine x, y, -2, 1, 1
        WalkLine x, y, -2, -1, 1
        WalkLine x, y, 2, 1, 1
        WalkLine x, y, 2, -1 , 1
    End ' Func CheckKnight

    Sub CheckBishop (x,y)
        ' Bishops can move diagonally only - check all four diagonals
        WalkLine x, y, 1, 1, Squares
        WalkLine x, y, 1, -1, Squares
        WalkLine x, y, -1, 1, Squares
        WalkLine x, y, -1, -1, Squares
    End ' Func CheckBishop

    Sub CheckQueen (x,y)
        ' Queens can move either diagonally or orthagonally
        ' Check all 8 directions, up to Squares moves
        WalkLine x, y, 1, 0, Squares
        WalkLine x, y, 0, 1, Squares
        WalkLine x, y, -1, 0, Squares
        WalkLine x, y, 0, -1, Squares
        WalkLine x, y, 1, 1, Squares
        WalkLine x, y, 1, -1, Squares
        WalkLine x, y, -1, 1, Squares
        WalkLine x, y, -1, -1, Squares
    End ' Func CheckQueen

    Sub CheckKing (x,y)
        Local CanCastle, NewBoard, KingPosition

        ' Kings can move either diagonally or orthagonally for 1 square
        ' Check all 8 directions, 1 move only
        WalkLine x, y, 1, 0, 1
        WalkLine x, y, 0, 1, 1
        WalkLine x, y, -1, 0, 1
        WalkLine x, y, 0, -1, 1
        WalkLine x, y, 1, 1, 1
        WalkLine x, y, 1, -1, 1
        WalkLine x, y, -1, 1, 1
        WalkLine x, y, -1, -1, 1

        ' Use this to keep track of the king for IsCheck
        KingPosition = BoardSet(3)

        ' Also, check for potential to castle either
        ' a-ward or g-ward
        CanCastle = BoardSet(1)
        ' See if King has been moved
        If ((Side=Black) And CanCastle(1)) Or ((Side=White) And CanCastle(4)) Then
            ' If here, then check and see if queen-rook has been moved
            If ((Side=Black) And CanCastle(0)) Or ((Side=White) And CanCastle(3)) Then
                ' See if intermediate squares are clear
                If (Board(4,y)=Blank) and (Board(3,y)=Blank) And (Board(2, y)= Blank) Then
                    ' See if intermediate moves cause check
                    NewBoard = Board
                    ' Can't castle out of check!
                    If Not IsCheck(NewBoard, KingPosition, Side) Then
                        ' Check to see if one square to left causes check
                        NewBoard(x,y) = Blank
                        NewBoard(x-1,y) = King * Side

                        KingPosition(Side,0) = x-1
                        KingPosition(Side,1) = y

                        If Not IsCheck(NewBoard, KingPosition, Side) Then
                            ' Check to see if two squares to left causes check
                            NewBoard(x-1,y) = Blank
                            NewBoard(x-2,y) = King * Side

                            KingPosition(Side,0) = x-2
                            KingPosition(Side,1) = y

                            If Not IsCheck(NewBoard, KingPosition, Side) Then
                                ' If we get here, king-side castle is allowed!
                                MoveArray << [x, y, x-2, y]
                            EndIf
                        EndIf
                    EndIf
                EndIf
            EndIf

            ' See if king-rook has been moved
            If ((Side=Black) And CanCastle(2)) Or ((Side=White) And CanCastle(5)) Then
                ' See if intermediate squares are clear
                If (Board(6, y)=Blank) and (Board(7, y)=Blank) Then
                    ' See if intermediate moves cause check
                    NewBoard = Board
                    ' Can't castle out of check!
                    If Not IsCheck(NewBoard, KingPosition, Side) Then
                        ' Check to see if one square to right causes check
                        NewBoard(x,y) = Blank
                        NewBoard(x+1,y) = King * Side

                        KingPosition(Side,0) = x+1
                        KingPosition(Side,1) = y

                        If Not IsCheck(NewBoard, KingPosition, Side) Then
                            ' Check to see if two squares to right causes check
                            NewBoard(x+1,y) = Blank
                            NewBoard(x+2,y) = King * Side

                            KingPosition(Side,0)=x+2
                            KingPosition(Side,1)=y

                            If Not IsCheck(NewBoard, KingPosition, Side) Then
                                ' If we get here, king-side castle is allowed!
                                MoveArray << [x, y, x+2, y]
                            EndIf
                        EndIf
                    EndIf
                EndIf
            EndIf
        EndIf ' Whew!
    End ' Func CheckKing


    ' -------------------------------------------------------------------
    ' Function actually begins here
    '

    Board = BoardSet(0)

    For x = 1 to Squares
        For y = 1 to Squares
            If Board(x,y) = Side * Pawn Then
                CheckPawn x, y
            ElseIf Board(x,y) = Side * Rook Then
                CheckRook x, y
            ElseIf Board(x,y) = Side * Knight Then
                CheckKnight x, y
            ElseIf Board(x,y) = Side * Bishop Then
                CheckBishop x, y
            ElseIf Board(x,y) = Side * Queen Then
                CheckQueen x, y
            ElseIf Board(x,y) = Side * King Then
                CheckKing x, y
            EndIf
        Next y
    Next x

    GenerateMoveList = MoveArray
End ' Func GenerateMoveList

' -------------------------------------------------------------------------
' -------------------------------------------------------------------------

' This function will allow the user to enter a move for a side
' It outputs a Move
Func GetUserMove (BoardSet, Side)

    ' Where the user is actually moving
    Local ValidMoveArray, Move, ClickMove, Board, OldClickMove,
    Local ValidMove, IsValid, IsInCheck, KingPosition

    ' --------------------------------------------------------
    ' Waits for the user to click on a square and returns
    ' the x,y board components as (x, y, 0, 0)
    ' (DataType = Move)
    ' Also, we will only accept clicks on Dark squares (X+Y) Mod 2 = 0
    Func GetSquare

        ' Enable tracking of mouse
        Pen on

        ' Loop until the left mouse button is pressed
        Repeat
        Until Pen(0)

        ' Convert mouse coordinates into Board coordinates
        ' Pen(1) = X of mouse position "Last mouse button down X"
        ' Pen(2) = Y of mouse position "Last mouse button down Y"
        #? pen(1)
        #? pen(2)
        GetSquare = [1 + Int(Pen(1)/BoxSize),Squares - Int(Pen(2)/BoxSize),0,0]

        ' Stop Mouse mechanism
        Pen Off
    End

    ' ---------------------------------------------------------------
    ' This will change the background of a square to color SquareColor
    Sub HighlightSquare (x, y, SquareColor)

        ' Only highlight a valid square
        If (x >= 1) and (x <= Squares) And (y >= 1) And (y <= Squares) Then

            ' Paint a rectangle inside the box to bound the fill
            Rect (x-1)*BoxSize+1, (Squares-y)*BoxSize+1, &
            x*BoxSize-1, ((Squares + 1)-y)*BoxSize-1, &
            Color SquareColor
            Paint (x-1)*BoxSize + 2, (Squares-y)*BoxSize + 2, SquareColor

        EndIf
    End ' Sub HighLightSquare (x,y, SquareColor)

    ' ------------------------------------------------------------------
    ' Function actually starts here

    OldClickMove = [0,0,0,0]

    Board = BoardSet(0)
    KingPosition = BoardSet(3)

    IsInCheck = IsCheck(Board, KingPosition, Side)

    ' Initialize variables
    ValidMoveArray = GenerateMoveList (BoardSet, Side)

    Move = [0,0,0,0]
    Done = False
    Repeat

        ' Put up a fresh board
        DisplayBoard BoardSet

        ' If we are in check, highlight this Side's King square
        If IsInCheck Then
            HighlightSquare KingPosition(Side,0), KingPosition(Side,1), ColorCheck
        EndIf

        ' Inform use of which side is to move
        At 0, BoxSize * Squares ' + TextHeight (PlayerName(Side))/2
        If Side = Black Then
            Color ColorBlack, ColorDarkSquare
        Else
            Color ColorWhite, ColorDarkSquare
        EndIf
        Print PlayerName(Side) + " to move";

        ' Get First Move
        Repeat
            ClickMove = GetSquare
            #? ClickMove
        Until ClickMove <> OldClickMove
        OldClickMove = ClickMove

        'See if it is valid
        IsValid = False
        For ValidMove in ValidMoveArray
            IsValid = (ClickMove(0) = ValidMove(0)) And &
            (ClickMove(1) = ValidMove(1))
            If IsValid Then
                Exit For
            EndIf
        Next ValidMove

        If IsValid Then

            HighLightSquare ClickMove(0), ClickMove(1), ColorSelect
            Move = ClickMove

            ' Get Second Move
            Repeat
                ClickMove = GetSquare
            Until ClickMove <> OldClickMove
            OldClickMove = ClickMove

            HighLightSquare ClickMove(0), ClickMove(1), ColorSelect
            Move(2) = ClickMove(0)
            Move(3) = ClickMove(1)
        Else
            ' Give error beep
            Play "O3A"

        EndIf

        ' Check to see if the move is valid
    Until Move in ValidMoveArray

    ' Return the move list
    GetUserMove = Move

End ' Func GetUserMove

' -------------------------------------------------------------------------
' -------------------------------------------------------------------------

' This function processes the Artificial Intelligence aspects for
' computer move generation, using the standard Minimax game algorithm
' recursively
' Because of the nature of MiniMax, it has to return two values:
' a MoveList and a Score. We will define a data structure of
' [Score, MoveList] to hold these
' Board is a board to process; Side is the side whose turn it is to move,
' Depth is CURRENT search depth (initally set to zero),
' DepthMax = Deepest level to explore, Evaluator is used to determine
' which board evaluation function to use (see EvaluateBoard)
' We keep all moves with same score in BestMoveList and choose randomly
' among them to try and avoid loops of repetitive moves
Func MiniMax(BoardSet, Side, Depth, DepthMax, Evaluator)

    Local MoveList, Move, Score, BestScore, NextMini, BestMoveList, Board

    Board = BoardSet(0)

    If Depth > DepthMax Then
        ' If we have exceeded the specified search depth,
        ' return the value of our board at this point and
        ' a null movelist
        MiniMax = [(EvaluateBoard (Board, Side, Evaluator)), 0]
    Else

        ' Find all possible moves for Side with the current Board
        MoveList = GenerateMoveList (BoardSet, Side)

        ' If there are no moves, then return the score
        ' of the current board and a null MoveList
        If Empty (MoveList) Then
            MiniMax = [EvaluateBoard (Board, Side, Evaluator), 0]
        Else
            ' Find the move that yields the best MiniMax score

            ' Set initial BestScore to lowest conceivable results, to
            ' guarantee it will be replaced with an actual result later
            BestScore = -999999
            Erase BestMoveList
            ' Step through each possible move
            For Move in MoveList

                ' Recurse down the search tree for the board that
                ' would result from making that move
                NextMini = MiniMax(MakeMove(BoardSet, Move, False), &
                -Side, Depth+1, DepthMax, Evaluator)

                ' If that move results in a better MiniMax score, then
                ' save that result
                Score = -NextMini(0)
                If Score >= BestScore Then
                    ' If this ties the current best score, add to the list
                    If Score = BestScore Then
                        BestMoveList << Move
                    Else
                        ' If this is a new best score, set the list to this
                        ' move only and update best score
                        BestScore = Score
                        BestMoveList = [Move]
                    EndIf
                EndIf
            Next Move

            ' Return our best move
            ' Select randomly from the list of moves with an equal / best score
            MiniMax = [BestScore, BestMoveList(Int(Rnd * UBound(BestMoveList)))]
        EndIf
    EndIf

End ' Func Minimax

' -------------------------------------------------------------------------
' -------------------------------------------------------------------------

' This function applies a custom evaluator to be used by MiniMax
' There should be a private function for each different approach
' to scoring the boards, and the main function code will chose among
' them based on Evaluator
' Right now we are just using "SimpeScore", which adds up points for
' pieces. You could add something that gives bonus points for control of
' the center, bonus points for knights in a closed games or bishops in
' an open game, etc.

Func EvaluateBoard (Board, Side, Evaluator)

    ' ------------------------------------------------------------------
    ' This function just adds up the pieces in Board (1=piece,2=King)
    ' and returns the net sum
    ' It was the simplest one I could think of
    Func ScoreSimple(Board, Side)
        Local x, y, Score

        Score = 0
        For x = 1 to Squares
            For y = 1 to Squares
                If Abs(Board(x,y)) = Pawn Then
                    Score = Score + 1 * Sgn(Board(x,y))
                ElseIf Abs(Board(x,y)) = Rook Then
                    Score = Score + 5 * Sgn(Board(x,y))
                ElseIf Abs(Board(x,y)) = Knight Then
                    Score = Score + 3 * Sgn(Board(x,y))
                ElseIf Abs(Board(x,y)) = Bishop Then
                    Score = Score + 3.25 * Sgn(Board(x,y))
                ElseIf Abs(Board(x,y)) = Queen Then
                    Score = Score + 9 * Sgn(Board(x,y))
                ElseIf Abs(Board(x,y)) = KingThen
                    Score = Score + 100 * Sgn(Board(x,y))
                EndIf
            Next y
        Next x

        ScoreSimple = Score * Side
    End ' Func ScoreSimple

    ' ------------------------------------------------------------------------
    ' Function begins here

    ' The idea is that we have multiple Evaluator functions as a test
    ' of different strategies

    If Evaluator = 2 Then
        EvaluateBoard = ScoreSimple (Board, Side)
    EndIf
End ' Func EvaluateBoard

' -------------------------------------------------------------------------
' -------------------------------------------------------------------------

' This function will determine the next move for Side based on the
' strategy in PlayerStrategy(Side) and return a MoveList
Func DetermineMove (BoardSet, Side)
    Local Temp

    ' This will display text to the right (by 1/4 square) of the board
    ' listing who is currently moving
    ' The color of the text will match the color of the pieces
    ' Set text color to match
    If Side = Black Then
        Color ColorBlack
    Else
        Color ColorWhite
    EndIf

    At 0, BoxSize * Squares ' + TextHeight (PlayerName(Side))/2
    Print PlayerName(Side) + " to move";

    If PlayerStrategy(Side) = 1 Then
        DetermineMove = GetUserMove (BoardSet, Side)
    ElseIf PlayerStrategy(Side) = 2 Then
        Temp = MiniMax(BoardSet, Side, 0, 1, 2)
        DetermineMove = Temp(1)
    ElseIf PlayerStrategy(Side) = 3 Then
        Temp = MiniMax(BoardSet, Side, 0, 2, 2)
        DetermineMove = Temp(1)
    ElseIf PlayerStrategy(Side) = 4 Then
        Temp = MiniMax(BoardSet, Side, 0, 3, 2)
        DetermineMove = Temp(1)
    EndIf
End ' Func DetermineMove

' -------------------------------------------------------------------------
' -------------------------------------------------------------------------

' This is the main control function for the checkers program
Sub Main

    Local BoardSet, MoveList, Winner, Counter

    ' Get User information:
    ' For each color, get a unique name for the player and
    ' a strategy for determining the move to make

    For Counter in [White, Black]

        ' Get the name of the players
        Print "Please enter the name for the "
        If Counter = White Then
            Print "white";
        Else
            Print "black";
        EndIf
        Print " player :";
        Input PlayerName (Counter)

        Print

        Repeat
            ' Get the strategy for each player to use
            Print "1. Ask the user"
            Print "2. Use SimpleScore Level 1"
            Print "3. Use SimpleScore Level 2"
            Print "4. Use SimpleScore Level 3"
            Print "Enter the strategy to use for "; PlayerName(Counter);
            Input PlayerStrategy(Counter)
        Until PlayerStrategy(Counter) in [1,2,3,4]

        Print
        Print
    Next Counter

    ' Initialize the board
    BoardSet = InitBoard

    ' Display the board
    DisplayBoard BoardSet

    ' Loop until game is over - it is over when one side cannot
    ' make any more moves
    While True

        ' Allow white to move is there is a move for White to make
        If Empty(GenerateMoveList (BoardSet, White)) Then
            Winner = Black
            Exit Loop
        Else
            ' Get White Move
            MoveList = DetermineMove (BoardSet, White)

            ' Apply White Move
            BoardSet = MakeMove (BoardSet, MoveList, True)

            ' Show the new board
            DisplayBoard BoardSet
        EndIf

        ' Allow Black to move if there is a move for Black to make
        If Empty(GenerateMoveList (BoardSet, Black)) Then
            Winner = White
            Exit Loop
        Else
            ' Get Black Move
            MoveList = DetermineMove (BoardSet, Black)

            ' Apply Black Move
            BoardSet = MakeMove (BoardSet, MoveList, True)

            ' Show the new board
            DisplayBoard BoardSet
        EndIf

    Wend

    ' Now, display the winner
    At 0, BoxSize * Squares '+ TextHeight ("Wins!") / 2
    If IsCheck (BoardSet(0), BoardSet(3), -Winner) Then
        If Winner = Black Then
            Color ColorBlack
        Else
            Color ColorWhite
        EndIf

        Print "CheckMate! " ; PlayerName(Winner) ; " wins!";
    Else
        Print "Stalemate!"
    EndIf

    ' Play a little fanfare and pause for 1 second
    Play "O2CEGO3CP1"
End ' Sub Main


' -------------------------------------------------------------------------
' -------------------------------------------------------------------------

' Call the Main procedure, then exit the program

' Declare array of player names
Dim PlayerName (Black to White)

' Declare array of strategy choices
Dim PlayerStrategy (Black to White)


Main
End
