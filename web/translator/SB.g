/** 
 * Convert BASIC programs of various dialects into SmallBASIC programs
 *
 * Copyright(C) 2009-2013 Chris Warren-Smith.
 *
 * This program is distributed under the terms of the GPL v2.0 or later
 * Download the GNU Public License (GPL) from www.gnu.org
 *
 */
grammar SB;

options {
  backtrack=true;
}

@header {
package net.sourceforge.smallbasic.translator;
import java.util.LinkedList;
import java.util.Queue;
import java.util.Set;
import java.util.HashSet;
import java.util.Iterator;
}

@lexer::header {
package net.sourceforge.smallbasic.translator;
}

@members {
TokenRewriteStream tokens; // avoid typecasts all over
}

program
@init {
  tokens = (TokenRewriteStream)input; 
  Token start = input.LT(1);
}
  : declaration+
  ;

declaration :
    argumentObject
  | arrayObject
  | clockObject
  | fileObject
  | graphicsWindowObject
  | mathObject
  | mouseObject
  | networkObject
  | programObject
  | shapesObject
  | soundObject
  | stackObject
  | textObject
  | textWindowObject
  | turtleObject
  | endKeywords
  | functionCall
  | ifTest
  | plainText
  ;

argumentObject
  : argumentObjectRef argumentObjectMethods
  ;

argumentObjectRef
  : m='Argument.' { tokens.replace($m, ""); }
  ;

argumentObjectMethods
  : m='Arguments'
  | m='GetArgument'
  ;

arrayObject
  : arrayObjectRef arrayObjectMethods
  ;

arrayObjectRef
  : m='Array.' { tokens.replace($m, ""); }
  ;

arrayObjectMethods
  : m='ContainsIndex' '(' v1=expr CO v2=expr e=')'
  | m='ContainsValue' '(' v1=expr CO v2=expr e=')'
  | m='GetItemCount' '(' v1=expr e=')'
  | m='GetValue' '(' v1=expr CO v2=expr e=')'
    { tokens.replace($m, $e, $v1.text + "[" + $v2.text + "]"); }
  | m='RemoveValue' '(' v1=expr CO v2=expr e=')'
  | m='SetValue' '(' v1=expr CO v2=expr CO v3=expr e=')'
    { tokens.replace($m, $e, $v1.text + "[" + $v2.text + "] = " + $v3.text); }
  ;

clockObject
  : clockObjectRef clockObjectMethods
  ;

clockObjectRef
  : m='Clock.' { tokens.replace($m, ""); }
  ;

clockObjectMethods
  : m='Date'
  | m='Day'
  | m='Hour'
  | m='Minute'
  | m='Month'
  | m='Second'
  | m='Time'
  | m='WeekDay'
  | m='Year'
  ;

fileObject
  : fileObjectRef fileObjectMethods
  ;

fileObjectRef
  : m='File.'  { tokens.replace($m, ""); }
  ;

fileObjectMethods
  : m='LastError'
  | m='AppendContents' '(' v1=expr CO v2=expr e=')'
  | m='CopyFile' '(' v1=expr CO v2=expr e=')'
  | m='CreateDirectory' '(' v1=expr e=')'
  | m='DeleteDirectory' '(' v1=expr e=')'
  | m='DeleteFile' '(' v1=expr e=')'
  | m='GetDirectories' '(' v1=expr CO v2=expr e=')'
  | m='GetFiles' '(' v1=expr CO v2=expr e=')'
  | m='GetSettingsFilePath'
  | m='GetTemporaryFilePath'
  | m='InsertLine' '(' v1=expr CO v2=expr CO v3=expr e=')'
  | m='ReadContents' '(' v1=expr e=')'
  | m='ReadLine' '(' v1=expr CO v2=expr e=')'
  | m='WriteContents' '(' v1=expr CO v2=expr e=')'
  | m='WriteLine' '(' v1=expr CO v2=expr CO v3=expr e=')'
  ;

graphicsWindowObject
  : graphicsWindowObjectRef graphicsWindowObjectMethods
  ;

