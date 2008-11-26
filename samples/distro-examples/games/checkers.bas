'app-plug-in
'menu Games/Checkers

' Checkers program

' Written by KC Goldberg and Sons, November, 2004
' Written in SmallBASIC for Windows, 0.9.5.2

' This program was written as a teaching example with me and my sons
' We used SmallBASIC mostly because it is cool and free. For this particular
' application, the array structure (including nested arrays) and the ability
' of functions to return an array make this almost LISP-like, which is perfect
' for turn-based games and the MiniMax algorithm

' In addition to the rich array strucure, there is a simple mechanism to
' capture both console input and mouse clicks, and a very accessible
' graphics package.

' Finally, although I have not done this, the ability to port this to my
' PalmPilot is just too cool!

' The program here will either ask the user to make a move using a
' "click on the board" interface, or generate one itself using the
' MiniMax algorithm.

' To add different board evaluators, look into function EvaluateBoard
' and Function DetermineMove for different ply depths

' =====================================================================
' Data types:
' Board: Array Squares x Squares of pieces
' B(1,1) = lower left
' B(Squares,Squares) = upper right
' Piece: Integer; -2,2: King; -1,1 Piece; 0 Blank
' Move: Array of 4x1 listing coordinates (from, to) e.g. [x1,y1,x2,y2]
' MoveList: An array containing moves e.g. [[x1,y1,x2,y2],[x2,y2,x3,y3], ...]
' To move a piece, a move exists for each intermediate step as well; a jump
' will imply three elements - initial, the square of the piece being jumped,
' and the termination; a double jump will have five elements, etc.
' MoveListArray: An array of MoveLists
' MiniMaxResult: [Score, Move] where score is a numeric board evaluation Score

' ===========================================================================
' Procedures:

' DisplayBoard (Board) : Display a board

' Functions:

' MakeMove (Board, MoveList) = Board : Apply a MoveList to a Board
' GenerateMoveList (Board, Side) = MoveListArray : Generate list of all valid moves for Side
' GetUserMove (Board, Side) = MoveList : Ask user to move for Side
' InitBoard = Board : Return a new board with pieces in initial position
' Func MiniMax(Board, Side, Depth, DepthMax, Evaluator) = MiniMaxResult
' : Perform MiniMax algorithm search to find best move for Side (see declaration
' for description of all parameters)
' EvaluateBoard (Board, Side, Evaluator) = Score : Return a numeric score with positive
' numbers favoring Side using the evaluation function specified by Evaluator
' This should only be called from within MiniMax; technically I should probably make
' it a private function of Minimax, but as it too has private functions, that seemed
' like a lot of overhead for Minimax, which will be called recursively
' DetermineMove (Board, Side) = MoveList : Use the method stored in PlayerStrategy(Side)
' to determine the next move for Side to make
' Main : Main procedure loop
'


' ===================================================================
' Constants
' Size of the board (default = 8 for standard checkers)
Const Squares = 8

' Number of rows to fill in at start (default = 3 for standard checkers)
Const InitialRows = 3

' These constants denote what occupies a piece on any board square
Const Piece = 1
Const King = 2
Const Blank = 0

' This defines the sides
Const Black = -1
Const White = 1

' Figure out how big the boxes should be for graphical display, in pixels
' (board will be square)
Const BoxSize = Min(xmax, ymax) / Squares

' Colors for display

' Color of board squares (background)
Const ColorDarkSquare = 0 ' Black
Const ColorLightSquare = 4 ' Dark Red
Const ColorSelect = 1 ' Deep Blue
Const ColorSelectFinal = 9 ' Bright blue

' Color of pieces on squares
Const ColorBlack = 8 ' Dark Gray
Const ColorWhite = 15 ' White

' =========================================================================
' =========================================================================

' -------------------------------------------------------------------------
' This will Generate a Board to the game starting values
Func InitBoard
    ' Variables we will be using
    Local x,y

    Dim Board (1 to Squares, 1 to Squares)

    ' First, clear out board
    For x = 1 to Squares
        For y = 1 to Squares
            Board(x,y) = Blank
        Next y
    Next x

    ' Now, put pieces in place

    ' We will only put pieces on the "dark" squares, which
    ' will be identified by the property that (x+y) is even
    ' i.e. (x+y) Mod = 2
    ' e.g. (1,1), (1,3), (2,2), ...

    For y = 1 to InitialRows
        For x = 1 to Squares
            ' White squares go on the "bottom" InitialRows squares
            If (x+y) Mod 2 = 0 then
                Board(x,y) = White
            EndIf

            ' Black squares go on the "top" InitialRows squares
            If (x + ((Squares+1) - y)) Mod 2 = 0 then
                Board (x, ((Squares+1) - y)) = Black
            Endif
        Next x
    Next y

    InitBoard = Board
