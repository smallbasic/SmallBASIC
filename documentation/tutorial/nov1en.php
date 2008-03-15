<?
	require("../wsblib.php"); 
	initPage();
	showPageTitle("", "Beginner's guide to SB - Part1");
?>

	<TR>
		<TD ID=TDB1>
		<? require("../leftmenu.php"); ?>
		</TD>

<!-- ----------------------------------------------------------------------------------------------------------------------- -->

		<TD  ID=TDB2>
		<p>				
			<p>
			<i>
			"What is computer programming? Nothing mysterious! Programming is simply communicating with a computer - 
			telling it what to do and when to do it. To program your computer you'll only learn two things: the
			language your computer understands, and the way you talk to it. No lengthy training periods or super-sophisticated
			skills are required."</i>
			<p>
			<div align=right>
			<font size=0>
			<b>"Texas Instruments TI-99/4A Computer. Beginner's BASIC"</b><br>Copyright (c)  1979, 1981 by Texas Instruments Incorporated
			</font>
			</div>
			<p>
			From the first book about computers that I was read.
			<p>
			<hr noshade>
			<p>The Palm, its keys and interesting places.<br><br>
			Before we start it is necessary to know where the keys, such as, MemoPad, Menu
and Applications are located.<br>
It is also necessary to have the knowledge to write text files into palm using
the plastic pen in the graffiti area.
			<p>
			<img src="kbe.gif">
			<p>
			<hr noshade>
			<p>
			<b>Our first program</b>
			<p>
			<hr noshade>
			<table><tr><td><img align=left src="startup.gif"></td><td>
			<p>
We turn on the Palm and we are ready to write our first program
Click on the SB icon...
			</td></tr></table><hr noshade>
			
			<table><tr><td><img align=left src="none.gif"></td><td>
			<p>
The first screen contains the list of our programs.  As you can see there is no
program yet because we didn't create any :)
			<br><br>
It is time to create one. Click on <b>NEW</b> at the bottom left of screen.			
			</td></tr></table><hr noshade>
			
			
			<table><tr><td><img align=left src="hello1.gif"></td><td>
			<p>
SB wants from us to give a pragram-name. That name is needed because we can store
more programs in our Palm.
			<br><br>
			In our example we use the name <b>hello</b>. So, we type the word <b>hello</b> and we click on <b>OK</b>
			</td></tr></table><hr noshade>
			
			<table><tr><td><img align=left src="hello2.gif"></td><td>
			<p>
SB changes screen.  We are on the screen where we can write the desired program.
			<br><br>
It is time to write the first commands to the palm.<br>
			Type:
			<xmp>
PRINT "Hello world"
			</xmp>
			<b>PRINT</b> means display, write something on the screen. This is the command to Palm.<br><br>
			Close to this command we write the parameters, what to print (in our example
"Hello world").
			<br><br>
The first two lines beginning with the sign ' are automatically added by SB and
are comments.  These notes are not commands to SB but comments we keep within
the program for our help.  SB will ignore any text line beginning with the ' sign.
			<br><br>
Now click on <b>S&R</b> (Save and Run = save the program and run its commands) located
at the bottom left of the screen.			
			</td></tr></table><hr noshade>
			
			<table><tr><td><img align=left src="hello3.gif"></td><td>
			<p>The result :)
			<br><br>
Palm execute the given command and inform us that the run procedure finished
with the word <b>* DONE *</b>.
			</td></tr></table><hr noshade>

			<table><tr><td><img align=left src="ideret.gif"></td><td>
			<p>It is time to return back to the screen where we write our commands..
			<br><br>
Click on MENU button, located outside of the screen of Palm and choose Close
			</td></tr></table><hr noshade>
	
			<p>
It is easy now to understand that programs are a sequence of commands to the
computer.
			<br><br>
SB offers a large variety of commands where you can type.  This catalog is
located at the first page of our site with title "SmallBASIC commands reference"
			<p>
			<hr noshade>
			<p><b>The syntax of commands</b>
			<p>
			<hr noshade>
			<p>