graphicsWindowObjectRef
  : m='GraphicsWindow.' { tokens.replace($m, ""); }
  ;

graphicsWindowObjectMethods
  : m='AddRectangle' '(' v1=expr CO v2=expr e=')'
  | m='Clear'
    { tokens.replace($m, "cls"); }
  | m='KeyDown' '=' ID 
    { tokens.replace($m, "' TODO handle keydown"); }
  | b = 'BackgroundColor' // (e='=' v=variable)?
//    { tokens.replace($b, $e, "color "); }
  | m='Title' '=' e=STRING 
    { tokens.replace($m, $e, "env('TITLE=" + $e.text + "')"); }
  | m='Height'
  | m='Width'
  | m='LastKey' '=' variable
  | m='ShowMessage' '(' v1=expr CO v2=expr e=')'
    { tokens.replace($m, $e, "html '" + $v1.text + 
        "<br><br><input type=button value=OK>', 'Title', 20,20,260,80"); }
  | m='GetLeftOfShape' '(' v1=expr e=')'
  | m='GetTopOfShape' '(' v1=expr e=')'
  | m='Get'
  | m='MoveShape' '(' v1=expr CO v2=expr CO v3=expr e=')'
  | m='BrushColor'
  | m='CanResize'
  | m='FontBold'
  | m='FontItalic'
  | m='FontName'
  | m='FontSize'
  | m='PenColor'
  | m='PenWidth'
  | m='KeyUp'
  | m='MouseDown'
  | m='MouseMove'
  | m='MouseUp'
  | m='DrawBoundText' '(' v1=expr CO v2=expr CO v3=expr CO v4=expr e=')'
  | m='DrawEllipse' '(' v1=expr CO v2=expr CO v3=expr CO v4=expr e=')'
  | m='DrawImage' '(' v1=expr CO v2=expr CO v3=expr e=')'
  | m='DrawLine' '(' v1=expr CO v2=expr CO v3=expr CO v4=expr e=')'
  | m='DrawRectangle' '(' v1=expr CO v2=expr CO v3=expr CO v4=expr e=')'
  | m='DrawResizedImage' '(' v1=expr CO v2=expr CO v3=expr CO v4=expr CO v5=expr e=')'
  | m='DrawText' '(' v1=expr CO v2=expr CO v3=expr e=')'
  | m='DrawTriangle' '(' v1=expr CO v2=expr CO v3=expr CO v4=expr CO v5=expr CO v6=expr e=')'
  | m='FillEllipse' '(' v1=expr CO v2=expr CO v3=expr CO v4=expr e=')'
  | m='FillRectangle' '(' v1=expr CO v2=expr CO v3=expr CO v4=expr e=')'
  | m='FillTriangle' '(' v1=expr CO v2=expr CO v3=expr CO v4=expr CO v5=expr CO v6=expr e=')'
  | m='GetColorFromRGB' '(' v1=expr CO v2=expr CO v3=expr e=')'
    { tokens.replace($m, $e, "RGB(" + $v1.text + "," + $v2.text + "," + $v3.text + ")"); }
  | m='GetPixel' '(' v1=expr CO v2=expr e=')'
  | m='GetRandomColor'
  | m='RemoveShape' '(' v1=expr e=')'
  | m='SetPixel' '(' v1=expr CO v2=expr CO v3=expr e=')'
    { tokens.replace($m, $e, "PSet " + $v1.text + "," + $v2.text + "," + $v3.text ); }
  | m='Left'
  | m='MouseX'
  | m='MouseY'
  | m='Hide'
  | m='Show'
  | m='Top'
  ;

mathObject
  : mathObjectRef mathObjectMethods
  ;

mathObjectRef
  : m='Math.'  { tokens.replace($m, ""); }
  ;