End

' -------------------------------------------------------------------
' This routine will display a graphic square with the checkers pieces
' kept in board B
Sub DisplayBoard (Board)
    ' Variables we will be using
    ' x and y for counters, FillColor a color
    Local x, y, FillColor

    ' Clear screen
    Cls

    ' Draw the background squares
    For x = 1 to Squares
        For y = 1 to Squares
            Rect (x-1)*BoxSize, (Squares-y)*BoxSize, &
            x*BoxSize, ((Squares + 1)-y)*BoxSize, &
            Color IF((x+y) Mod 2 = 0, ColorDarkSquare, ColorLightSquare) &
            Filled
        Next y
    Next x

    ' Draw the pieces as circles 75% the size of the squares
    For x = 1 to Squares
        For y = 1 to Squares
            If Board(x,y) <> Blank then

                ' Choose color for circle
                If Sgn(Board(x,y)) = White then
                    FillColor = ColorWhite
                Elseif Sgn(Board(x,y)) = Black then
                    FillColor = ColorBlack
                EndIf

                ' Draw the checker
                Circle (x-0.5)*BoxSize, ((Squares+0.5)-y)*BoxSize, &
                BoxSize*0.375, &
                1 , Color FillColor Filled

                ' If the piece is a king, draw a black circle in the center
                ' and another colored concentric circle and a final central
                ' black circle
                If Abs(Board(x,y)) = King then
                    Circle (x-0.5)*BoxSize, ((Squares+0.5)-y)*BoxSize, &
                    BoxSize*0.275, &
                    1 , Color 0 Filled
                    Circle (x-0.5)*BoxSize, ((Squares+0.5)-y)*BoxSize, &
                    BoxSize*0.175, &
                    1 , Color FillColor Filled
                    Circle (x-0.5)*BoxSize, ((Squares+0.5)-y)*BoxSize, &
                    BoxSize*0.075, &
                    1 , Color 0 Filled
                Endif

            EndIf
        Next y
    Next x
End

' -------------------------------------------------------------------
' This function is passed a Board and a MoveList and returns the
' board after that series of moves is made
' Note: When jumping, the MoveList should move the piece only one
' square at a time - which will include driving right over the
' enemy piece
' If ShowMoves = True, then display intermediate moves and play
' appropriate tunes, allowing the user to follow along
Func MakeMove(Board, MoveList, ShowMoves)

    ' Piece is the type of piece we are making
    ' Move is an array of four coordinates, from/to (x1, y1, x2, y2)
    Local Piece, Move

    ' Step through each of the moves in ML
    For Move in MoveList

        If ShowMoves Then
            Paint (Move(0)-1)*BoxSize + 1, (Squares-Move(1))*BoxSize + 1, &
            ColorSelectFinal
        EndIf

        ' Find out what kind of piece we are starting with
        Piece = Board(Move(0), Move(1))

        ' Blank out the square where we started
        Board(Move(0), Move(1)) = Blank

        ' Fill in the square we are moving to
        Board(Move(2), Move(3)) = Piece

        ' See if it should be a king (should be if it lands in either
        ' row 1 or row Squares
        If Move(3) = Squares or Move(3) = 1 then
            Board(Move(2), Move(3)) = King * Sgn(Piece)
        Endif

        If ShowMoves Then
            Paint (Move(2)-1)*BoxSize + 1, (Squares-Move(3))*BoxSize + 1, &
            ColorSelectFinal
            Play "V025O3C"
        EndIf
    Next Move

    If ShowMoves Then
        DisplayBoard Board
        Play "O3G"
    EndIf

    ' Return the new board
    MakeMove = Board
End

' -------------------------------------------------------------------