The computer is a machine, to understand what we ask we should type it with a
particular format.  We saw from the above example how we syntax the command
PRINT.
						<p>
			<img src="syntax1e.gif">
			<p>
double quotes indicate to the SB that what we type is a text.  This is required;
as we will see later we can give non-text parameters.
			<p>
What happened when we want to give more that one parameter?			
			<p>
			<img src="syntax2e.gif">
			<p>
So far so good... But why is needed to give more than one parameter?<br>
In our example there is no sense. Both the two commands are doing the same
thing.<br>
What happened if we want to display the result of an addition?
			<br>
			<xmp>
PRINT "The result of the addition 1+2 is =", 1+2
			</xmp>
The second parameter is an operation. When computer execute this command will
print on screen:
			<p>
			<xmp>
The result of the addition 1+2 is = 3
			</xmp>
			<p>
			<hr noshade>
			<p><b>Operations</b>
			<p>
			<hr noshade>				
			<p><b>Addition, abstraction, multiplication and division</b>
			<img align=right src="praxis.gif">
			<xmp>
PRINT 2+4, 4-2, 3*4, 10/5
			</xmp>
Operations do not have big difference as you type them by hand.
What you should always remember, is that the symbol of multiplication is the
asterisk sign (*)
			<p>
In our example we tell to computer to print the result of the following
operations:
			2+4, 4-2, 3*4, 10/5. It is expected to see the following result
			<p>
			<xmp>
	6    2    12   2
			</xmp>
			<P>
Note:<br>
Decimal numbers are indicated with the dot sign (.) not the comma sign (,) as
the English system. E.g.
PRINT "ð = ", 3.14
			
			<p>			
			<hr noshade>				
			<p><b>Operations Priority</b>
			<img align=right src="praxis2.gif">
			<p>
SB solve the operations with the logic where we have learned in school: firstly
the multiplication and division afterwards the addition and abstraction.  This
is logical and right, but how we can use more complicated operations?
			<p>Example<p>
			<img src="div.gif">
			<br>
in the above example, firstly we should make the addition 3+6 and afterwards to
divide the result by 3.  The addition 3+6 has priority in relation to division.
			<p>
To indicate this priority, we are using the brackets: (3+6)/3.
Suchlike are indicated more complex operations:
			<p>
			<img src="div2.gif"> = ((3+3)*2)/3
			<p>
			<hr noshade>				
			<p><b>Power</b>
			<p>
The power of a number is indicated with the ^
The 4 in power of 2 is indicated as 4^2, the 6 in power of 3 is indicated as
6^3.
<br>
When we want to declare very large or small numbers, we can use the "scientific"
syntax:. nEd, n = number, d = exponent of 10, e.g. 1E11 = 1 * 10 ^ 11
The E express the part "* 10^".
			<p>
			<hr noshade>
			<p><b>Variables</b>			
			<p>
			<hr noshade>
			<p>Variables are memory where is used to store particular values, e.g.
			<xmp>
		Á=5
		PRINT A
			</xmp>
			<p>
At the fist line we declare that the variable, named A, is equal to 5.<br>
At the second line we print the value of A variable.  Note that the parameter of
PRINT is without double quotes because at this time we ask of BASIC to print the
value of A variable.
			<p>
Imagine the computer's memory like a very big furniture full of  drawers.<br>
1. Each of the drawer is a variable.<br>
2. The label outside of each drawer is the name of the variable.<br>
3. Inside of each drawer we can store the value of each variable.
			<p>
In at the above example (A=5), we reserved a drawer, we wrote outside at the
label the name A and we store inside the number 5.
			<p>
Let see more variables
			<xmp>
		A=5
		B=8
		A=3
		PRINT A,B
			</xmp>
			<p>
Let see what happened into the memory.
			<p>
			<img src="a-b-e.gif">
			<p>