mathObjectMethods
  : m='Abs' '(' v1=expr e=')'
  | m='Pi'
  | m='Ceiling' '(' v1=expr e=')'
  | m='Cos' '(' v1=expr e=')'
  | m='Floor' '(' v1=expr e=')'
  | m='GetDegrees' '(' v1=expr e=')'
  | m='GetRadians' '(' v1=expr e=')'
  | m='GetRandomVariable' '(' v1=expr e=')'
  | m='GetRandomNumber' '(' v1=expr e=')' 
    { tokens.replace($m, $e, "1 + ((Rnd*1000) \% " + $v1.text + ")"); }
  | m='Log' '(' v1=expr e=')'
  | m='Max' '(' v1=expr CO v2=expr e=')'
  | m='Min' '(' v1=expr CO v2=expr e=')'
  | m='NaturalLog' '(' v1=expr e=')'
  | m='Power' '(' v1=expr CO v2=expr e=')'
  | m='Remainder' '(' v1=expr CO v2=expr e=')'
  | m='Round' '(' v1=expr e=')'
  | m='Sin' '(' v1=expr e=')'
  | m='SquareRoot' '(' v1=expr e=')'
  | m='Tan' '(' v1=expr e=')'
  ;

mouseObject
  : mouseObjectRef mouseObjectMethods
  ;

mouseObjectRef
  : m='Mouse.' { tokens.replace($m, ""); }
  ;

mouseObjectMethods
  : m='IsLeftButtonDown'
  | m='IsRightButtonDown'
  | m='MouseX'
  | m='MouseY'
  | m='HideCursor'
  | m='ShowCursor'
  ;

networkObject
  : networkObjectRef networkObjectMethods
  ;

networkObjectRef
  : m='Network.' { tokens.replace($m, ""); }
  ;

networkObjectMethods
  : m='DownloadFile' '(' v1=expr e=')'
  | m='GetWebPageContents' '(' v1=expr e=')'
  ;

programObject
  : programObjectRef programObjectMethods
  ;

programObjectRef
  : m='Program.' { tokens.replace($m, ""); }
  ;

programObjectMethods
  : m='Directory'
  | m='Delay' '(' v1=expr e=')'
  | m='End'
  ;

shapesObject
  : shapesObjectRef shapesObjectMethods
  ;

shapesObjectRef
  : m='Shapes.' { tokens.replace($m, ""); }
  ;

shapesObjectMethods
  : m='AddRectangle' '(' v1=expr CO v2=expr e=')' 
  | m='AddEllipse' '(' v1=expr CO v2=expr e=')'
  | m='Animate' '(' v1=expr CO v2=expr CO v3=expr CO v4=expr e=')' 
  | m='GetLeft' '(' v1=expr e=')'
  | m='Move' '(' v1=expr CO v2=expr CO v3=expr e=')'
  ;

soundObject
  : soundObjectRef soundObjectMethods
  ;

soundObjectRef
  : m='Sound.' { tokens.replace($m, ""); }
  ;

soundObjectMethods
  : m='PlayBellRingAndWait' '(' e=')' { tokens.replace($m, $e, "Beep"); }
  | m='PlayClick' '(' ')'
  ;

stackObject
  : stackObjectRef stackObjectMethods
  ;

stackObjectRef
  : m='Stack.' { tokens.replace($m, ""); }
  ;

stackObjectMethods
  : m='GetCount' '(' v1=expr e=')'
  | m='PopValue' '(' v1=expr e=')'
  | m='PushValue' '(' v1=expr CO v2=expr e=')'
  ;

textObject
  : textObjectRef textObjectMethods
  ;

textObjectRef
  : m='Text.'  { tokens.replace($m, ""); }
  ;

textObjectMethods
  : m='Append' '(' v1=expr CO v2=expr e=')' 
    { tokens.replace($m, $e, $v1.text + " + " + $v2.text); }
  | m='ConvertToLowerCase' '(' v1=expr e=')'
  | m='ConvertToUpperCase' '(' v1=expr e=')'
  | m='EndsWith' '(' v1=expr CO v2=expr e=')'
  | m='GetCharacter' '(' v1=expr e=')'
  | m='GetCharacterCode' '(' v1=expr e=')'
  | m='GetLength' '(' v1=expr e=')'
  | m='GetSubText' '(' v1=expr CO v2=expr CO v3=expr e=')'
  | m='IsSubText' '(' v1=expr CO v2=expr e=')'
  | m='StartsWith' '(' v1=expr CO v2=expr e=')'
  ;