' This function is passed a board and a color and returns a MoveListArray listing
' all valid moves that color Side can make
' If there are jumps, then only jumps will be in the movelist (which is to say,
' if you have a jump, you must take it!)
Func GenerateMoveList (Board, Side)

    ' -------------------------------------------------------------------
    ' Takes a board, starting position, and offset and will return TRUE
    ' if that is a valid move, and false if it is not
    ' abs(dx) must = abs(dy)
    Func ValidSingleMove (x, y, dx, dy)

        ValidSingleMove = False

        ' Check for single move
        ' There is a valid move if the square 1 square away from
        ' Board(x,y) in direction of dx,dy is Blank
        If ((x+dx) >= 1) and ((x+dx) <= Squares) and &
            ((y+dy) >= 1) and ((y+dy) <= Squares) Then
            If Board(x+dx, y+dy) = Blank then
                ValidSingleMove = True
            EndIf
        EndIf
    End

    ' -------------------------------------------------------------------
    ' This returns an array of Moves (a MoveList); however, these
    ' moves are NOT a sequence to be execute, just a list of up to
    ' four possible moves, which is then processed by FindAllJumps
    Func FindJumps (Board, x, y)
        Local MoveList, dx

        Erase MoveList

        For dx in [-1, 1]
            ' All pieces can jump "forward"

            ' First, check that coordinates of destination square are legal
            If ((x + dx*2) >= 1) And ((x + dx*2) <= Squares) And &
                ((y+2*Side) >= 1) And ((y+2*Side) <= Squares) Then
                ' Then, check if destination square is open and
                ' that intermediate square is enemy piece
                If (Board(x+dx*2,y+2*Side)=Blank) And &
                    (Sgn(Board(x+dx,y+side))=-Side) Then
                    ' If we get here, we have a valid jump; add it
                    ' to the movelist
                    MoveList << &
                    [[x, y, x+dx, y+Side], [x+dx, y+Side, x+dx*2, y+Side*2]]
                EndIf
            EndIf

            ' Only Kings can jump "backwards"
            If Abs(Board(x,y)) = King then
                If ((x + dx*2) >= 1) And ((x + dx*2) <= Squares) And &
                    ((y-2*Side) >= 1) And ((y-2*Side) <= Squares) Then

                    ' Then, check if destination square is open and
                    ' that intermediate square is enemy piece
                    If (Board(x+dx*2,y-2*Side)=Blank) And &
                        (Sgn(Board(x+dx,y-side))=-Side) Then
                        ' If we get here, we have a valid jump; add it
                        ' to the movelist
                        MoveList << &
                        [[x, y, x+dx, y-Side], [x+dx, y-Side, x+dx*2, y-Side*2]]
                    EndIf
                EndIf
            EndIf

        Next Dx

        FindJumps = MoveList
    End ' Func FindJumps

    ' -------------------------------------------------------------------
    Func FindAllJumps (x, y)

        Local MoveList, MoveListArray, Move, Board2
        Local MoveList2, MoveListArray2
        Local MoveList3
        Local Counter

        ' Populate move list with first degree of available jumps
        MoveListArray = FindJumps(Board, x,y)

        If Not Empty (MoveListArray) Then
            ' For each jump, "make" the jump on a temporary Board and
            ' see if that leads to new jumps; if it does, then
            ' we have to make a list of NEW jumps with the original
            ' sequence as a base and delete the initiating sequence
            Counter = 0
            While Not Empty(MoveListArray(Counter))
                ' See if there is a move

                If Not Empty(MoveListArray(Counter)) Then

                    ' OK - start with this MoveList
                    MoveList = MoveListArray(Counter)

                    ' Apply this MoveList to the current Board and
                    ' find where the checker ends up (x2 and y2 from Move)
                    Board2 = MakeMove(Board, MoveList, False)
                    Move = MoveList(UBound(MoveList))

                    ' Now, see if there are any more jumps from the now terminal
                    ' position
                    MoveListArray2 = FindJumps(Board2, Move(2), Move(3))

                    ' If there are any, then for each of the MoveLists in MoveListArray2,
                    ' create a new entry in MoveListArray that adds the most recent
                    ' position on to the move before at the end of the array, and
                    ' then delete the current one
                    If Not Empty(MoveListArray2) Then
                        For MoveList2 in MoveListArray2
                            MoveList3 = MoveList
                            For Move in MoveList2
                                MoveList3 << Move
                            Next Move
                            MoveListArray << MoveList3
                        Next MoveList2

                        ' Get rid of the current MoveList from MoveListArray
                        ' Back counter by one because the initiating sequence
                        ' was deleted
                        Delete MoveListArray, Counter
                        Counter = Counter - 1
                    EndIf
                EndIf

                Counter = Counter + 1

                If Counter > UBound(MoveListArray) Then
                    Exit Loop ' The While Loop
                EndIf

            Wend
        EndIf

        FindAllJumps = MoveListArray
    End ' Func FindAllJumps

    ' -------------------------------------------------------------------
    ' Function actually begins here
    '

    ' MoveList is an array of Moves
    Local MoveList

    ' MoveListArray is an array of MoveLists
    Local MoveListArray

    ' MasterMoveListArray is used to keep track of final move lists
    Local MasterMoveListArray

    ' Coordinates to move through the board
    Local x, y, dx

    Erase MasterMoveListArray

    ' Start by seeing if there are possible jumps
    For x = 1 to Squares
        For y = 1 to Squares

            ' Find a piece, then see if it can do a jump!
            If Sgn(Board(x, y)) = Side Then

                ' Start adding legal jumps starting with [x,y]
                MoveListArray = FindAllJumps(x, y)

                ' Copy MoveLists from MoveListArray into MasterMoveListArray
                For MoveList in MoveListArray
                    MasterMoveListArray << MoveList
                Next MoveList

            EndIf
        Next y
    Next x

    ' Check to see if there are no jumps were found - only then look for non-jumping
    ' moves
    If Empty (MasterMoveListArray) Then

        ' In this case, search for non-jumping moves
        For x = 1 to Squares
            For y = 1 to Squares
                If Sgn(Board(x,y)) = Side Then

                    ' Check to see if we can move a single square left or right
                    For dx in [-1, 1]
                        If ValidSingleMove (x, y, dx, Side) Then
                            MasterMoveListArray << [[x, y, x+dx, y+Side]]
                        EndIf

                        ' If the piece is a King, also see if we can move backwards
                        If Abs(Board(x,y)) = King then
                            If ValidSingleMove (x, y, dx, -Side) Then
                                MasterMoveListArray << [[x, y, x+dx, y-Side]]
                            EndIf
                        EndIf
                    Next dx
                EndIf
            Next y
        Next x
    EndIf

    ' Return the MoveList
    GenerateMoveList = MasterMoveListArray