Finally the command PRINT will display on the screen the values of the drawers A
and B.  The result displayed on the screen will be 3 and 8
			<p>
			<hr noshade>
			<p><b>Our second program and the INPUT command</b>
			<p>
			<hr noshade>			
			<p>
"Very beautiful all these drawers but why they are needed?"<br><br>
Very soon we will see their use, firstly we will learn the <b>INPUT</b> command
			<p>
			<hr noshade>

			<table><tr><td><img align=left src="inp1.gif"></td><td>
			<p>Let write the following program:
			<p>
			INPUT "Your name please ", A<br>
			PRINT "Hello", A<br>
			<br>
			<p>What INPUT command is doing?
			<p>
This command puts our Palm to wait from the user (us) to type a value.  The
value typed from the user will be stored into the variable (in our example A)<br>
			<br>
Now click on the word S&R (Save and Run) located at the bottom left of the
screen.
			</td></tr></table><hr noshade>
			
			<table><tr><td><img align=left src="inp2.gif"></td><td>
			<p>
Palm started to execute the given commands and now waits at the INPUT command.<br>
<br>
It waits from us to type our name. Type your name.
			</td></tr></table><hr noshade>
			
			<table><tr><td><img align=left src="inp3.gif"></td><td>
			<p>Here is mine...
			<p>
Now we should tell to INPUT command that OK we wrote our name, go to the next
command.  To understand the INPUT command that the input procedure is finished
we should press ENTER, meaning that we should draw a line in the graffiti place
from top right to the bottom left.
			<p>
			<img src="enter.gif">
			</td></tr></table><hr noshade>
			
			<table><tr><td><img align=left src="inp4.gif"></td><td>
			<p>
After the given ENTER , the INPUT command stores your name into the variable A.<br>
<br>
Now we can see the result of the PRINT "Hello", A
			<br>
			<p>That's it...
			</td></tr></table><hr noshade>

			
			<p>
Let see a more serious program, which is converting temperature degrees
Fahrenheit and Celsius.
			<p>
			<hr noshade>
			<table><tr><td><img align=left src="ftoc1.gif"></td><td>
			<p>
We know, if you don't you learn now, that 5/9* (Fahrenheit degrees -32) =
Celsius degrees.  We will use the letter C for Celsius degrees and F for
Fahrenheit degrees.
			<br><br>
			We type the following program:<br>
			<br>
			F=87 ' we declare into the program that the degrees F we want to convert are 87<br>
			C=5/9*(F-32) ' we declare the operation of C<br>
			' and we display the result<br>
			PRINT "The"<br>
			PRINT F, "degrees Fahrenheit are"<br>
			PRINT C, "degrees Celsius"<br>
			<br>
Now press on S&R (Save and Run) located at the bottom left of the screen.
			</td></tr></table><hr noshade>
			
			<table><tr><td><img align=left src="ftoc2.gif"></td><td>
			<p>Look at the result :)
			<br><br>
Is it right? Who knows... if I wrote the right furmula... :)
<br><br>
We go back into the code (MENU and Close) to use the INPUT command!
			</td></tr></table><hr noshade>

			<table><tr><td><img align=left src="ftoc3.gif"></td><td>
			<p>
Replace the command f-87 with the INPUT F, in order to ask every time we run the
program for a value of F we want to convert.
			<p>
Now press on S&R (Save and Run) located at the bottom left of the screen.
			</td></tr></table><hr noshade>
			
			<table><tr><td><img align=left src="ftoc4.gif"></td><td>
			<p>
The program started and waits from us to type a value for F.
			</td></tr></table><hr noshade>
			
			<table><tr><td><img align=left src="ftoc5.gif"></td><td>
			<p>At this example, I gave the value 60.

			<p>Here is the result.
			</td></tr></table><hr noshade>

			<p>
			End of first part.<br>
In the next part, we will talk about commands for checking the flow and the
loops of a program.<br>
These are commands, which are converting our Palm into a
very powerful tool.
			<p>
		</TD>
	</TR>

<? closePage(); ?>