textWindowObject
  : textWindowObjectRef textWindowObjectMethods
  ;

textWindowObjectRef
  : m='TextWindow.' { tokens.replace($m, ""); }
  ;

textWindowObjectMethods 
  : m='CursorLeft'
  | m='CursorTop'
  | m='ForegroundColor'
  | m='Left'
  | m='Top'
  | m='Hide'
  | m='Pause'
  | m='PauseIfVisible'
  | m='PauseWithoutMessage'
  | m='Read'
  | m='ReadNumber'
  | m='Write' '(' v1=expr e=')' { tokens.replace($m, $e, "print " + $v1.text + ";"); }
  | m='BackgroundColor'
  | m='Title'
  | m='Clear' { tokens.replace($m, "cls"); }
  | m='Show'
  | m='WriteLine' '(' v1=expr e=')' { tokens.replace($m, $e, "print " + $v1.text); }
  ;

turtleObject
  : turtleObjectRef turtleObjectMethods
  ;

turtleObjectRef
  : m='Turtle.' { tokens.replace($m, ""); }
  ;

turtleObjectMethods 
  : m='Speed' '=' a1=plainText
  | m='Move' '(' v1=expr ')'
  | m='Turn' '(' v1=expr ')'
  ;

ifTest
  : ifStatement '(' ifExpr ')' ifExpr?
  | ifStatement ifExpr
  ;

ifStatement
  : ('If' | 'While')
  ;

ifExpr 
  : expr (logicalOperator expr)*
  ;

functionCall
  : m='(' v1=expr e=')' { 
      if ($v1.text.length() == 0) {
        tokens.replace($m, $e, "");
      }
    }
  ;

expr
  : term? (OPR term)*
  ;

term
  : ('(' expr ')' | variable)
  ;

variable
  : argumentObject
  | arrayObject
  | clockObject
  | fileObject
  | graphicsWindowObject
  | mathObject
  | mouseObject
  | stackObject
  | shapesObject
  | textObject
  | ID
  | NUM
  | STRING
  ;

endKeywords
  : m='EndWhile' { tokens.replace($m, "Wend"); }
  | m='EndIf'  { tokens.replace($m, "EndIf"); }
  | m='ElseIf' { tokens.replace($m, "ElseIf"); }
  | m='EndSub' { tokens.replace($m, "End Sub"); }
  | m='EndFor' { tokens.replace($m, "Next"); }
  ;

plainText
  : ID | NUM | OPR | PUNC | STRING | EQEQ | EQ
  ;

logicalOperator
  : ('='|'>'|'<'|'<='|'>='|'Or'|'And'|'&'|'|')
  ;

CO
  : (',')
  ;

STRING
  : '"' ( EscapeSequence | ~('\\'|'"') )* '"'
  ;

fragment EscapeSequence
  : '\\' ('b' | 't' | 'n' | 'f' | 'r' | '\"' | '\'' | '\\')
  ;

ID
  : ('a'..'z' | 'A'..'Z' | '_') ('a'..'z' | 'A'..'Z' | '0'..'9' | '_')* 
  ;

NUM
  : ('0'..'9' | '.')+ 
  ;

QUOTE
  : ( '"' )
  ;

PUNC
  : ( '_' | ':' | '.' | '>' | '<' | '[' | ']')* 
  ;

OPR
  : ('+' | '-' | '/' | '*') 
  ;

EQ
  : '=' 
  ;

EQEQ
  : '=='
  ;

WS
  : (' '|'\t'|'\n')+  { $channel=HIDDEN; } 
  ;

LINE_COMMENT
  : ('\'' | 'REM ' | 'rem ') ~('\n'|'\r')* '\r'? '\n' { $channel=HIDDEN; } 
  ;