End

' -------------------------------------------------------------------
' This function will allow the user to enter a move for a side
' It outputs a MoveList

' Basic sequence:
' 1. Get valid move list
' 2. Repeat until UserMoveList is a complete valid move list
' 3. Redraw board from scratch
' 4. Highlight squares chosen so far (UserMoveList)
' 5. Let user enter a new square (UserMove)
' 6. Confirm that the new move, when added on to growing UserMoveList, is valid

' The working sequence of user moves (kept in UserMoveList) is terminated
' by a Move with last two coordinates 0,0 (e.g. [1,1,0,0])
Func GetUserMove (Board, Side)

    ' Where the user is actually moving
    Local UserMove, UserMoveList

    ' All valid Moves
    Local ValidMoveListArray

    ' Working variables
    Local Move, ClickMove, MoveList, IsValid, Counter, Done, OldClickMove

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

        GetSquare = [1 + Int(Pen(1)/BoxSize),Squares - Int(Pen(2)/BoxSize),0,0]

        ' Stop Mouse mechanism
        Pen Off
    End

    ' ---------------------------------------------------------------
    ' This will change the background of a square to color SquareColor
    Sub HighlightSquare (x, y, SquareColor)

        ' Only highlight a valid square
        If (x >= 1) and (x <= Squares) And (y >= 1) And (y <= Squares) Then
            Paint (x-1)*BoxSize + 1, (Squares-y)*BoxSize + 1, SquareColor
        EndIf
    End ' Sub HighLightSquare (x,y)

    ' ------------------------------------------------------------------
    ' Function actually starts here

    ' Initialize variables
    Erase UserMoveList
    Erase UserMove
    ValidMoveListArray = GenerateMoveList (Board, Side)
    OldClickMove = [0,0,0,0]

    Done = False
    Repeat

        ' Only redraw current board if we are starting with a fresh movelist
        ' to save time

        If Empty(UserMoveList) Then
            DisplayBoard Board
            ' Inform use of which side is to move
            At BoxSize * (Squares + 0.25), TextHeight (PlayerName(Side))
            ' Set text color to match
            If Side = Black Then
                Color ColorBlack, ColorDarkSquare
            Else
                Color ColorWhite, ColorDarkSquare
            EndIf
            Print PlayerName(Side) ; " to move"
        EndIf

        ' Highlight squares choses so far
        ' Although for an incomplete move, [Move(2),Move(3)]=0,0
        ' the HighlightSquare function checks for that and will
        ' only highlight valid coordinates

        For Move in UserMoveList
            HighlightSquare Move(0), Move(1), ColorSelect
            HighlightSquare Move(2), Move(3), ColorSelect
        Next Move

        ' Allow user to select a square and highlight it
        ' Users are only allowed to select "Dark", squares, which
        ' have the properties that the (x+y) coordinate sum is even
        Repeat
            Repeat
                ClickMove = GetSquare
            Until (ClickMove(0) + ClickMove(1)) Mod 2 = 0
        Until ClickMove <> OldClickMove
        OldClickMove = ClickMove

        HighLightSquare ClickMove(0), ClickMove(1), ColorSelect

        ' If this is the first square selected, then just record it and make sure
        ' it is valid
        ' If it is the second, then start adding Moves into UserMoveList
        ' using the last square checked as the first square in this move
        If Empty(UserMoveList) Then

            ' Make sure it is a valid Move
            IsValid = False

            ' Compare this move against the opening sequences of all valid moves
            For MoveList in ValidMoveListArray

                If MoveList(0)(0)=ClickMove(0) And MoveList(0)(1)=ClickMove(1) Then
                    IsValid = True
                EndIf
            Next MoveList

            ' If this is a valid move, store the coordinates as the first
            ' move in UserMoveList
            ' Otherwise, leave UserMoveList Empty and beep and get a new choice
            If IsValid Then
                UserMoveList = [ClickMove]
            Else
                Beep
                OldClickMove = [0,0,0,0]
            EndIf
        Else

            ' Check to see if this is the second coordinate in a Move
            ' If so, complete the first entry in UserMoveList
            ' If not, create a whole new move with the last coordinates
            ' in the last entry of UserMoveList serving as the first
            ' coordinates in the new move, and the just-clicked square
            ' serving as the second coordinates
            If (UserMoveList(0)(2)=0) and (UserMoveList(0)(3)=0) then
                ' Check to see if this is a jump
                If (Abs(ClickMove(0)-UserMoveList(0)(0)) > 1) Or &
                    (Abs(ClickMove(1)-UserMoveList(0)(1)) > 1) Then
                    UserMoveList(0)(2)=(ClickMove(0)+UserMoveList(0)(0))/2
                    UserMoveList(0)(3)=(ClickMove(1)+UserMoveList(0)(1))/2

                    UserMoveList << [UserMoveList(0)(2),UserMoveList(0)(3), &
                    ClickMove(0),ClickMove(1)]
                Else
                    UserMoveList(0)(2)=ClickMove(0)
                    UserMoveList(0)(3)=ClickMove(1)
                EndIf
            Else
                ' Create a new move by adding on to the end
                Move = UserMoveList(UBound(UserMoveList))

                ' If this is a jump, see if we need to insert an intermediate move
                If (Abs(Move(2)-ClickMove(0))>1) Or (Abs(Move(3)-ClickMove(1))>1) Then
                    UserMoveList << [Move(2), Move(3), &
                    (Move(2)+ClickMove(0))/2, (Move(3)+ClickMove(1))/2]
                    UserMoveList << [(Move(2)+ClickMove(0))/2, (Move(3)+ClickMove(1))/2, &
                    ClickMove(0), ClickMove(1)]
                Else
                    UserMoveList << [Move(2), Move(3), ClickMove(0), ClickMove(1)]
                EndIf
            EndIf

            ' Now, see if this is a valid sequence from ValidMoveListArray

            ' OK - at this point, we have UserMoveList containing
            ' moves that have been completed so far - test them to
            ' see if they represent a valid sequence
            For MoveList in ValidMoveListArray
                ' Compare each MoveList against TempUserMoveList
                ' If we can get through one whole sequence, then
                ' it is valid
                IsValid = True
                For Counter = 0 to Min (UBound(MoveList), UBound(UserMoveList))
                    If MoveList(Counter) <> UserMoveList(Counter) Then
                        IsValid = False
                    EndIf
                Next

                ' If we get here, then we have confirmed that there is a sequence
                ' of valid moves in TempUserMoveList so far - no need to check for
                ' more
                If IsValid then
                    ' Check to make sure that for this move, the UserMoveList is
                    ' NOT a superset (ie UBound(UserMoveList) > UBound(MoveList))
                    ' of MoveList
                    If UBound(UserMoveList) > UBound(MoveList) Then
                        IsValid = False
                    Else
                        Exit For
                    EndIf
                EndIf
            Next MoveList

            ' If we get to here and the move is not valid, beep and get a new
            ' sequence of moves
            If Not IsValid Then
                Beep
                Erase UserMoveList
            EndIf
        EndIf


        ' Test to see if we have completed a sequence
        If Not Empty(UserMoveList) Then
            For MoveList in ValidMoveListArray
                If UserMoveList = MoveList Then
                    Done = True
                EndIf
            Next
        EndIf

    Until Done

    ' Return the move list
    GetUserMove = UserMoveList

End ' Func GetUserMove

' ------------------------------------------------------------------
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
Func MiniMax(Board, Side, Depth, DepthMax, Evaluator)

    Local MoveList, Move, Score, BestScore, NextMini, BestMoveList

    If Depth > DepthMax Then
        ' If we have exceeded the specified search depth,
        ' return the value of our board at this point and
        ' a null movelist
        MiniMax = [(EvaluateBoard (Board, Side, Evaluator)), 0]
    Else

        ' Find all possible moves for Side with the current Board
        MoveList = GenerateMoveList (Board, Side)

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
                NextMini = MiniMax(MakeMove(Board,Move,False), &
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

' ------------------------------------------------------------------------
' This function applies a custom evaluator to be used by MiniMax
' There should be a private function for each different approach
' to scoring the boards, and the main function code will chose among
' them based on Evaluator
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
                Score = Score + Board(x,y)
            Next y
        Next x

        ScoreSimple = Score * Side
    End ' Func ScoreSimple

    ' ------------------------------------------------------------------------
    ' This score adds to SimpleScore by giving bonus for gutter squares
    ' and a bonus for advancing pieces
    ' Written by Nat
    Func ScoreNat (Board, Side)
        Local x, y, Score

        ' Start with neutral score
        Score = 0

        For x = 1 to Squares
            For y = 1 to Squares
                Score = Score + Board(x,y)

                ' Bonus for being x = 1 or Squares
                If (x=1) or (x = Squares) Then
                    Score = Score + 0.5 * Board(x,y)
                EndIf

                ' Bonus for advancing to final row for Pieces only
                If Abs(Board(x,y)) = Piece then
                    If Side = 1 Then
                        Score = Score + 0.125 * y
                    Else
                        Score = Score - 0.125 * ((Squares + 1) - y)
                    EndIf
                EndIf

            Next y
        Next x

        ScoreNat = Score * Side
    End ' Func ScoreNat

    ' ------------------------------------------------------------------------
    ' This score adds to SimpleScore by giving penalty for gutter squares
    ' Written by Sam
    Func ScoreSam (Board, Side)
        Local x, y, Score

        ' Start with neutral score
        Score = 0

        For x = 1 to Squares
            For y = 1 to Squares
                Score = Score + Board(x,y)

                ' Penalty for being x = 1 or Squares
                If (x=1) or (x = Squares) Then
                    Score = Score - 0.5 * Board(x,y)
                EndIf

                ' Penalty for being in corner
                If ((x=1) or (x=Squares)) And ((y=1) or (y=Squares)) Then
                    Score = Score - 0.5 * Board(x,y)
                EndIf

            Next y
        Next x

        ScoreSam = Score * Side
    End ' Func ScoreSam

    ' ------------------------------------------------------------------------
    ' Function begins here

    ' The idea is that we have multiple Evaluator functions as a test
    ' of different strategies

    If Evaluator = 2 Then
        EvaluateBoard = ScoreSimple (Board, Side)
    ElseIf Evaluator = 3 Then
        EvaluateBoard = ScoreNat (Board, Side)
    ElseIf Evaluator = 4 Then
        EvaluateBoard = ScoreSam (Board, Side)
    EndIf
End ' Func EvaluateBoard


' -------------------------------------------------------------------
' This function will determine the next move for Side based on the
' strategy in PlayerStrategy(Side) and return a MoveList
Func DetermineMove (Board, Side)
    Local Temp

    ' This will display text to the right (by 1/4 square) of the board
    ' listing who is currently moving
    ' The color of the text will match the color of the pieces
    At BoxSize * (Squares + 0.25), TextHeight (PlayerName(Side))
    ' Set text color to match
    If Side = Black Then
        Color ColorBlack, ColorDarkSquare
    Else
        Color ColorWhite, ColorDarkSquare
    EndIf

    At BoxSize * (Squares + 0.25), TextHeight (PlayerName(Side))
    Print PlayerName(Side) + " to move"

    If PlayerStrategy(Side) = 1 Then
        DetermineMove = GetUserMove (Board, Side)
    ElseIf PlayerStrategy(Side) = 2 Then
        Temp = MiniMax(Board, Side, 0, 2, 2)
        DetermineMove = Temp(1)
    ElseIf PlayerStrategy(Side) = 3 Then
        Temp = MiniMax(Board, Side, 0, 3, 2)
        DetermineMove = Temp(1)
    ElseIf PlayerStrategy(Side) = 4 Then
        Temp = MiniMax(Board, Side, 0, 4, 2)
        DetermineMove = Temp(1)
    ElseIf PlayerStrategy(Side) = 5 Then
        Temp = MiniMax(Board, Side, 0, 5, 2)
        DetermineMove = Temp(1)
    ElseIf PlayerStrategy(Side) = 6 Then
        Temp = MiniMax(Board, Side, 0, 4, 3)
        DetermineMove = Temp(1)

        At (Squares + 0.25) * BoxSize, TextHeight ("1") * 3
        Print "Score :"; Temp(0)
    ElseIf PlayerStrategy(Side) = 7 Then
        Temp = MiniMax(Board, Side, 0, 4, 4)
        DetermineMove = Temp(1)

        At (Squares + 0.25) * BoxSize, TextHeight ("1") * 3
        Print "Score :"; Temp(0)
    EndIf
End ' Func DetermineMove

' -------------------------------------------------------------------
' This is the main control function for the checkers program
Sub Main

    Local Board, MoveList, Winner, Counter

    ' Get User information:
    ' For each color, get a unique name for the player and
    ' a strategy for determining the move to make

    For Counter in [White, Black]

        ' Get the name of the players
        Print "Please enter the name for the ";
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
            Print "2. Use SimpleScore Level 2"
            Print "3. Use SimpleScore Level 3"
            Print "4. Use SimpleScore Level 4"
            Print "5. Use SimpleScore Level 5"
            Print "6. Use NatScore Level 4"
            Print "7. Use SamScore Level 4"
            Print "Enter the strategy to use for "; PlayerName(Counter);
            Input PlayerStrategy(Counter)
        Until PlayerStrategy(Counter) in [1,2,3,4,5,6,7]

        Print
        Print
    Next Counter

    ' Initialize the board
    Board = InitBoard

    ' Display the board
    DisplayBoard Board

    ' Loop until game is over - it is over when one side cannot
    ' make any more moves
    While True

        ' Allow white to move is there is a move for White to make
        If Empty(GenerateMoveList (Board, White)) Then
            Winner = Black
            Exit Loop
        Else
            ' Get White Move
            MoveList = DetermineMove (Board, White)

            ' Apply White Move
            Board = MakeMove (Board, MoveList, True)

            ' Show the new board
            DisplayBoard Board
        EndIf

        ' Allow Black to move if there is a move for Black to make
        If Empty(GenerateMoveList (Board,Black)) Then
            Winner = White
            Exit Loop
        Else
            ' Get Black Move
            MoveList = DetermineMove (Board, Black)

            ' Apply Black Move
            Board = MakeMove (Board, MoveList, True)

            ' Show the new board
            DisplayBoard Board
        EndIf

    Wend

    ' Now, display the winner
    At BoxSize * (Squares + 0.25), TextHeight (PlayerName(Winner))
    Print PlayerName(Winner) ; " wins!"

    ' Play a little fanfare and pause for 1 second
    Play "O2CEGO3CP1"
    Play "p1"
End

' -------------------------------------------------------------------
' Call the Main procedure, then exit the program

' Declare array of player names
Dim PlayerName (Black to White)

' Declare array of strategy choices
Dim PlayerStrategy (Black to White)

Main
End
