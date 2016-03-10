/* txt2c - SB CLI Help */
/* structure */
typedef struct { const char *code; const char *type; const char *syntax; const char *descr; } help_node_t;

help_node_t help_data[] = {
{ "#!...", "MACRO", 
"_\b#_\b!_\b._\b._\b.", 
"          Used by Unix to make source runs as a script executable\n"
"\n"
},

{ "#sec:", "MACRO", 
"_\b#_\bs_\be_\bc_\b: _\bs_\be_\bc_\bt_\bi_\bo_\bn_\b-_\bn_\ba_\bm_\be", 
"          Used internally to store the section name. Sections names are\n"
"          used at limited OSes like PalmOS for multiple 32kB source code\n"
"          sections. With a few words _\bD_\bO_\b _\bN_\bO_\bT_\b _\bU_\bS_\bE_\b _\bI_\bT!\n"
"\n"
},

{ "#inc:", "MACRO", 
"_\b#_\bi_\bn_\bc_\b: _\bf_\bi_\bl_\be", 
"          Used to include a SmallBASIC source file into the current\n"
"          BASIC code\n"
"\n"
},

{ "#unit-path:", "MACRO", 
"_\b#_\bu_\bn_\bi_\bt_\b-_\bp_\ba_\bt_\bh_\b: _\bp_\ba_\bt_\bh", 
"          Used to setup additional directories for searching for\n"
"          unit-files This meta does nothing more than to setting up the\n"
"          environment variable SB_UNIT_PATH. Directories on Unix must\n"
"          separated by \':\', and on DOS/Windows by \';\'\n"
"\n"
"   Examples\n"
"\n"
"...\n"
"#inc:\"mylib.bas\"\n"
"...\n"
"MyLibProc \"Hi\"\n"
"\n"
"2.7 Arrays and Matrices\n"
"\n"
"   Define a 3x2 matrix\n"
"\n"
"A = [11, 12; 21, 22; 31, 32]\n"
"\n"
"   That creates the array\n"
"\n"
"| 11  12 |\n"
"| 21  22 | = A\n"
"| 31  32 |\n"
"\n"
"   The comma used to separate column items; the semi-colon used to\n"
"   separate rows. Values between columns can be omitted.\n"
"\n"
"A = [ ; ; 1, 2 ; 3, 4, 5]\n"
"\n"
"   This creates the array\n"
"\n"
"| 0  0  0 |\n"
"| 1  2  0 | = A\n"
"| 3  4  5 |\n"
"\n"
"   Supported operators:\n"
"\n"
"   Add/sub:\n"
"\n"
"B = [1, 2; 3, 4]: C = [5, 6; 7, 8]\n"
"\n"
"A = B + C\n"
"C = A - B\n"
"\n"
"   Equal:\n"
"\n"
"bool=(A=B)\n"
"\n"
"   Unary:\n"
"\n"
"A2 = -A\n"
"\n"
"   Multiplication:\n"
"\n"
"A = [1, 2; 3, 4]: B = [5 ; 6]\n"
"C = A * B\n"
"D = 0.8 * A\n"
"\n"
"   Inverse:\n"
"\n"
"A = [ 1, -1, 1; 2, -1, 2; 3, 2, -1]\n"
"? INVERSE(A)\n"
"\n"
"   Gauss-Jordan:\n"
"\n"
"? \"Solve this:\"\n"
"? \"  5x - 2y + 3z = -2\"\n"
"? \" -2x + 7y + 5z =  7\"\n"
"? \"  3x + 5y + 6z =  9\"\n"
"?\n"
"A = [ 5, -2, 3; -2, 7, 5; 3, 5, 6]\n"
"B = [ -2; 7; 9]\n"
"C = LinEqn(A, B)\n"
"? \"[x;y;z] = \"; C\n"
"\n"
"   There is a problem with 1 dimension arrays, because 1-dim arrays does\n"
"   not specify how SmallBASIC must see them.\n"
"\n"
"DIM A(3)\n"
"\n"
"| 1 2 3 | = A\n"
"\n"
"or\n"
"\n"
"| 1 |\n"
"| 2 | = A\n"
"| 3 |\n"
"\n"
"   And because this is not the same thing. (ex. for multiplication) So\n"
"   the default is columns\n"
"\n"
"DIM A(3) \' or A(1,3)\n"
"\n"
"| 1 2 3 | = A\n"
"\n"
"   For vertical arrays you must declare it as 2-dim arrays Nx1\n"
"\n"
"DIM A(3,1)\n"
"\n"
"| 1 |\n"
"| 2 | = A\n"
"| 3 |\n"
"\n"
"2.8 Nested arrays\n"
"\n"
"   Nested arrays are allowed\n"
"\n"
"A = [[1,2] , [3,4]]\n"
"B = [1, 2, 3]\n"
"C = [4, 5]\n"
"B(2) = C\n"
"print B\n"
"\n"
"   This will be printed\n"
"\n"
"[1, 2, [4, 5], 3]\n"
"\n"
"   You can access them by using a second (or third, etc) pair of\n"
"   parenthesis.\n"
"\n"
"B(2)(1) = 16\n"
"print B(2)(1)\n"
"\n"
"Result:\n"
"    16\n"
"\n"
"2.9 The operator IN\n"
"\n"
"   IN operator is used to compare if the left-value belongs to\n"
"   right-value.\n"
"\n"
"\' Using it with arrays\n"
"print 1 in [2,3]        :REM FALSE\n"
"print 1 in [1,2]        :REM TRUE\n"
"print \"b\" in [\"a\", \"b\", \"c\"] :REM TRUE\n"
"...\n"
"\' Using it with strings\n"
"print \"na\" in \"abcde\"   :REM FALSE\n"
"print \"cd\" in \"abcde\"   :REM TRUE\n"
"...\n"
"\' Using it with number (true only if left = right)\n"
"print 11 in 21          :REM FALSE\n"
"print 11 in 11          :REM TRUE\n"
"...\n"
"\' special case\n"
"\' auto-convert integers/reals\n"
"print 12 in \"234567\"    :REM FALSE\n"
"print 12 in \"341256\"    :REM TRUE\n"
"\n"
"2.10 The operator LIKE\n"
"\n"
"   LIKE is a regular-expression operator. It is compares the left part\n"
"   of the expression with the pattern (right part). Since the original\n"
"   regular expression code is too big (for handhelds), I use only a\n"
"   subset of it, based on an excellent old stuff by J. Kercheval\n"
"   (match.c, public-domain, 1991). But there is an option to use PCRE\n"
"   (Perl-Compatible Regular Expression library) on systems that is\n"
"   supported (Linux); (see OPTION).\n"
"\n"
"   The same code is used for filenames (FILES(), DIRWALK) too.\n"
"\n"
"   In the pattern string:\n"
"\n"
"   *                matches any sequence of characters (zero or more)\n"
"   ?                matches any character\n"
"   [SET]            matches any character in the specified set,\n"
"   [!SET] or [^SET] matches any character not in the specified set.\n"
"\n"
"   A set is composed of characters or ranges; a range looks like\n"
"   character hyphen character (as in 0-9 or A-Z). [0-9a-zA-Z_] is the\n"
"   minimal set of characters allowed in the [..] pattern construct.\n"
"\n"
"   To suppress the special syntactic significance of any of `[]*?!^-\\\',\n"
"   and match the character exactly, precede it with a `\\\'.\n"
"\n"
"? \"Hello\" LIKE \"*[oO]\" : REM TRUE\n"
"? \"Hello\" LIKE \"He??o\" : REM TRUE\n"
"? \"Hello\" LIKE \"hello\" : REM FALSE\n"
"? \"Hello\" LIKE \"[Hh]*\" : REM TRUE\n"
"\n"
"2.11 The pseudo-operator <<\n"
"\n"
"   This operator can be used to append elements to an array.\n"
"\n"
"A << 1\n"
"A << 2\n"
"A << 3\n"
"\n"
"? A(1)\n"
"\n"
"2.12 Subroutines and Functions\n"
"\n"
"   _\bS_\by_\bn_\bt_\ba_\bx_\b _\bo_\bf_\b _\bp_\br_\bo_\bc_\be_\bd_\bu_\br_\be_\b _\b(_\bS_\bU_\bB_\b)_\b _\bs_\bt_\ba_\bt_\be_\bm_\be_\bn_\bt_\bs\n"
"\n"
"SUB name [([BYREF] par1 [, ...[BYREF] parN)]]\n"
"  [LOCAL var[, var[, ...]]]\n"
"  [EXIT SUB]\n"
"  ...\n"
"END\n"
"\n"
"   _\bS_\by_\bn_\bt_\ba_\bx_\b _\bo_\bf_\b _\bf_\bu_\bn_\bc_\bt_\bi_\bo_\bn_\b _\b(_\bF_\bU_\bN_\bC_\b)_\b _\bs_\bt_\ba_\bt_\be_\bm_\be_\bn_\bt_\bs\n"
"\n"
"FUNC name[([BYREF] par1 [, ...[BYREF] parN)]]\n"
"  [LOCAL var[, var[, ...]]]\n"
"  [EXIT FUNC]\n"
"  ...\n"
"  name=return-value\n"
"END\n"
"\n"
"   On functions you must use the function\'s name to return the value.\n"
"   That is, the function-name acts like a variable and it is the\n"
"   function\'s returned value.\n"
"\n"
"   The parameters are \'by value\' by default. Passing parameters by value\n"
"   means the executor makes a copy of the parameter to stack. The value\n"
"   in caller\'s code will not be changed.\n"
"\n"
"   Use BYREF keyword for passing parameters \'by reference\'. Passing\n"
"   parameters by reference means the executor push the pointer of\n"
"   variable into the stack. The value in caller\'s code will be the\n"
"   changed.\n"
"\n"
"\' Passing \'x\' by value\n"
"SUB F(x)\n"
"  x=1\n"
"END\n"
"\n"
"x=2\n"
"F x\n"
"? x:REM displays 2\n"
"\n"
"\' Passing \'x\' by reference\n"
"SUB F(BYREF x)\n"
"  x=1\n"
"END\n"
"\n"
"x=2\n"
"F x\n"
"? x:REM displays 1\n"
"\n"
"   You can use the symbol \'@\' instead of BYREF. There is no difference\n"
"   between @ and BYREF.\n"
"\n"
"SUB F(@x)\n"
"  x=1\n"
"END\n"
"\n"
"   On a multi-section (PalmOS) applications sub/funcs needs declaration\n"
"   on the main section.\n"
"\n"
"#sec:Main\n"
"declare func f(x)\n"
"\n"
"#sec:another section\n"
"func f(x)\n"
"...\n"
"end\n"
"\n"
"   Use the LOCAL keyword for local variables. LOCAL creates variables\n"
"   (dynamic) at routine\'s code.\n"
"\n"
"SUB MYPROC\n"
"  LOCAL N:REM LOCAL VAR\n"
"  N=2\n"
"  ? N:REM displays 2\n"
"END\n"
"\n"
"N=1:REM GLOBAL VAR\n"
"MYPROC\n"
"? N:REM displays 1\n"
"\n"
"   You can send arrays as parameters.\n"
"\n"
"   When using arrays as parameters its better to use them as BYREF;\n"
"   otherwise their data will be duplicated in memory space.\n"
"\n"
"SUB FBR(BYREF tbl)\n"
"  ? FRE(0)\n"
"  ...\n"
"END\n"
"\n"
"SUB FBV(tbl)\n"
"  ? FRE(0)\n"
"  ...\n"
"END\n"
"\n"
"\' MAIN\n"
"DIM dt(128)\n"
"...\n"
"? FRE(0)\n"
"FBR dt\n"
"? FRE(0)\n"
"FBV dt\n"
"? FRE(0)\n"
"\n"
"   Passing & returning arrays, using local arrays.\n"
"\n"
"func fill(a)\n"
"  local b, i\n"
"\n"
"  dim b(16)\n"
"  for i=0 to 16\n"
"    b(i)=16-a(i)\n"
"  next\n"
"  fill=b\n"
"end\n"
"\n"
"DIM v()\n"
"v=fill(v)\n"
"\n"
"2.13 Single-line Functions\n"
"\n"
"   There is also an alternative FUNC/DEF syntax (single-line functions).\n"
"   This is actually a macro for compatibility with the BASIC\'s DEF FN\n"
"   command, but quite usefull.\n"
"\n"
"   _\bS_\by_\bn_\bt_\ba_\bx:\n"
"\n"
"FUNC name[(par1[,...])] = expression\n"
"or\n"
"DEF name[(par1[,...])] = expression\n"
"\n"
"DEF MySin(x) = SIN(x)\n"
"? MySin(pi/2)\n"
"\n"
"2.14 Nested procedures and functions\n"
"\n"
"   One nice feauture, are the nested procedures/functions. The nested\n"
"   procedures/functions are visible only inside the \"parent\"\n"
"   procedure/function.\n"
"\n"
"   There is no way to access a global procedure with the same name of a\n"
"   local... yet...\n"
"\n"
"FUNC f(x)\n"
"    Rem Function: F/F1()\n"
"    FUNC f1(x)\n"
"        Rem Function: F/F1/F2()\n"
"        FUNC f2(x)\n"
"            f2=cos(x)\n"
"        END\n"
"        f1 = f2(x)/4\n"
"    END\n"
"    Rem Function: F/F3()\n"
"    FUNC f3\n"
"        f3=f1(pi/2)\n"
"    END\n"
"REM\n"
"? f1(pi) : REM OK\n"
"? f2(pi) : REM ERROR\n"
"f = x + f1(pi) + f3 : REM OK\n"
"END\n"
"\n"
"2.15 Units (SB libraries)\n"
"\n"
"   * Linux ONLY for now *\n"
"\n"
"   Units are a set of procedures, functions and/or variables that can be\n"
"   used by another SB program or SB unit. The main section of the unit\n"
"   (commands out of procedure or function bodies) is the initialization\n"
"   code.\n"
"\n"
"   A unit declared by the use of UNIT keyword.\n"
"\n"
"UNIT MyUnit\n"
"\n"
"   The functions, procedure or variables which we want to be visible to\n"
"   another programs must be declared with the EXPORT keyword.\n"
"\n"
"UNIT MyUnit\n"
"EXPORT MyF\n"
"...\n"
"FUNC MyF(x)\n"
"...\n"
"END\n"
"\n"
"   _\b* _\bK_\be_\be_\bp_\b _\bf_\bi_\bl_\be_\b-_\bn_\ba_\bm_\be_\b _\ba_\bn_\bd_\b _\bu_\bn_\bi_\bt_\b-_\bn_\ba_\bm_\be_\b _\bt_\bh_\be_\b _\bs_\ba_\bm_\be_\b._\b _\bT_\bh_\ba_\bt_\b _\bh_\be_\bl_\bp_\bs_\b _\bt_\bh_\be_\b _\bS_\bB_\b _\bt_\bo\n"
"   _\ba_\bu_\bt_\bo_\bm_\ba_\bt_\bi_\bc_\ba_\bl_\bl_\by_\b _\br_\be_\bc_\bo_\bm_\bp_\bi_\bl_\be_\b _\bt_\bh_\be_\b _\br_\be_\bq_\bu_\bi_\br_\be_\bd_\b _\bu_\bn_\bi_\bt_\bs_\b _\bw_\bh_\be_\bn_\b _\bi_\bt_\b _\bi_\bs_\b _\bn_\be_\be_\bd_\be_\bd_\b.\n"
"\n"
"   To link a program with a unit we must use the IMPORT keyword.\n"
"\n"
"IMPORT MyUnit\n"
"\n"
"   To access a member of a unit we must use the unit-name, a point and\n"
"   the name of the member.\n"
"\n"
"IMPORT MyUnit\n"
"...\n"
"PRINT MyUnit.MyF(1/1.6)\n"
"\n"
"   Full example:\n"
"\n"
"   file my_unit.bas:\n"
"\n"
"UNIT MyUnit\n"
"\n"
"EXPORT F, V\n"
"\n"
"REM a shared function\n"
"FUNC F(x)\n"
"    F = x*x\n"
"END\n"
"\n"
"REM a non-shared function\n"
"FUNC I(x)\n"
"    I = x+x\n"
"END\n"
"\n"
"REM Initialization code\n"
"V=\"I am a shared variable\"\n"
"L=\"I am invisible to the application\"\n"
"PRINT \"Unit \'MyUnit\' initialized :)\"\n"
"\n"
"   file my_app.bas:\n"
"\n"
"IMPORT MyUnit\n"
"\n"
"PRINT MyUnit.V\n"
"PRINT MyUnit.F(2)\n"
"\n"
"2.16 The pseudo-operators ++/--/p=\n"
"\n"
"   The ++ and -- operators are used to increase or decrease the value of\n"
"   a variable by 1.\n"
"\n"
"x = 4\n"
"x ++ : REM x <- x + 1 = 5\n"
"x -- : REM x <- x - 1 = 4\n"
"\n"
"   The generic p= operators are used as in C Where p any character of\n"
"   -+/\\*^%&|\n"
"\n"
"x += 4 : REM x <- x + 4\n"
"x *= 4 : REM x <- x * 4\n"
"\n"
"   All these pseudo-operators are not allowed inside of expressions\n"
"\n"
"y = x ++ \' ERROR\n"
"z = (y+=4)+5 \' ALSO ERROR\n"
"\n"
"2.17 The USE keyword\n"
"\n"
"   This keyword is used on specific commands to passing a user-defined\n"
"   expression.\n"
"\n"
"   Example:\n"
"\n"
"SPLIT s,\" \",v USE TRIM(x)\n"
"\n"
"   In that example, every element of V() will be \'trimmed\'.\n"
"\n"
"   Use the x variable to specify the parameter of the expression. If the\n"
"   expression needs more parameter, you can use also the names y and z\n"
"\n"
"2.18 The DO keyword\n"
"\n"
"   This keyword is used to declare single-line commands. It can be used\n"
"   with WHILE and FOR-family commands.\n"
"\n"
"   Example:\n"
"\n"
"FOR f IN files(\"*.txt\") DO PRINT f\n"
"...\n"
"WHILE i < 4 DO i ++\n"
"\n"
"   Also, it can be used by IF command (instead of THEN), but is not\n"
"   suggested.\n"
"\n"
"                             3. Programming Tips\n"
"\n"
"   Programmers must use clean and logical code. Weird code may be faster\n"
"   but it is not good.\n"
"\n"
"3.1 Using LOCAL variables\n"
"\n"
"   When a variable is not declared it is by default a global variable. A\n"
"   usual problem is that name may be used again in a function or\n"
"   procedure.\n"
"\n"
"FUNC F(x)\n"
"  FOR i=1 TO 6\n"
"    ...\n"
"  NEXT\n"
"END\n"
"\n"
"FOR i=1 TO 10\n"
"  PRINT F(i)\n"
"NEXT\n"
"\n"
"   In this example, the result is a real mess, because the i of the main\n"
"   loop will always (except the first time) have the value 6!\n"
"\n"
"   This problem can be solved if we use the LOCAL keyword to declare the\n"
"   i in the function body.\n"
"\n"
"FUNC F(x)\n"
"  LOCAL i\n"
"\n"
"  FOR i=1 TO 6\n"
"    ...\n"
"  NEXT\n"
"END\n"
"\n"
"FOR i=1 TO 10\n"
"  PRINT F(i)\n"
"NEXT\n"
"\n"
"   It is good to declare all local variables on the top of the function.\n"
"   For compatibility reasons, the func./proc. variables are not declared\n"
"   as \'local\' by default. That it is WRONG but as I said ...\n"
"   compatibility.\n"
"\n"
"3.2 Loops and variables\n"
"\n"
"   When we write loops it is much better to initialize the counters on\n"
"   the top of the loop instead of the top of the program or nowhere.\n"
"\n"
"i = 0\n"
"REPEAT\n"
"  ...\n"
"  i = i + 1\n"
"UNTIL i > 10\n"
"\n"
"   Initializing the variables at the top of the loop, can make code\n"
"   better readable, and can protect us from usual pitfalls such as\n"
"   forgeting to giving init value or re-run the loop without reset the\n"
"   variables.\n"
"\n"
"3.3 Loops and expressions\n"
"\n"
"   FOR-like commands are evaluate the \'destination\' everytime. Also,\n"
"   loops are evaluate the exit-expression everytime too.\n"
"\n"
"FOR i=0 TO LEN(FILES(\"*.txt\"))-1\n"
"    PRINT i\n"
"NEXT\n"
"\n"
"   In that example the \'destination\' is the LEN(FILES(\"*.txt\"))-1 For\n"
"   each value of i the destination will be evaluated. That is WRONG but\n"
"   it is supported by BASIC and many other languages.\n"
"\n"
"   So, it is much better to be rewritten as\n"
"\n"
"idest=LEN(FILES(\"*.txt\"))-1\n"
"FOR i=0 TO idest\n"
"    PRINT i\n"
"NEXT\n"
"\n"
"   Of course, it is much faster too.\n"
"\n"
"                                 4. Commands\n"
"\n"
},

{ "OPTION", "STATEMENT", 
"_\bO_\bP_\bT_\bI_\bO_\bN _\bk_\be_\by_\bw_\bo_\br_\bd_\b _\bp_\ba_\br_\ba_\bm_\be_\bt_\be_\br_\bs", 
"          This special command is used to pass parameters to the\n"
"          SB-environment. There are two styles for that, the run-time\n"
"          (like BASE) which can change the value at run-time, and the\n"
"          compile-time (like PREDEF) which used only in compile-time and\n"
"          the value cannot be changed on run-time.\n"
"\n"
"  2.5.1 Run-Time\n"
"\n"
},

{ "OPTION", "STATEMENT", 
"_\bO_\bP_\bT_\bI_\bO_\bN _\bB_\bA_\bS_\bE_\b _\bl_\bo_\bw_\be_\br_\b-_\bb_\bo_\bu_\bn_\bd", 
"          The OPTION BASE statement sets the lowest allowable subscript\n"
"          of arrays to lower-bound. The default is zero. The OPTION BASE\n"
"          statement can be used in any place in the source code but that\n"
"          is the wrong use of this except if we have a _\bg_\bo_\bo_\bd reason. In\n"
"          most cases the OPTION BASE must declared at first lines of the\n"
"          program before any DIM declaration.\n"
"\n"
},

{ "OPTION", "STATEMENT", 
"_\bO_\bP_\bT_\bI_\bO_\bN _\bM_\bA_\bT_\bC_\bH_\b _\b{_\bP_\bC_\bR_\bE_\b _\b[_\bC_\bA_\bS_\bE_\bL_\bE_\bS_\bS_\b]_\b|_\bS_\bI_\bM_\bP_\bL_\bE_\b}", 
"          Sets as default matching algorithm to (P)erl-(C)ompatible\n"
"          (R)egular (E)xpressions library or back to simple one.\n"
"          Matching-algorithm is used in LIKE and FILES.\n"
"\n"
"          PRCE works only in systems with this library and it must be\n"
"          linked with. Also, there is no extra code on compiler which\n"
"          means that SB compiles the pattern everytime it is used.\n"
"\n"
"  2.5.2 Compile-Time\n"
"\n"
},

{ "OPTION", "STATEMENT", 
"_\bO_\bP_\bT_\bI_\bO_\bN _\bP_\bR_\bE_\bD_\bE_\bF_\b _\bp_\ba_\br_\ba_\bm_\be_\bt_\be_\br", 
"          Sets parameters of the compiler. Where parameter\n"
"\n"
"        `QUITE\'\n"
"               Sets the quite flag (-q option)\n"
"        `COMMAND cmdstr\'\n"
"               Sets the COMMAND$ string to cmdstr (useful for debug\n"
"               reasons)\n"
"        `GRMODE [widthxheight[xbpp]]\'\n"
"               Sets the graphics mode flag (-g option) or sets the\n"
"               prefered screen resolution. Example: (Clie HiRes)\n"
"\n"
"OPTION PREDEF GRMODE 320x320x16\n"
"\n"
"        `TEXTMODE\'\n"
"               Sets the text mode flag (-g- option)\n"
"        `CSTR\'\n"
"               Sets as default string style the C-style special\n"
"               character encoding (\'\\\')\n"
"\n"
"2.6 Meta-commands\n"
"\n"
},

{ "REM", "STATEMENT", 
"_\bR_\bE_\bM _\bc_\bo_\bm_\bm_\be_\bn_\bt", 
"          Adds explanatory text to a program listing. comment commentary\n"
"          text, ignored by BASIC.\n"
"\n"
"          Instead of the keyword we can use the symbol \' or the #. The #\n"
"          can be used as remarks only if its in the first character of\n"
"          the line.\n"
"\n"
"          Example:\n"
"\n"
"\' That text-line is just a few remarks\n"
"...\n"
"REM another comment\n"
"...\n"
"# one more comment\n"
"\n"
},

{ "LET", "STATEMENT", 
"_\bL_\bE_\bT _\bv_\ba_\br_\b _\b=_\b _\be_\bx_\bp_\br", 
"          Assigns the value of an expression to a variable. The LET is\n"
"          optional.\n"
"\n"
"        var\n"
"               A valid variable name.\n"
"        expr\n"
"               The value assigned to variable.\n"
"\n"
"          Example:\n"
"\n"
"LET x = 4\n"
"x = 1               \' Without the LET keyword\n"
"z = \"String data\"   \' Assign string\n"
"...\n"
"DIM v(4)\n"
"z=v                 \' Assign array (z = clone of v)\n"
"\n"
},

{ "CONST", "STATEMENT", 
"_\bC_\bO_\bN_\bS_\bT _\bn_\ba_\bm_\be_\b _\b=_\b _\be_\bx_\bp_\br", 
"          Declares a constant.\n"
"\n"
"        name\n"
"               An identifier that follows the rules for naming BASIC\n"
"               variables.\n"
"        expr\n"
"               An expression consisting of literals, with or without\n"
"               operators, only.\n"
"\n"
"          Example:\n"
"\n"
"COSNT G = 6.67259E-11\n"
"\n"
},

{ "DIM", "STATEMENT", 
"_\bD_\bI_\bM _\bv_\ba_\br_\b(_\b[_\bl_\bo_\bw_\be_\br_\b _\bT_\bO_\b]_\b _\bu_\bp_\bp_\be_\br_\b _\b[_\b,_\b _\b._\b._\b._\b]_\b)_\b _\b[_\b,_\b _\b._\b._\b._\b]", 
"          The DIM statement reserves space in computer\'s memory for\n"
"          arrays. The array will have (upper-lower)+1 elements. If the\n"
"          lower is not specified, and the OPTION BASE hasn\'t used, the\n"
"          arrays are starting from 0.\n"
"\n"
"          Example:\n"
"\n"
"REM One dimension array of 7 elements, starting from 0\n"
"DIM A(6)\n"
"...\n"
"REM One dimension array of 6 elements, starting from 1\n"
"DIM A(1 TO 6)\n"
"...\n"
"REM Three dimension array\n"
"DIM A(1 TO 6, 1 TO 4, 1 TO 8)\n"
"...\n"
"REM Allocating zero-length arrays:\n"
"DIM z()\n"
"...\n"
"IF LEN(Z)=0 THE APPEND Z, \"The first element\"\n"
"\n"
},

{ "LABEL", "STATEMENT", 
"_\bL_\bA_\bB_\bE_\bL _\bn_\ba_\bm_\be", 
"          Defines a label. A label is a mark at this position of the\n"
"          code.\n"
"\n"
"          There are two kinds of labels, the \'numeric\' and the\n"
"          \'alphanumeric\'.\n"
"\n"
"          \'Numeric\' labels does not needed the keyword LABEL, but\n"
"          \'alphanumeric\' does.\n"
"\n"
"          Example:\n"
"\n"
"1000 ? \"Hello\"\n"
"...\n"
"LABEL AlphaLabel: ? \"Hello\"\n"
"...\n"
"GOTO 1000\n"
"GOTO AlphaLabel\n"
"\n"
},

{ "GOTO", "STATEMENT", 
"_\bG_\bO_\bT_\bO _\bl_\ba_\bb_\be_\bl", 
"          Causes program execution to branch to a specified position\n"
"          (label).\n"
"\n"
},

{ "GOSUB", "STATEMENT", 
"_\bG_\bO_\bS_\bU_\bB _\bl_\ba_\bb_\be_\bl", 
"          Causes program execution to branch to the specified label;\n"
"          when the RETURN command is encountered, execution branches to\n"
"          the command immediately following the most recent GOSUB\n"
"          command.\n"
"\n"
},

{ "RETURN", "STATEMENT", 
"_\bR_\bE_\bT_\bU_\bR_\bN", 
"          Execution branches to the command immediately following the\n"
"          most recent GOSUB command.\n"
"\n"
"...\n"
"GOSUB my_routine\n"
"PRINT \"RETURN sent me here\"\n"
"...\n"
"LABEL my_routine\n"
"PRINT \"I am in my routine\"\n"
"RETURN\n"
"\n"
},

{ "ON", "STATEMENT", 
"_\bO_\bN _\b{_\bG_\bO_\bT_\bO_\b|_\bG_\bO_\bS_\bU_\bB_\b}_\b _\bl_\ba_\bb_\be_\bl_\b1_\b _\b[_\b,_\b _\b._\b._\b._\b,_\b _\bl_\ba_\bb_\be_\bl_\bN_\b]", 
"          Causes BASIC to branch to one of a list of labels.\n"
"\n"
"        expr\n"
"               A numeric expression in the range 0 to 255. Upon\n"
"               execution of the ON...GOTO command (or ON...GOSUB), BASIC\n"
"               branches to the nth item in the list of labels that\n"
"               follows the keyword GOTO (or GOSUB).\n"
"\n"
},

{ "FOR", "STATEMENT", 
"_\bF_\bO_\bR _\bc_\bo_\bu_\bn_\bt_\be_\br_\b _\b=_\b _\bs_\bt_\ba_\br_\bt_\b _\bT_\bO_\b _\be_\bn_\bd_\b _\b[_\bS_\bT_\bE_\bP_\b _\bi_\bn_\bc_\br_\b]_\b _\b._\b._\b._\b _\bN_\bE_\bX_\bT", 
"          Begins the definition of a FOR/NEXT loop.\n"
"\n"
"        counter\n"
"               A numeric variable to be used as the loop counter.\n"
"        start\n"
"               A numeric expression; the starting value of counter.\n"
"        end\n"
"               A numeric expression; the ending value of counter.\n"
"        incr\n"
"               A numeric expression; the value by which counter is\n"
"               incremented or decremented with each iteration of the\n"
"               loop. The default value is +1.\n"
"\n"
"          BASIC begins processing of the FOR/NEXT block by setting\n"
"          counter equal to start. Then, if \'incr\' is positive and\n"
"          counter is not greater than end, the commands between the FOR\n"
"          and the NEXT are executed.\n"
"\n"
"          When the NEXT is encountered, counter is increased by \'incr\',\n"
"          and the process is repeated. Execution passes to the command\n"
"          following the NEXT if counter is greater than end.\n"
"\n"
"          If increment is negative, execution of the FOR/NEXT loop is\n"
"          terminated whenever counter becomes less than end.\n"
"\n"
"          FOR/NEXT loops may be nested to any level of complexity, but\n"
"          there must be a NEXT for each FOR.\n"
"\n"
"          Example:\n"
"\n"
"FOR C=1 TO 9\n"
"    PRINT C\n"
"NEXT\n"
"\n"
},

{ "FOR", "STATEMENT", 
"_\bF_\bO_\bR _\be_\bl_\be_\bm_\be_\bn_\bt_\b _\bI_\bN_\b _\ba_\br_\br_\ba_\by_\b _\b._\b._\b._\b _\bN_\bE_\bX_\bT", 
"          Begins the definition of a FOR/NEXT loop.\n"
"\n"
"        element\n"
"               A variable to be used as the copy of the current element.\n"
"        array\n"
"               An array expression\n"
"\n"
"          The commands-block will repeated for LEN(array) times. Each\n"
"          time the \'element\' will holds the value of the current element\n"
"          of the array.\n"
"\n"
"          FOR/NEXT loops may be nested to any level of complexity, but\n"
"          there must be a NEXT for each FOR.\n"
"\n"
"          Example:\n"
"\n"
"A=[1,2,3]\n"
"FOR E IN A\n"
"    PRINT E\n"
"NEXT\n"
"...\n"
"\' This is the same with that\n"
"A=[1,2,3]\n"
"FOR I=LBOUND(A) TO UBOUND(A)\n"
"    E=A(I)\n"
"    PRINT E\n"
"NEXT\n"
"\n"
},

{ "WHILE", "STATEMENT", 
"_\bW_\bH_\bI_\bL_\bE _\be_\bx_\bp_\br_\b _\b._\b._\b._\b _\bW_\bE_\bN_\bD", 
"          Begins the definition of a WHILE/WEND loop.\n"
"\n"
"        expr\n"
"               An expression\n"
"\n"
"          BASIC starts by evaluating expression. If expression is\n"
"          nonzero (true), the next command is executed. If expression is\n"
"          zero (false), control passes to the first command following\n"
"          the next WEND command.\n"
"\n"
"          When BASIC encounters the WEND command, it reevaluates the\n"
"          expression parameter to the most recent WHILE. If that\n"
"          parameter is still nonzero (true), the process is repeated;\n"
"          otherwise, execution continues at the next command.\n"
"\n"
"          WHILE/WEND loops may be nested to any level of complexity, but\n"
"          there must be a WEND for each WHILE.\n"
"\n"
"          Example:\n"
"\n"
"C=1\n"
"WHILE C<10\n"
"    PRINT C\n"
"    C=C+1\n"
"WEND\n"
"...\n"
"\' This is the same with that\n"
"FOR C=1 TO 9\n"
"    PRINT C\n"
"NEXT\n"
"\n"
},

{ "REPEAT", "STATEMENT", 
"_\bR_\bE_\bP_\bE_\bA_\bT _\b._\b._\b._\b _\bU_\bN_\bT_\bI_\bL_\b _\be_\bx_\bp_\br", 
"          Begins the definition of a REPEAT/UNTIL loop.\n"
"\n"
"        expr\n"
"               An expression\n"
"\n"
"          BASIC starts executing the commands between the REPEAT and\n"
"          UNTIL commands. When BASIC encounters the UNTIL command, it\n"
"          evaluates the expression parameter. If that parameter is zero\n"
"          (false), the process will be repeated; otherwise, execution\n"
"          continues at the next command.\n"
"\n"
"          REPEAT/UNTIL loops may be nested to any level of complexity,\n"
"          but there must be an UNTIL for each REPEAT.\n"
"\n"
"          Example:\n"
"\n"
"C=1\n"
"REPEAT\n"
"    PRINT C\n"
"    C=C+1\n"
"UNTIL C=10\n"
"...\n"
"\' This is the same with that\n"
"FOR C=1 TO 9\n"
"    PRINT C\n"
"NEXT\n"
"\n"
},

{ "IF", "STATEMENT", 
"_\bI_\bF _\b._\b._\b.", 
"          _\bS_\by_\bn_\bt_\ba_\bx:\n"
"\n"
"IF expression1 [THEN]\n"
"    .\n"
"    . [commands]\n"
"    .\n"
"[ [ELSEIF | ELIF] expression2 [THEN]\n"
"    .\n"
"    . [commands]\n"
"    .\n"
"]\n"
"[ELSE\n"
"    .\n"
"    . [commands]\n"
"    .\n"
"]\n"
"{ ENDIF | FI }\n"
"\n"
"          Block-style IF.\n"
"\n"
"          Causes BASIC to make a decision based on the value of an\n"
"          expression.\n"
"\n"
"        expression\n"
"               An expression; 0 is equivalent to FALSE, while all other\n"
"               values are equivalent to TRUE.\n"
"        commands\n"
"               One or more commands.\n"
"\n"
"          Each expression in the IF/ELSEIF construct is tested in order.\n"
"          As soon as an expression is found to be TRUE, then its\n"
"          corresponding commands are executed. If no expressions are\n"
"          TRUE, then the commands following the ELSE keyword are\n"
"          executed. If ELSE is not specified, then execution continues\n"
"          with the command following the ENDIF.\n"
"\n"
"          IF, ELSE, ELSEIF, and ENDIF must all be the first keywords on\n"
"          their respective lines.\n"
"\n"
"          THEN is optional, but if its defined it must be the last\n"
"          keyword on its line; if anything other than a comment follows\n"
"          on the same line with THEN, BASIC thinks it\'s reading a\n"
"          single-line IF/THEN/ELSE construct.\n"
"\n"
"          IF blocks may be nested.\n"
"\n"
"          Example:\n"
"\n"
"x=1\n"
"IF x=1 THEN\n"
"    PRINT \"true\"\n"
"ELSE\n"
"    PRINT \"false\"\n"
"ENDIF\n"
"...\n"
"\' Alternate syntax:\n"
"x=1\n"
"IF x=1\n"
"    PRINT \"true\"\n"
"ELSE\n"
"    PRINT \"false\"\n"
"FI\n"
"\n"
"          Single-line IF.\n"
"\n"
"          _\bS_\by_\bn_\bt_\ba_\bx:\n"
"\n"
"IF expression THEN [num-label]|[command] [ELSE [num-label]|[command]]\n"
"\n"
"          Causes BASIC to make a decision based on the value of an\n"
"          expression.\n"
"\n"
"        expression\n"
"               An expression; 0 is equivalent to FALSE, while all other\n"
"               values are equivalent to TRUE.\n"
"        command\n"
"               Any legal command or a numeric label. If a number is\n"
"               specified, it is equivalent to a GOTO command with the\n"
"               specified numeric-label.\n"
"\n"
"          Example:\n"
"\n"
"\' Single-line IF\n"
"x=1\n"
"IF x=1 THEN PRINT \"true\" ELSE PRINT \"false\"\n"
"...\n"
"IF x=1 THEN 1000\n"
"...\n"
"1000 PRINT \"true\"\n"
"\n"
},

{ "END", "STATEMENT", 
"_\bE_\bN_\bD _\b[_\be_\br_\br_\bo_\br_\b]", 
"\n"
},

{ "STOP", "STATEMENT", 
"_\bS_\bT_\bO_\bP _\b[_\be_\br_\br_\bo_\br_\b]", 
"          Terminates execution of a program, closes all files opened by\n"
"          the program, and returns control to the operating system.\n"
"\n"
"        error\n"
"               A numeric expression.\n"
"\n"
"          The error is the value which will returned to operating\n"
"          system; if its not specified the BASIC will return 0.\n"
"\n"
"          _\bD_\bO_\bS_\b/_\bW_\bi_\bn_\bd_\bo_\bw_\bs _\bT_\bh_\be_\b _\b\'_\be_\br_\br_\bo_\br_\b\'_\b _\bv_\ba_\bl_\bu_\be_\b _\bi_\bs_\b _\bv_\be_\br_\by_\b _\bw_\be_\bl_\bl_\b _\bk_\bn_\bo_\bw_\bn_\b _\ba_\bs_\b _\bE_\bR_\bR_\bO_\bR_\bL_\bE_\bV_\bE_\bL\n"
"          _\bv_\ba_\bl_\bu_\be_\b.\n"
"\n"
},

{ "RESTORE", "STATEMENT", 
"_\bR_\bE_\bS_\bT_\bO_\bR_\bE _\bl_\ba_\bb_\be_\bl", 
"          Specifies the position of the next data to be read.\n"
"\n"
"        label\n"
"               A valid label.\n"
"\n"
},

{ "DATA", "STATEMENT", 
"_\bD_\bA_\bT_\bA _\bc_\bo_\bn_\bs_\bt_\ba_\bn_\bt_\b1_\b _\b[_\b,_\bc_\bo_\bn_\bs_\bt_\ba_\bn_\bt_\b2_\b]_\b._\b._\b.", 
"          Stores one or more constants, of any type, for subsequent\n"
"          access via READ command.\n"
"\n"
"          DATA commands are nonexecutable statements that supply a\n"
"          stream of data constants for use by READ commands. All the\n"
"          items supplied by all the DATA commands in a program make up\n"
"          one continuous \"string\" of information that is accessed in\n"
"          order by your program\'s READ commands.\n"
"\n"
"          Example:\n"
"\n"
"RESTORE MyDataBlock\n"
"FOR I=1 TO 3\n"
"    READ v\n"
"    PRINT v\n"
"NEXT\n"
"END\n"
"...\n"
"LABEL MyDataBlock\n"
"DATA 1,2,3\n"
"\n"
},

{ "ERASE", "STATEMENT", 
"_\bE_\bR_\bA_\bS_\bE _\bv_\ba_\br_\b[_\b,_\b _\bv_\ba_\br_\b[_\b,_\b _\b._\b._\b._\b _\bv_\ba_\br_\b]_\b]", 
"\n"
"        var\n"
"               Any variable.\n"
"\n"
"          Deallocates the memory used by the specified arrays or\n"
"          variables. After that these variables turned to simple\n"
"          integers with zero value.\n"
"\n"
"          Example:\n"
"\n"
"DIM x(100)\n"
"...\n"
"PRINT FRE(0)\n"
"ERASE x\n"
"PRINT FRE(0)\n"
"PRINT x(1):REM ERROR\n"
"\n"
},

{ "EXIT", "STATEMENT", 
"_\bE_\bX_\bI_\bT _\b[_\bF_\bO_\bR_\b|_\bL_\bO_\bO_\bP_\b|_\bS_\bU_\bB_\b|_\bF_\bU_\bN_\bC_\b]", 
"          Exits a multiline function definition, a loop, or a\n"
"          subprogram. By default (if no parameter is specified) exits\n"
"          from last command block (loop, for-loop or routine).\n"
"\n"
"        FOR\n"
"               Exit from the last FOR-NEXT loop\n"
"        LOOP\n"
"               Exit from the last WHILE-WEND or REPEAT-UNTIL loop\n"
"        SUB\n"
"               Return from the current routine\n"
"        FUNC\n"
"               Return from the current function\n"
"\n"
},

{ "READ", "COMMAND", 
"_\bR_\bE_\bA_\bD _\bv_\ba_\br_\b[_\b,_\b _\bv_\ba_\br_\b _\b._\b._\b._\b]", 
"          Assigns values in DATA items to specified variables.\n"
"\n"
"        var\n"
"               Any variable.\n"
"\n"
"          Unless a RESTORE command is executed, BASIC moves to the next\n"
"          DATA item with each READ assignment. If BASIC runs out of DATA\n"
"          items to READ, an run-time error occurs.\n"
"\n"
"          Example:\n"
"\n"
"FOR c=1 TO 6\n"
"    READ x\n"
"    PRINT x\n"
"NEXT\n"
"...\n"
"DATA \"a,b,c\", 2\n"
"DATA 3, 4\n"
"DATA \"fifth\", 6\n"
"\n"
},

{ "APPEND", "COMMAND", 
"_\bA_\bP_\bP_\bE_\bN_\bD _\ba_\b,_\b _\bv_\ba_\bl_\b _\b[_\b,_\b _\bv_\ba_\bl_\b _\b[_\b,_\b _\b._\b._\b._\b]_\b]", 
"\n"
"        a\n"
"               An array-variable.\n"
"        val\n"
"               Any value or expression\n"
"\n"
"          Inserts the values at the end of the specified array.\n"
"\n"
},

{ "INSERT", "COMMAND", 
"_\bI_\bN_\bS_\bE_\bR_\bT _\ba_\b,_\b _\bi_\bd_\bx_\b,_\b _\bv_\ba_\bl_\b _\b[_\b,_\b _\bv_\ba_\bl_\b _\b[_\b,_\b _\b._\b._\b._\b]_\b]_\b]", 
"\n"
"        a\n"
"               An array-variable.\n"
"        idx\n"
"               Position in the array.\n"
"        val\n"
"               Any value or expression.\n"
"\n"
"          Inserts the values to the specified array at the position idx.\n"
"\n"
},

{ "DELETE", "COMMAND", 
"_\bD_\bE_\bL_\bE_\bT_\bE _\ba_\b,_\b _\bi_\bd_\bx_\b _\b[_\b,_\b _\bc_\bo_\bu_\bn_\bt_\b]", 
"\n"
"        a\n"
"               An array-variable.\n"
"        idx\n"
"               Position in the array.\n"
"        count\n"
"               The number of the elements to be deleted.\n"
"\n"
"          Deletes \'count\' elements at position \'idx\' of array A\n"
"\n"
"                                  5. System\n"
"\n"
},

{ "RTE", "COMMAND", 
"_\bR_\bT_\bE _\b[_\bi_\bn_\bf_\bo_\b _\b[_\b,_\b _\b._\b._\b._\b]_\b]", 
"          Creates a Run-Time-Error. The parameters will be displayed on\n"
"          error-line.\n"
"\n"
},

{ "TIMEHMS", "COMMAND", 
"_\bT_\bI_\bM_\bE_\bH_\bM_\bS _\bh_\bm_\bs_\b _\b|_\b _\bt_\bi_\bm_\be_\br_\b,_\b _\bB_\bY_\bR_\bE_\bF_\b _\bh_\b,_\b _\bB_\bY_\bR_\bE_\bF_\b _\bm_\b,_\b _\bB_\bY_\bR_\bE_\bF_\b _\bs", 
"          Converts a time-value to hours, minutes and seconds integer\n"
"          values\n"
"\n"
},

{ "DATEDMY", "COMMAND", 
"_\bD_\bA_\bT_\bE_\bD_\bM_\bY _\bd_\bm_\by_\b _\b|_\b _\bj_\bu_\bl_\bi_\ba_\bn_\b__\bd_\ba_\bt_\be_\b,_\b _\bB_\bY_\bR_\bE_\bF_\b _\bd_\b,_\b _\bB_\bY_\bR_\bE_\bF_\b _\bm_\b,_\b _\bB_\bY_\bR_\bE_\bF_\b _\by", 
"          Returns the day, month and the year as integers.\n"
"\n"
},

{ "DELAY", "COMMAND", 
"_\bD_\bE_\bL_\bA_\bY _\bm_\bs", 
"          Delay for a specified amount of milliseconds. This \'delay\' is\n"
"          also depended to system clock.\n"
"\n"
},

{ "SORT", "COMMAND", 
"_\bS_\bO_\bR_\bT _\ba_\br_\br_\ba_\by_\b _\b[_\bU_\bS_\bE_\b _\bc_\bm_\bp_\bf_\bu_\bn_\bc_\b]", 
"          Sorts an array.\n"
"\n"
"          The cmpfunc (if its specified) it takes 2 vars to compare.\n"
"          cmpfunc must returns\n"
"\n"
"          -1 if x < y, +1 if x > y, 0 if x = y\n"
"\n"
"FUNC qscmp(x,y)\n"
"IF x=y\n"
"    qscmp=0\n"
"ELIF x>y\n"
"    qscmp=1\n"
"ELSE\n"
"    qscmp=-1\n"
"ENDIF\n"
"END\n"
"...\n"
"DIM A(5)\n"
"FOR i=0 TO 5\n"
"    A(i)=RND\n"
"NEXT\n"
"SORT A USE qscmp(x,y)\n"
"\n"
},

{ "SEARCH", "COMMAND", 
"_\bS_\bE_\bA_\bR_\bC_\bH _\bA_\b,_\b _\bk_\be_\by_\b,_\b _\bB_\bY_\bR_\bE_\bF_\b _\br_\bi_\bd_\bx_\b _\b[_\bU_\bS_\bE_\b _\bc_\bm_\bp_\bf_\bu_\bn_\bc_\b]", 
"          Scans an array for the key. If key is not found the SEARCH\n"
"          command returns (in ridx) the value (LBOUND(A)-1). In\n"
"          default-base arrays that means -1.\n"
"\n"
"          The cmpfunc (if its specified) it takes 2 vars to compare. It\n"
"          must return 0 if x = y; non-zero if x <> y\n"
"\n"
"FUNC cmp(x,y)\n"
"  cmp=!(x=y)\n"
"END\n"
"...\n"
"DIM A(5)\n"
"FOR i=0 TO 5\n"
"    A(i)=5-i\n"
"NEXT\n"
"SEARCH A, 4, r USE cmp(x,y)\n"
"PRINT r:REM prints 1\n"
"PRINT A(r): REM prints 4\n"
"\n"
},

{ "CHAIN", "COMMAND", 
"_\bC_\bH_\bA_\bI_\bN _\bf_\bi_\bl_\be", 
"          Transfers control to another SmallBASIC program.\n"
"\n"
"          file - A string expression that follows OS file naming\n"
"          conventions; The file must be a SmallBASIC source code file.\n"
"\n"
"CHAIN \"PROG2.BAS\"\n"
"\n"
},

{ "EXEC", "COMMAND", 
"_\bE_\bX_\bE_\bC _\bf_\bi_\bl_\be", 
"          Transfers control to another program\n"
"\n"
"          This routine works like CHAIN with the exception the file can\n"
"          be any executable file.\n"
"\n"
"          EXEC never returns\n"
"\n"
},

{ "ENVIRON", "COMMAND", 
"_\bE_\bN_\bV_\bI_\bR_\bO_\bN _\b\"_\be_\bx_\bp_\br_\b\"", 
"\n"
},

{ "ENV", "COMMAND", 
"_\bE_\bN_\bV _\b\"_\be_\bx_\bp_\br_\b\"", 
"          Adds a variable to or deletes a variable from the current\n"
"          environment variable-table.\n"
"\n"
"        expr\n"
"               A string expression of the form \"name=parameter\"\n"
"\n"
"          If name already exists in the environment table, its current\n"
"          setting is replaced with the new setting. If name does not\n"
"          exist, the new variable is added.\n"
"\n"
"          _\bP_\ba_\bl_\bm_\bO_\bS _\bS_\bB_\b _\be_\bm_\bu_\bl_\ba_\bt_\be_\bs_\b _\be_\bn_\bv_\bi_\br_\bo_\bn_\bm_\be_\bn_\bt_\b _\bv_\ba_\br_\bi_\ba_\bb_\bl_\be_\bs_\b.\n"
"\n"
},

{ "RUN", "COMMAND", 
"_\bR_\bU_\bN _\bc_\bm_\bd_\bs_\bt_\br", 
"          Loads a secondary copy of system\'s shell and, executes an\n"
"          program, or an shell command.\n"
"\n"
"        cmdstr\n"
"               Shell\'s specific command string\n"
"\n"
"          After the specified shell command or program terminates,\n"
"          control is returned to the line following the RUN command.\n"
"\n"
"          _\bP_\ba_\bl_\bm_\bO_\bS _\bT_\bh_\be_\b _\b\'_\bc_\bm_\bd_\bs_\bt_\br_\b\'_\b _\bi_\bs_\b _\bt_\bh_\be_\b _\bC_\br_\be_\ba_\bt_\bo_\br_\b-_\bI_\bD_\b.\n"
"          _\bP_\ba_\bl_\bm_\bO_\bS _\bT_\bh_\be_\b _\bR_\bU_\bN_\b _\bn_\be_\bv_\be_\br_\b _\br_\be_\bt_\bu_\br_\bn_\bs_\b.\n"
"\n"
},

{ "TRON", "COMMAND", 
"_\bT_\bR_\bO_\bN", 
"\n"
},

{ "TROFF", "COMMAND", 
"_\bT_\bR_\bO_\bF_\bF", 
"          TRACE ON/OFF. When trace mechanism is ON, the SB displays each\n"
"          line number as the program is executed\n"
"\n"
},

{ "LOGPRINT", "COMMAND", 
"_\bL_\bO_\bG_\bP_\bR_\bI_\bN_\bT _\b._\b._\b.", 
"          PRINT to SB\'s logfile. The syntax is the same with the PRINT\n"
"          command.\n"
"\n"
},

{ "POKE[{16|32}]", "COMMAND", 
"_\bP_\bO_\bK_\bE_\b[_\b{_\b1_\b6_\b|_\b3_\b2_\b}_\b] _\ba_\bd_\bd_\br_\b,_\b _\bv_\ba_\bl_\bu_\be", 
"          Writes a specified byte, word or dword at a specified memory\n"
"          address.\n"
"\n"
},

{ "USRCALL", "COMMAND", 
"_\bU_\bS_\bR_\bC_\bA_\bL_\bL _\ba_\bd_\bd_\br", 
"          Transfers control to an assembly language subroutine.\n"
"\n"
"          The USRCALL is equal to:\n"
"\n"
"void (*f)(void);\n"
"f = (void (*)(void)) addr;\n"
"f();\n"
"\n"
},

{ "BCOPY", "COMMAND", 
"_\bB_\bC_\bO_\bP_\bY _\bs_\br_\bc_\b__\ba_\bd_\bd_\br_\b,_\b _\bd_\bs_\bt_\b__\ba_\bd_\bd_\br_\b,_\b _\bl_\be_\bn_\bg_\bt_\bh", 
"          Copies a memory block from \'src_addr\' to \'dst_addr\'\n"
"\n"
},

{ "BLOAD", "COMMAND", 
"_\bB_\bL_\bO_\bA_\bD _\bf_\bi_\bl_\be_\bn_\ba_\bm_\be_\b[_\b,_\b _\ba_\bd_\bd_\br_\be_\bs_\bs_\b]", 
"          Loads a specified memory image file into memory.\n"
"\n"
},

{ "BSAVE", "COMMAND", 
"_\bB_\bS_\bA_\bV_\bE _\bf_\bi_\bl_\be_\bn_\ba_\bm_\be_\b,_\b _\ba_\bd_\bd_\br_\be_\bs_\bs_\b,_\b _\bl_\be_\bn_\bg_\bt_\bh", 
"          Copies a specified portion of memory to a specified file.\n"
"\n"
},

{ "STKDUMP", "COMMAND", 
"_\bS_\bT_\bK_\bD_\bU_\bM_\bP", 
"          Displays the SB\'s internal executor\'s stack\n"
"\n"
"          _\b* _\bF_\bo_\br_\b _\bd_\be_\bb_\bu_\bg_\b _\bp_\bu_\br_\bp_\bo_\bs_\be_\bs_\b;_\b _\bi_\bt_\b _\bi_\bs_\b _\bn_\bo_\bt_\b _\bs_\bu_\bp_\bp_\bo_\br_\bt_\be_\bd_\b _\bo_\bn_\b _\b\"_\bl_\bi_\bm_\bi_\bt_\be_\bd_\b\"_\b _\bO_\bS_\be_\bs_\b.\n"
"\n"
"                             6. Graphics & Sound\n"
"\n"
"   The SB\'s Graphics commands are working only with integers. (Of\n"
"   course, 2D algebra commands are working with reals) That is different\n"
"   of QB, but its much faster.\n"
"\n"
"6.1 The colors\n"
"\n"
"   _\bM_\bo_\bn_\bo_\bc_\bh_\br_\bo_\bm_\be\n"
"          0 = black, 15 = white\n"
"   _\b2_\bb_\bi_\bt_\b _\b(_\b4_\b _\bc_\bo_\bl_\bo_\br_\bs_\b)\n"
"          0 = black, 15 = white, 1-6, 8 = dark-gray, 7, 9-14 =\n"
"          light-gray\n"
"   _\b4_\bb_\bi_\bt_\b _\b(_\b1_\b6_\b _\bc_\bo_\bl_\bo_\br_\bs_\b)\n"
"          16 Standard VGA colors, 16 colors of gray (on PalmOS)\n"
"   _\b8_\bb_\bi_\bt_\b _\b(_\b2_\b5_\b6_\b _\bp_\ba_\bl_\be_\bt_\bt_\be_\bd_\b _\bc_\bo_\bl_\bo_\br_\bs_\b)\n"
"          16 Standard VGA colors. The rest colors are ignored.\n"
"   _\b1_\b5_\bb_\bi_\bt_\b _\b(_\b3_\b2_\bK_\b _\bc_\bo_\bl_\bo_\br_\bs_\b)_\b,_\b _\b1_\b6_\bb_\bi_\bt_\b _\b(_\b6_\b4_\bK_\b _\bc_\bo_\bl_\bo_\br_\bs_\b)_\b _\ba_\bn_\bd_\b _\b2_\b4_\bb_\bi_\bt_\b _\b(_\b1_\b._\b7_\bM_\b _\bc_\bo_\bl_\bo_\br_\bs_\b)\n"
"          Color 0..15 is the standard VGA colors, full 24-bit RGB colors\n"
"          can be passed by using negative number.\n"
"\n"
"6.2 The points\n"
"\n"
"   Any point can be specified by an array of 2 elements or by 2\n"
"   parameters\n"
"\n"
"   Example:\n"
"\n"
"LINE x1, y1, x2, y2\n"
"or\n"
"LINE [x1, y1], [x2, y2]\n"
"\n"
"   Also, the polylines can work with the same way.\n"
"\n"
"DIM poly(10)\n"
"...\n"
"poly[0] = [x, y]\n"
"\n"
"6.3 The STEP keyword\n"
"\n"
"   The STEP keyword calculates the next x,y parameters relative to\n"
"   current position. That position can be returned by using the POINT(0)\n"
"   and POINT(1) functions.\n"
"\n"
"6.4 The \'aspect\' parameter\n"
"\n"
"   The x/y factor.\n"
"\n"
"6.5 The FILLED keyword\n"
"\n"
"   The FILLED keyword fills the result of the command with the drawing\n"
"   color.\n"
"\n"
"6.6 Graphics Commands\n"
"\n"
},

{ "ARC", "COMMAND", 
"_\bA_\bR_\bC _\b[_\bS_\bT_\bE_\bP_\b]_\b _\bx_\b,_\by_\b,_\br_\b,_\ba_\bs_\bt_\ba_\br_\bt_\b,_\ba_\be_\bn_\bd_\b _\b[_\b,_\ba_\bs_\bp_\be_\bc_\bt_\b _\b[_\b,_\bc_\bo_\bl_\bo_\br_\b]_\b]_\b _\b[_\bC_\bO_\bL_\bO_\bR", 
"          _\bc_\bo_\bl_\bo_\br_\b]\n"
"          Draws an arc. astart,aend = first,last angle in radians.\n"
"\n"
},

{ "CHART", "COMMAND", 
"_\bC_\bH_\bA_\bR_\bT _\b{_\bL_\bI_\bN_\bE_\bC_\bH_\bA_\bR_\bT_\b|_\bB_\bA_\bR_\bC_\bH_\bA_\bR_\bT_\b}_\b,_\b _\ba_\br_\br_\ba_\by_\b(_\b)_\b _\b[_\b,_\b _\bt_\by_\bp_\be_\b _\b[_\b,_\b _\bx_\b1_\b,_\b _\by_\b1_\b,_\b _\bx_\b2_\b,", 
"          _\by_\b2_\b]_\b]\n"
"          Draws a chart of array values in the rectangular area\n"
"          x1,y1,x2,y2\n"
"\n"
"          Where \'type\':\n"
"\n"
"          0 simple\n"
"          1 with marks\n"
"          2 with ruler\n"
"          3 with marks & ruler\n"
"\n"
},

{ "PLOT", "COMMAND", 
"_\bP_\bL_\bO_\bT _\bx_\bm_\bi_\bn_\b,_\b _\bx_\bm_\ba_\bx_\b _\bU_\bS_\bE_\b _\bf_\b(_\bx_\b)", 
"          Graph of f(x)\n"
"\n"
"          Example:\n"
"\n"
"PLOT 0, 2*PI USE SIN(x)\n"
"\n"
},

{ "CIRCLE", "COMMAND", 
"_\bC_\bI_\bR_\bC_\bL_\bE _\b[_\bS_\bT_\bE_\bP_\b]_\b _\bx_\b,_\by_\b,_\br_\b _\b[_\b,_\ba_\bs_\bp_\be_\bc_\bt_\b _\b[_\b,_\b _\bc_\bo_\bl_\bo_\br_\b]_\b]_\b _\b[_\bC_\bO_\bL_\bO_\bR_\b _\bc_\bo_\bl_\bo_\br_\b]", 
"          _\b[_\bF_\bI_\bL_\bL_\bE_\bD_\b]\n"
"\n"
"        x\n"
"        y\n"
"               the circle\'s center\n"
"        r\n"
"               the radius\n"
"\n"
"          Draws a circle (or an ellipse if the aspect is specified).\n"
"\n"
},

{ "COLOR", "COMMAND", 
"_\bC_\bO_\bL_\bO_\bR _\bf_\bo_\br_\be_\bg_\br_\bo_\bu_\bn_\bd_\b-_\bc_\bo_\bl_\bo_\br_\b _\b[_\b,_\b _\bb_\ba_\bc_\bk_\bg_\br_\bo_\bu_\bn_\bd_\b-_\bc_\bo_\bl_\bo_\br_\b]", 
"          Specifies the foreground and background colors\n"
"\n"
},

{ "DRAWPOLY", "COMMAND", 
"_\bD_\bR_\bA_\bW_\bP_\bO_\bL_\bY _\ba_\br_\br_\ba_\by_\b _\b[_\b,_\bx_\b-_\bo_\br_\bi_\bg_\bi_\bn_\b,_\by_\b-_\bo_\br_\bi_\bg_\bi_\bn_\b _\b[_\b,_\b _\bs_\bc_\ba_\bl_\be_\bf_\b _\b[_\b,_\b _\bc_\bo_\bl_\bo_\br_\b]_\b]_\b]", 
"          _\b[_\bC_\bO_\bL_\bO_\bR_\b _\bc_\bo_\bl_\bo_\br_\b]_\b _\b[_\bF_\bI_\bL_\bL_\bE_\bD_\b]\n"
"          Draws a polyline\n"
"\n"
"          If the array does not uses points as element arrays, then even\n"
"          elements for x (starting from 0), odd elements for y\n"
"\n"
},

{ "DRAW", "COMMAND", 
"_\bD_\bR_\bA_\bW _\bs_\bt_\br_\bi_\bn_\bg", 
"          Draws an object according to instructions specified as a\n"
"          string.\n"
"\n"
"          string - A string expression containing commands in the BASIC\n"
"          graphics definition language.\n"
"\n"
"          Graphics Definition Language\n"
"\n"
"          In the movement instructions below, n specifies a distance to\n"
"          move. The number of pixels moved is equal to n multiplied by\n"
"          the current scaling factor, which is set by the S command.\n"
"\n"
"   Un Move up.\n"
"   Dn Move down.\n"
"   Ln Move left.\n"
"   Rn Move right.\n"
"   En Move diagonally up and right.\n"
"   Fn Move diagonally down and right.\n"
"   Gn Move diagonally down and left.\n"
"   Hn Move diagonally up and left.\n"
"   Mx,y Move to coordinate x,y. If x is preceded by a + or -, the\n"
"   movement is relative to the last point referenced.\n"
"   B A prefix command. Next movement command moves but doesn\'t plot.\n"
"   N A prefix command. Next movement command moves, but returns\n"
"   immediately to previous point.\n"
"\n"
"          _\b* _\bT_\bh_\bi_\bs_\b _\bc_\bo_\bm_\bm_\ba_\bn_\bd_\b _\bi_\bt_\b _\bi_\bs_\b _\bh_\ba_\bd_\b _\bn_\bo_\bt_\b _\bt_\be_\bs_\bt_\be_\bd_\b _\b-_\b _\bp_\bl_\be_\ba_\bs_\be_\b _\br_\be_\bp_\bo_\br_\bt_\b _\ba_\bn_\by_\b _\bb_\bu_\bg_\b _\bo_\br\n"
"          _\bi_\bn_\bc_\bo_\bm_\bp_\ba_\bt_\bi_\bb_\bi_\bl_\bi_\bt_\by_\b.\n"
"\n"
},

{ "LINE", "COMMAND", 
"_\bL_\bI_\bN_\bE _\b[_\bS_\bT_\bE_\bP_\b]_\b _\bx_\b,_\by_\b _\b[_\b{_\b,_\b|_\bS_\bT_\bE_\bP_\b}_\b _\bx_\b2_\b,_\by_\b2_\b]_\b _\b[_\b,_\b _\bc_\bo_\bl_\bo_\br_\b _\b|_\b _\bC_\bO_\bL_\bO_\bR_\b _\bc_\bo_\bl_\bo_\br_\b]", 
"          Draws a line\n"
"\n"
},

{ "PSET", "COMMAND", 
"_\bP_\bS_\bE_\bT _\b[_\bS_\bT_\bE_\bP_\b]_\b _\bx_\b,_\by_\b _\b[_\b,_\b _\bc_\bo_\bl_\bo_\br_\b _\b|_\b _\bC_\bO_\bL_\bO_\bR_\b _\bc_\bo_\bl_\bo_\br_\b]", 
"          Draw a pixel\n"
"\n"
},

{ "RECT", "COMMAND", 
"_\bR_\bE_\bC_\bT _\b[_\bS_\bT_\bE_\bP_\b]_\b _\bx_\b,_\by_\b _\b[_\b{_\b,_\b|_\bS_\bT_\bE_\bP_\b}_\b _\bx_\b2_\b,_\by_\b2_\b]_\b _\b[_\b,_\b _\bc_\bo_\bl_\bo_\br_\b _\b|_\b _\bC_\bO_\bL_\bO_\bR_\b _\bc_\bo_\bl_\bo_\br_\b]", 
"          _\b[_\bF_\bI_\bL_\bL_\bE_\bD_\b]\n"
"          Draws a rectangular parallelogram\n"
"\n"
},

{ "PAINT", "COMMAND", 
"_\bP_\bA_\bI_\bN_\bT _\b[_\bS_\bT_\bE_\bP_\b]_\b _\bx_\b,_\b _\by_\b _\b[_\b,_\bc_\bo_\bl_\bo_\br_\b _\b[_\b,_\bb_\bo_\br_\bd_\be_\br_\b]_\b]", 
"          Fills an enclosed area on the graphics screen with a specific\n"
"          color.\n"
"\n"
"        x\n"
"        y\n"
"               Screen coordinate (column, row) within the area that is\n"
"               to be filled.\n"
"        color\n"
"               The fill-color\n"
"        border\n"
"               The boundary-color\n"
"\n"
"          if the border-color is specified then the PAINT will fill all\n"
"          the area which is specified by the border-color. (fill-until,\n"
"          color!=point(x,y)\n"
"\n"
"          if the border-color is NOT specified then the PAINT will fill\n"
"          all the are with the same color as the pixel at x,y.\n"
"          (fill-while, color=point(x,y))\n"
"\n"
},

{ "VIEW", "COMMAND", 
"_\bV_\bI_\bE_\bW _\b[_\bx_\b1_\b,_\by_\b1_\b,_\bx_\b2_\b,_\by_\b2_\b _\b[_\b,_\bc_\bo_\bl_\bo_\br_\b _\b[_\b,_\bb_\bo_\br_\bd_\be_\br_\b-_\bc_\bo_\bl_\bo_\br_\b]_\b]_\b]", 
"          Defines a viewport.\n"
"\n"
"        x1\n"
"        y1\n"
"        x2\n"
"        y2\n"
"               Corner coordinates of the viewport.\n"
"        color\n"
"               If included, BASIC fills the viewport with the specified\n"
"               color.\n"
"        border-color\n"
"               If included, BASIC draws a border, in a specified color,\n"
"               around the defined viewport.\n"
"\n"
"          The viewport defined by VIEW is disabled by a VIEW command\n"
"          with no parameters.\n"
"\n"
},

{ "WINDOW", "COMMAND", 
"_\bW_\bI_\bN_\bD_\bO_\bW _\b[_\bx_\b1_\b,_\by_\b1_\b,_\bx_\b2_\b,_\by_\b2_\b]", 
"          Specifies \"world\" coordinates for the screen.\n"
"\n"
"        x1\n"
"        y1\n"
"        x2\n"
"        y2\n"
"               The corner coordinates of the world space.\n"
"\n"
"          The WINDOW command allows you to redefine the corners of the\n"
"          display screen as a pair of \"world\" coordinates.\n"
"\n"
"          The world space defined by WINDOW is disabled by a WINDOW\n"
"          command with no parameters.\n"
"\n"
},

{ "BEEP", "COMMAND", 
"_\bB_\bE_\bE_\bP", 
"          Generates a beep sound\n"
"\n"
},

{ "PLAY", "COMMAND", 
"_\bP_\bL_\bA_\bY _\bs_\bt_\br_\bi_\bn_\bg", 
"          Play musical notes\n"
"\n"
"        A-G[-|+|#][nnn][.]\n"
"               Play note A..G, +|# is sharp, - is flat, . is multiplier\n"
"               1.5\n"
"        On\n"
"               Octave 0..6, < moves down one octave, > moves up one\n"
"               octave\n"
"        Nnn\n"
"               Play note 0..84 (0 = pause)\n"
"        Pnnn\n"
"               Pause 1..64\n"
"        Lnnn\n"
"               Length of note 1..64 (1/nnn)\n"
"        Tnnn\n"
"               Tempo 32..255. Number of 1/4 notes per minute.\n"
"        MS\n"
"               Staccato (1/2)\n"
"        MN\n"
"               Normal (3/4)\n"
"        ML\n"
"               Legato\n"
"        Vnnn\n"
"               Volume 0..100\n"
"        MF\n"
"               Play on foreground\n"
"        MB\n"
"               Play on background\n"
"        Q\n"
"               Clear sound queue\n"
"\n"
},

{ "SOUND", "COMMAND", 
"_\bS_\bO_\bU_\bN_\bD _\bf_\br_\be_\bq_\b,_\b _\bd_\bu_\br_\b__\bm_\bs_\b _\b[_\b,_\b _\bv_\bo_\bl_\b]_\b _\b[_\bB_\bG_\b]", 
"          Plays a sound\n"
"\n"
"        freq\n"
"               The frequency\n"
"        dur_ms\n"
"               The duration in milliseconds\n"
"        vol\n"
"               The volume in 1/100 units\n"
"        BG\n"
"               Play it in background\n"
"\n"
},

{ "NOSOUND", "COMMAND", 
"_\bN_\bO_\bS_\bO_\bU_\bN_\bD", 
"          Stops background sound. Also, clears the sound queue.\n"
"\n"
"                               7. Miscellaneous\n"
"\n"
},

{ "RANDOMIZE", "COMMAND", 
"_\bR_\bA_\bN_\bD_\bO_\bM_\bI_\bZ_\bE _\b[_\bi_\bn_\bt_\b]", 
"          Seeds the random number generator\n"
"\n"
},

{ "PEN", "COMMAND", 
"_\bP_\bE_\bN _\bO_\bN_\b|_\bO_\bF_\bF", 
"          Enables/Disables the PEN/MOUSE mechanism.\n"
"\n"
},

{ "PAUSE", "COMMAND", 
"_\bP_\bA_\bU_\bS_\bE _\b[_\bs_\be_\bc_\bs_\b]", 
"          Pauses the execution for a specified length of time, or until\n"
"          user hit the keyboard.\n"
"\n"
},

{ "SWAP", "COMMAND", 
"_\bS_\bW_\bA_\bP _\ba_\b,_\b _\bb", 
"          Exchanges the values of two variables. The parameters may be\n"
"          variables of any type.\n"
"\n"
"                                8. File system\n"
"\n"
"8.1 Special Device Names\n"
"\n"
"   \"COM1:[speed]\"\n"
"          Serial port 1\n"
"   \"COM2:[speed]\"\n"
"          Serial port 2\n"
"   \"PDOC:filename\"\n"
"          Compressed PDOC files for PalmOS or PDB/PDOC files on other\n"
"          systems. PDOCFS opens and uncompress the file on OPEN; and\n"
"          compress the file on CLOSE. So, it will use a lot of memory\n"
"          and time (its depended on size of the data).\n"
"   \"MEMO:memo-title\"\n"
"          MemoDB of PalmOS or regular file on other systems. Memo\n"
"          records (virtual files) are limited to 3935 bytes\n"
"   \"SOCL:server:port\"\n"
"          Socket client. Actually a telnet client.\n"
"   \"MMC:filename\"\n"
"          eBookMan only. Opens an MMC file.\n"
"\n"
"   Example: OPEN \"COM1:\" AS #1\n"
"   OPEN \"COM2:38400\" AS #2\n"
"\n"
"8.2 File System Commands\n"
"\n"
},

{ "OPEN", "COMMAND", 
"_\bO_\bP_\bE_\bN _\bf_\bi_\bl_\be_\b _\b[_\bF_\bO_\bR_\b _\b{_\bI_\bN_\bP_\bU_\bT_\b|_\bO_\bU_\bT_\bP_\bU_\bT_\b|_\bA_\bP_\bP_\bE_\bN_\bD_\b}_\b]_\b _\bA_\bS_\b _\b#_\bf_\bi_\bl_\be_\bN", 
"          Makes a file or device available for sequential input,\n"
"          sequential output.\n"
"\n"
"        file\n"
"               A string expression that follows OS file naming\n"
"               conventions.\n"
"        fileN\n"
"               A file-handle (integer 1 to 256).\n"
"        FOR -\n"
"\n"
"               INPUT  Sequential input\n"
"               OUTPUT Sequential output\n"
"               APPEND Sequential output, beginning at current EOF\n"
"\n"
"          The files are always opened as shared.\n"
"\n"
},

{ "CLOSE", "COMMAND", 
"_\bC_\bL_\bO_\bS_\bE _\b#_\bf_\bi_\bl_\be_\bN", 
"          Close a file or device\n"
"\n"
},

{ "TLOAD", "COMMAND", 
"_\bT_\bL_\bO_\bA_\bD _\bf_\bi_\bl_\be_\b,_\b _\bB_\bY_\bR_\bE_\bF_\b _\bv_\ba_\br_\b _\b[_\b,_\b _\bt_\by_\bp_\be_\b]", 
"          Loads a text file into array variable. Each text-line is an\n"
"          array element.\n"
"\n"
"        file\n"
"               A string expression that follows OS file naming\n"
"               conventions.\n"
"        var\n"
"               Any variable\n"
"        type\n"
"               0 = load into array (default), 1 = load into string\n"
"\n"
},

{ "TSAVE", "COMMAND", 
"_\bT_\bS_\bA_\bV_\bE _\bf_\bi_\bl_\be_\b,_\b _\bv_\ba_\br", 
"          Writes an array to a text file. Each array element is a\n"
"          text-line.\n"
"\n"
"        file\n"
"               A string expression that follows OS file naming\n"
"               conventions.\n"
"        var\n"
"               An array variable or a string variable. Expressions are\n"
"               not allowed for memory reasons.\n"
"\n"
},

{ "CHMOD", "COMMAND", 
"_\bC_\bH_\bM_\bO_\bD _\bf_\bi_\bl_\be_\b,_\b _\bm_\bo_\bd_\be", 
"          Change permissions of a file\n"
"\n"
"        file\n"
"               A string expression that follows OS file naming\n"
"               conventions.\n"
"        mode\n"
"               The mode is compatible with the chmod()\'s \'mode\'\n"
"               parameter as its described on GNU\'s manual. See ACCESS()\n"
"               for more information.\n"
"\n"
"\' Make myfile available to anyone (read/write)\n"
"CHMOD \"myfile.bas\", 0o666\n"
"...\n"
"\' Make myfile available to anyone (execute/read/write)\n"
"CHMOD \"myfile.bas\", 0o777\n"
"\n"
},

{ "PRINT#", "COMMAND", 
"_\bP_\bR_\bI_\bN_\bT_\b# _\bf_\bi_\bl_\be_\bN_\b,_\b _\b[_\bU_\bS_\bI_\bN_\bG_\b._\b._\b._\b]_\b _\b._\b._\b.", 
"          Write string to a file. The syntax is the same with the PRINT\n"
"          command.\n"
"\n"
"          _\b* _\bW_\be_\b _\bc_\ba_\bn_\b _\bu_\bs_\be_\b _\b\'_\bU_\bS_\bG_\b\'_\b _\bi_\bn_\bs_\bt_\be_\ba_\bd_\b _\bo_\bf_\b _\b\'_\bU_\bS_\bI_\bN_\bG_\b\'_\b.\n"
"\n"
},

{ "LINPUT#", "COMMAND", 
"_\bL_\bI_\bN_\bP_\bU_\bT_\b# _\b[_\bf_\bi_\bl_\be_\bN_\b{_\b,_\b|_\b;_\b}_\b]_\b _\bv_\ba_\br", 
"\n"
},

{ "LINEINPUT#", "COMMAND", 
"_\bL_\bI_\bN_\bE_\bI_\bN_\bP_\bU_\bT_\b# _\b[_\b#_\bf_\bi_\bl_\be_\bN_\b{_\b,_\b|_\b;_\b}_\b]_\b _\bv_\ba_\br", 
"\n"
},

{ "LINE", "COMMAND", 
"_\bL_\bI_\bN_\bE_\b _\bI_\bN_\bP_\bU_\bT_\b# _\b[_\bf_\bi_\bl_\be_\bN_\b{_\b,_\b|_\b;_\b}_\b]_\b _\bv_\ba_\br", 
"          Reads a whole text line from file or console.\n"
"\n"
},

{ "INPUT#", "COMMAND", 
"_\bI_\bN_\bP_\bU_\bT_\b# _\bf_\bi_\bl_\be_\bN_\b;_\b _\bv_\ba_\br_\b1_\b _\b[_\b,_\bd_\be_\bl_\bi_\bm_\b]_\b _\b[_\b,_\b _\bv_\ba_\br_\b2_\b _\b[_\b,_\bd_\be_\bl_\bi_\bm_\b]_\b]_\b _\b._\b._\b.", 
"          Reads data from file\n"
"\n"
},

{ "BPUTC#", "COMMAND", 
"_\bB_\bP_\bU_\bT_\bC_\b# _\bf_\bi_\bl_\be_\bN_\b;_\b _\bb_\by_\bt_\be", 
"          (Binary mode) Writes a byte on file or device\n"
"\n"
},

{ "SEEK#", "COMMAND", 
"_\bS_\bE_\bE_\bK_\b# _\bf_\bi_\bl_\be_\bN_\b;_\b _\bp_\bo_\bs", 
"          Sets file position for the next read/write\n"
"\n"
},

{ "KILL", "COMMAND", 
"_\bK_\bI_\bL_\bL _\b\"_\bf_\bi_\bl_\be_\b\"", 
"          Deletes the specified file\n"
"\n"
},

{ "WRITE#", "COMMAND", 
"_\bW_\bR_\bI_\bT_\bE_\b# _\bf_\bi_\bl_\be_\bN_\b;_\b _\bv_\ba_\br_\b1_\b _\b[_\b,_\b _\b._\b._\b._\b]", 
"\n"
},

{ "READ#", "COMMAND", 
"_\bR_\bE_\bA_\bD_\b# _\bf_\bi_\bl_\be_\bN_\b;_\b _\bv_\ba_\br_\b1_\b _\b[_\b,_\b _\b._\b._\b._\b]", 
"          The READ/WRITE command set is used to store variables to a\n"
"          file as binary data.\n"
"\n"
"          The common problem with INPUT/PRINT set is there are many\n"
"          conflicts with data.\n"
"\n"
"PRINT #1; \"Hello, world\"\n"
"\n"
"          You have wrote only one string and you want read it in one\n"
"          variable, but this is impossible for INPUT command to\n"
"          understand it, because INPUT finds the separator comma, so it\n"
"          thinks there are two variables not one.\n"
"\n"
"          So, now, you can store arrays, strings etc and what is you\n"
"          write is what you will read the next time.\n"
"\n"
"          BTW its faster too.\n"
"\n"
"          _\b* _\bT_\bh_\be_\b _\bp_\ba_\br_\ba_\bm_\be_\bt_\be_\br_\bs_\b _\bc_\ba_\bn_\b _\bb_\be_\b _\bv_\ba_\br_\bi_\ba_\bb_\bl_\be_\bs_\b _\bO_\bN_\bL_\bY_\b.\n"
"          _\b* _\bI_\bt_\bs_\b _\bv_\be_\br_\by_\b _\bb_\ba_\bd_\b _\bi_\bd_\be_\ba_\b _\bt_\bo_\b _\bm_\bi_\bx_\be_\bd_\b _\bR_\bE_\bA_\bD_\b/_\bW_\bR_\bI_\bT_\bE_\b _\bc_\bo_\bm_\bm_\ba_\bn_\bd_\bs_\b _\bw_\bi_\bt_\bh\n"
"          _\bI_\bN_\bP_\bU_\bT_\b/_\bP_\bR_\bI_\bN_\bT_\b _\bc_\bo_\bm_\bm_\ba_\bn_\bd_\bs_\b _\bi_\bn_\b _\bt_\bh_\be_\b _\bs_\ba_\bm_\be_\b _\bf_\bi_\bl_\be_\b.\n"
"\n"
},

{ "COPY", "COMMAND", 
"_\bC_\bO_\bP_\bY _\b\"_\bf_\bi_\bl_\be_\b\"_\b,_\b _\b\"_\bn_\be_\bw_\bf_\bi_\bl_\be_\b\"", 
"          Makes a copy of specified file to the \'newfile\'\n"
"\n"
},

{ "RENAME", "COMMAND", 
"_\bR_\bE_\bN_\bA_\bM_\bE _\b\"_\bf_\bi_\bl_\be_\b\"_\b,_\b _\b\"_\bn_\be_\bw_\bn_\ba_\bm_\be_\b\"", 
"          Renames the specified file\n"
"\n"
},

{ "MKDIR", "COMMAND", 
"_\bM_\bK_\bD_\bI_\bR _\bd_\bi_\br", 
"          Create a directory. This does not working on PalmOS.\n"
"\n"
},

{ "CHDIR", "COMMAND", 
"_\bC_\bH_\bD_\bI_\bR _\bd_\bi_\br", 
"          Changes the current working directory. This does not working\n"
"          on PalmOS.\n"
"\n"
},

{ "RMDIR", "COMMAND", 
"_\bR_\bM_\bD_\bI_\bR _\bd_\bi_\br", 
"          Removes a directory. This does not working on PalmOS.\n"
"\n"
},

{ "DIRWALK", "COMMAND", 
"_\bD_\bI_\bR_\bW_\bA_\bL_\bK _\bd_\bi_\br_\be_\bc_\bt_\bo_\br_\by_\b _\b[_\b,_\b _\bw_\bi_\bl_\bd_\bc_\ba_\br_\bd_\bs_\b]_\b _\b[_\bU_\bS_\bE_\b _\b._\b._\b._\b]", 
"          Walk through the directories. The user-defined function must\n"
"          returns zero to stop the process.\n"
"\n"
"FUNC PRNF(x)\n"
"    ? x\n"
"    PRNF=TRUE\n"
"END\n"
"...\n"
"DIRWALK \".\" USE PRNF(x)\n"
"\n"
"          _\bP_\ba_\bl_\bm_\bO_\bS _\bN_\bo_\bt_\b _\bs_\bu_\bp_\bp_\bo_\br_\bt_\be_\bd_\b.\n"
"\n"
},

{ "EXPRSEQ", "COMMAND", 
"_\bE_\bX_\bP_\bR_\bS_\bE_\bQ _\bB_\bY_\bR_\bE_\bF_\b _\ba_\br_\br_\ba_\by_\b,_\b _\bx_\bm_\bi_\bn_\b,_\b _\bx_\bm_\ba_\bx_\b,_\b _\bc_\bo_\bu_\bn_\bt_\b _\bU_\bS_\bE_\b _\be_\bx_\bp_\br_\be_\bs_\bs_\bi_\bo_\bn", 
"          Returns an array with \'count\' elements. Each element had the\n"
"          \'y\' value of its position as it is returned by the expression.\n"
"\n"
"REM same as v=SEQ(0,1,11)\n"
"EXPRSEQ v, 0, 1, 11 USE x\n"
"\n"
},

{ "ROOT", "COMMAND", 
"_\bR_\bO_\bO_\bT _\bl_\bo_\bw_\b,_\b _\bh_\bi_\bg_\bh_\b,_\b _\bs_\be_\bg_\bs_\b,_\b _\bm_\ba_\bx_\be_\br_\br_\b,_\b _\bB_\bY_\bR_\bE_\bF_\b _\br_\be_\bs_\bu_\bl_\bt_\b,_\b _\bB_\bY_\bR_\bE_\bF_\b _\be_\br_\br_\bc_\bo_\bd_\be", 
"          _\bU_\bS_\bE_\b _\be_\bx_\bp_\br\n"
"          Roots of F(x)\n"
"\n"
"        low\n"
"               the lower limit\n"
"        high\n"
"               the upper limit\n"
"        segs\n"
"               the number of segments (spaces)\n"
"        maxerr\n"
"               tolerance (IF ABS(F(x)) < maxerr THEN OK)\n"
"        errcode\n"
"               0 for success; otherwise calculation error\n"
"        result\n"
"               the result\n"
"\n"
"FUNC F(x)\n"
" F = SIN(x)\n"
"END\n"
"...\n"
"ROOT 1, 5, 500, 0.00001, result, errcode USE F(x)\n"
"\n"
},

{ "DERIV", "COMMAND", 
"_\bD_\bE_\bR_\bI_\bV _\bx_\b,_\b _\bm_\ba_\bx_\bt_\br_\bi_\be_\bs_\b,_\b _\bm_\ba_\bx_\be_\br_\br_\b,_\b _\bB_\bY_\bR_\bE_\bF_\b _\br_\be_\bs_\bu_\bl_\bt_\b,_\b _\bB_\bY_\bR_\bE_\bF_\b _\be_\br_\br_\bc_\bo_\bd_\be_\b _\bU_\bS_\bE", 
"          _\be_\bx_\bp_\br\n"
"          Calculation of derivative\n"
"\n"
"        x\n"
"               value of x\n"
"        maxtries\n"
"               maximum number of retries\n"
"        maxerr\n"
"               tolerance\n"
"        errcode\n"
"               0 for success; otherwise calculation error\n"
"        result\n"
"               the result\n"
"\n"
},

{ "DIFFEQN", "COMMAND", 
"_\bD_\bI_\bF_\bF_\bE_\bQ_\bN _\bx_\b0_\b,_\b _\by_\b0_\b,_\b _\bx_\bf_\b,_\b _\bm_\ba_\bx_\bs_\be_\bg_\b,_\b _\bm_\ba_\bx_\be_\br_\br_\b,_\b _\bB_\bY_\bR_\bE_\bF_\b _\by_\bf_\b,_\b _\bB_\bY_\bR_\bE_\bF_\b _\be_\br_\br_\bc_\bo_\bd_\be", 
"          _\bU_\bS_\bE_\b _\be_\bx_\bp_\br\n"
"          Differential equation - Runge-Kutta method\n"
"\n"
"        x0\n"
"        y0\n"
"               initial x,y\n"
"        xf\n"
"               x final\n"
"        maxseg\n"
"               maximum number of segments on x\n"
"        maxerr\n"
"               tolerance (acceptable error between the last 2 times)\n"
"        errcode\n"
"               0 for success; otherwise calculation error\n"
"        yf\n"
"               the result\n"
"\n"
"                                10. 2D Algebra\n"
"\n"
},

{ "POLYEXT", "COMMAND", 
"_\bP_\bO_\bL_\bY_\bE_\bX_\bT _\bp_\bo_\bl_\by_\b(_\b)_\b,_\b _\bB_\bY_\bR_\bE_\bF_\b _\bx_\bm_\bi_\bn_\b,_\b _\bB_\bY_\bR_\bE_\bF_\b _\by_\bm_\bi_\bn_\b,_\b _\bB_\bY_\bR_\bE_\bF_\b _\bx_\bm_\ba_\bx_\b,_\b _\bB_\bY_\bR_\bE_\bF", 
"          _\by_\bm_\ba_\bx\n"
"          Returns the polyline\'s extents\n"
"\n"
},

{ "INTERSECT", "COMMAND", 
"_\bI_\bN_\bT_\bE_\bR_\bS_\bE_\bC_\bT _\bA_\bx_\b,_\b _\bA_\by_\b,_\b _\bB_\bx_\b,_\b _\bB_\by_\b,_\b _\bC_\bx_\b,_\b _\bC_\by_\b,_\b _\bD_\bx_\b,_\b _\bD_\by_\b,_\b _\bB_\bY_\bR_\bE_\bF_\b _\bt_\by_\bp_\be_\b,_\b _\bB_\bY_\bR_\bE_\bF", 
"          _\bR_\bx_\b,_\b _\bB_\bY_\bR_\bE_\bF_\b _\bR_\by\n"
"          Calculates the intersection of the two line segments A-B and\n"
"          C-D\n"
"\n"
"          Returns: Rx,Ry = cross\n"
"\n"
"          type = cross-type\n"
"\n"
"        0\n"
"               No cross (R = external cross)\n"
"        1\n"
"               One cross\n"
"        2\n"
"               Parallel\n"
"        3\n"
"               Parallel (many crosses)\n"
"        4\n"
"               The cross is one of the line segments edges.\n"
"\n"
"10.1 2D & 3D graphics transformations\n"
"\n"
"   2D & 3D graphics transformations can represented as matrices.\n"
"\n"
"   (c=cose, s=sine)\n"
"\n"
},

{ "M3IDENT", "COMMAND", 
"_\bM_\b3_\bI_\bD_\bE_\bN_\bT _\bB_\bY_\bR_\bE_\bF_\b _\bm_\b3_\bx_\b3", 
"          Resets matrix (Identity)\n"
"\n"
"|  1  0  0 |\n"
"|  0  1  0 |\n"
"|  0  0  1 |\n"
"\n"
},

{ "M3ROTATE", "COMMAND", 
"_\bM_\b3_\bR_\bO_\bT_\bA_\bT_\bE _\bB_\bY_\bR_\bE_\bF_\b _\bm_\b3_\bx_\b3_\b,_\b _\ba_\bn_\bg_\bl_\be_\b _\b[_\b,_\b _\bx_\b,_\b _\by_\b]", 
"          Rotate by angle with center x,y\n"
"\n"
"|  c  s  0 |\n"
"| -s  c  0 |\n"
"|  *  *  1 |\n"
"\n"
},

{ "M3SCALE", "COMMAND", 
"_\bM_\b3_\bS_\bC_\bA_\bL_\bE _\bB_\bY_\bR_\bE_\bF_\b _\bm_\b3_\bx_\b3_\b,_\b _\bx_\b,_\b _\by_\b,_\b _\bS_\bx_\b,_\b _\bS_\by", 
"          Scaling\n"
"\n"
"| Sx  0  0 |\n"
"|  0 Sy  0 |\n"
"|  *  *  1 |\n"
"\n"
},

{ "M3TRANS", "COMMAND", 
"_\bM_\b3_\bT_\bR_\bA_\bN_\bS _\bB_\bY_\bR_\bE_\bF_\b _\bm_\b3_\bx_\b3_\b,_\b _\bT_\bx_\b,_\b _\bT_\by", 
"          Translation\n"
"\n"
"|  1  0  0 |\n"
"|  0  1  0 |\n"
"| Tx Ty  1 |\n"
"\n"
},

{ "M3APPLY", "COMMAND", 
"_\bM_\b3_\bA_\bP_\bP_\bL_\bY _\bm_\b3_\bx_\b3_\b,_\b _\bB_\bY_\bR_\bE_\bF_\b _\bp_\bo_\bl_\by", 
"          Apply matrice to poly-line\n"
"\n"
"          Additional information:\n"
"\n"
"|  1  0  0 |\n"
"|  0 -1  0 | = reflection on x\n"
"|  0  0  1 |\n"
"\n"
"| -1  0  0 |\n"
"|  0  1  0 | = reflection on y\n"
"|  0  0  1 |\n"
"\n"
"          3D-Graphics Matrices:\n"
"\n"
"|  1  0  0 Tx |\n"
"|  0  1  0 Ty | = translation\n"
"|  0  0  1 Tz |\n"
"|  0  0  0  1 |\n"
"\n"
"| Sx  0  0  0 |\n"
"|  0 Sy  0  0 | = scaling\n"
"|  0  0 Sz  0 |\n"
"|  0  0  0  1 |\n"
"\n"
"|  1  0  0  0 |\n"
"|  0  c -s  0 | = rotation on x\n"
"|  0  s  c  0 |\n"
"|  0  0  0  1 |\n"
"\n"
"|  c  0  s  0 |\n"
"|  0  1  0  0 | = rotation on y\n"
"| -s  0  c  0 |\n"
"|  0  0  0  1 |\n"
"\n"
"|  c -s  0  0 |\n"
"|  s  c  0  0 | = rotation on z\n"
"|  0  0  1  0 |\n"
"|  0  0  0  1 |\n"
"\n"
"          Any change to matrix will combined with its previous value.\n"
"\n"
"DIM poly(24)\n"
"DIM M(2,2)\n"
"...\n"
"M3IDENT M\n"
"M3ROTATE M, pi/2, 0, 0\n"
"M3SCALE M, 0, 0, 1.24, 1.24\n"
"...\n"
"\' Draw the original polyline\n"
"DRAWPOLY poly\n"
"...\n"
"\' Draw the polyline\n"
"\' rotated by pi/2 from 0,0 and scaled by 1.24\n"
"M3APPLY M, poly\n"
"DRAWPOLY poly\n"
"\n"
"                                 11. Strings\n"
"\n"
},

{ "SPRINT", "COMMAND", 
"_\bS_\bP_\bR_\bI_\bN_\bT _\bv_\ba_\br_\b;_\b _\b[_\bU_\bS_\bI_\bN_\bG_\b._\b._\b._\b;_\b]_\b _\b._\b._\b.", 
"          Create formated string and storing it to var The syntax is the\n"
"          same with the PRINT command.\n"
"\n"
"SPRINT s; 12.34; TAB(12); 11.23;\n"
"\n"
"          _\b* _\bY_\bo_\bu_\b _\bc_\ba_\bn_\b _\bu_\bs_\be_\b _\b\'_\bU_\bS_\bG_\b\'_\b _\bi_\bn_\bs_\bt_\be_\ba_\bd_\b _\bo_\bf_\b _\b\'_\bU_\bS_\bI_\bN_\bG_\b\'_\b.\n"
"\n"
},

{ "SINPUT", "COMMAND", 
"_\bS_\bI_\bN_\bP_\bU_\bT _\bs_\br_\bc_\b;_\b _\bv_\ba_\br_\b _\b[_\b,_\b _\bd_\be_\bl_\bi_\bm_\b]_\b _\b[_\b,_\bv_\ba_\br_\b _\b[_\b,_\b _\bd_\be_\bl_\bi_\bm_\b]_\b]_\b _\b._\b._\b.", 
"          Splits the string \'src\' into variables which are separated by\n"
"          delimiters.\n"
"\n"
"SINPUT \"if x>1 then y\"; vif, \" \", vcond, \"then\", vdo\n"
"? vcond, vdo\n"
"\' result in monitor\n"
"\' x>1   y\n"
"\n"
},

{ "SPLIT", "COMMAND", 
"_\bS_\bP_\bL_\bI_\bT _\bs_\bt_\br_\bi_\bn_\bg_\b,_\b _\bd_\be_\bl_\bi_\bm_\bi_\bt_\be_\br_\bs_\b,_\b _\bw_\bo_\br_\bd_\bs_\b(_\b)_\b _\b[_\b,_\b _\bp_\ba_\bi_\br_\bs_\b]_\b _\b[_\bU_\bS_\bE_\b _\be_\bx_\bp_\br_\b]", 
"          Returns the words of the specified string into array \'words\'\n"
"\n"
"          Example:\n"
"\n"
"s=\"/etc/temp/filename.ext\"\n"
"SPLIT s, \"/.\", v()\n"
"FOR i=0 TO UBOUND(v)\n"
"  PRINT i;\" [\";v(i);\"]\"\n"
"NEXT\n"
"\'\n"
"displays:\n"
"0 []\n"
"1 [etc]\n"
"2 [temp]\n"
"3 [filename]\n"
"4 [ext]\n"
"\n"
},

{ "JOIN", "COMMAND", 
"_\bJ_\bO_\bI_\bN _\bw_\bo_\br_\bd_\bs_\b(_\b)_\b,_\b _\bd_\be_\bl_\bi_\bm_\bi_\bt_\be_\br_\bs_\b,_\b _\bs_\bt_\br_\bi_\bn_\bg", 
"          Returns the words of the specified string into array \'words\'\n"
"\n"
"          Example:\n"
"\n"
"s=\"/etc/temp/filename.ext\"\n"
"SPLIT s, \"/.\", v()\n"
"JOIN v(), \"/\", s\n"
"PRINT \"[\";s;\"]\"\n"
"\'\n"
"displays:\n"
"[/etc/temp/filename/ext]\n"
"\n"
"                                 12. Console\n"
"\n"
"12.1 Supported console codes\n"
"\n"
"   _\b* _\b\\_\be_\b _\b=_\b _\bC_\bH_\bR_\b(_\b2_\b7_\b)\n"
"\n"
"   \\t     tab (32 pixels)\n"
"   \\a     beep\n"
"   \\r\\n   new line (cr/lf)\n"
"   \\xC    clear screen\n"
"   \\e[K   clear to EOL\n"
"   \\e[nG  moves cursor to specified column\n"
"   \\e[0m  reset all attributes to their defaults\n"
"   \\e[1m  set bold on\n"
"   \\e[4m  set underline on\n"
"   \\e[7m  reverse video\n"
"   \\e[21m set bold off\n"
"   \\e[24m set underline off\n"
"   \\e[27m set reverse off\n"
"   \\e[3nm set foreground\n"
"          color. where n:\n"
"          0 black\n"
"          1 red\n"
"          2 green\n"
"          3 brown\n"
"          4 blue\n"
"          5 magenta\n"
"          6 cyan\n"
"          7 white\n"
"   \\e[4nm set background color.\n"
"          (see set foreground)\n"
"\n"
"   PalmOS only:\n"
"\n"
"   \\e[8nm (n=0..7) select system font\n"
"   \\e[9nm (n=0..3) select buildin font\n"
"\n"
"   eBookMan only:\n"
"\n"
"   \\e[50m select 9pt font\n"
"   \\e[51m select 12pt font\n"
"   \\e[52m select 16pt font\n"
"   \\e[nT  move to n/80th screen character position\n"
"\n"
"12.2 Console Commands\n"
"\n"
},

{ "PRINT", "COMMAND", 
"_\bP_\bR_\bI_\bN_\bT _\b[_\bU_\bS_\bI_\bN_\bG_\b _\b[_\bf_\bo_\br_\bm_\ba_\bt_\b]_\b;_\b]_\b _\b[_\be_\bx_\bp_\br_\b|_\bs_\bt_\br_\b _\b[_\b{_\b,_\b|_\b;_\b}_\b _\b[_\be_\bx_\bp_\br_\b|_\bs_\bt_\br_\b]_\b]_\b _\b._\b._\b.", 
"          Displays a text or the value of an expression.\n"
"\n"
"          _\bP_\bR_\bI_\bN_\bT_\b _\bS_\bE_\bP_\bA_\bR_\bA_\bT_\bO_\bR_\bS\n"
"\n"
"          TAB(n) Moves cursor position to the nth column.\n"
"          SPC(n) Prints a number of spaces specified by n.\n"
"          ;      Carriage return/line feed suppressed after printing.\n"
"          ,      Carriage return/line feed suppressed after printing.\n"
"                 A TAB character is placed.\n"
"\n"
"          _\bT_\bh_\be_\b _\bP_\bR_\bI_\bN_\bT_\b _\bU_\bS_\bI_\bN_\bG\n"
"\n"
"          Print USING, is using the FORMAT() to display numbers and\n"
"          strings. Unlike the FORMAT, this one can include literals,\n"
"          too.\n"
"\n"
"   _ Print next character as a literal. The combination _#, for\n"
"   example, allows you to include a number sign as a literal\n"
"   in your numeric format.\n"
"   [other] Characters other than the foregoing may be included as\n"
"   literals in the format string.\n"
"\n"
"          _\b* _\bW_\bh_\be_\bn_\b _\ba_\b _\bP_\bR_\bI_\bN_\bT_\b _\bU_\bS_\bI_\bN_\bG_\b _\bc_\bo_\bm_\bm_\ba_\bn_\bd_\b _\bi_\bs_\b _\be_\bx_\be_\bc_\bu_\bt_\be_\bd_\b _\bt_\bh_\be_\b _\bf_\bo_\br_\bm_\ba_\bt_\b _\bw_\bi_\bl_\bl\n"
"          _\br_\be_\bm_\ba_\bi_\bn_\bs_\b _\bo_\bn_\b _\bt_\bh_\be_\b _\bm_\be_\bm_\bo_\br_\by_\b _\bu_\bn_\bt_\bi_\bl_\b _\ba_\b _\bn_\be_\bw_\b _\bf_\bo_\br_\bm_\ba_\bt_\b _\bi_\bs_\b _\bp_\ba_\bs_\bs_\be_\bd_\b._\b _\bC_\ba_\bl_\bl_\bi_\bn_\bg_\b _\ba\n"
"          _\bP_\bR_\bI_\bN_\bT_\b _\bU_\bS_\bI_\bN_\bG_\b _\bw_\bi_\bt_\bh_\bo_\bu_\bt_\b _\ba_\b _\bn_\be_\bw_\b _\bf_\bo_\br_\bm_\ba_\bt_\b _\bs_\bp_\be_\bc_\bi_\bf_\bi_\be_\bd_\b _\bt_\bh_\be_\b _\bP_\bR_\bI_\bN_\bT_\b _\bw_\bi_\bl_\bl_\b _\bu_\bs_\be\n"
"          _\bt_\bh_\be_\b _\bf_\bo_\br_\bm_\ba_\bt_\b _\bo_\bf_\b _\bp_\br_\be_\bv_\bi_\bo_\bu_\bs_\b _\bc_\ba_\bl_\bl_\b.\n"
"\n"
"          Examples:\n"
"\n"
"PRINT USING \"##: #,###,##0.00\";\n"
"FOR i=0 TO 20\n"
"    PRINT USING; i+1, A(i)\n"
"NEXT\n"
"....\n"
"PRINT USING \"Total ###,##0 of \\ \\\"; number, \"bytes\"\n"
"\n"
"          _\b* _\bT_\bh_\be_\b _\bs_\by_\bm_\bb_\bo_\bl_\b _\b?_\b _\bc_\ba_\bn_\b _\bb_\be_\b _\bu_\bs_\be_\bd_\b _\bi_\bn_\bs_\bt_\be_\ba_\bd_\b _\bo_\bf_\b _\bk_\be_\by_\bw_\bo_\br_\bd_\b _\bP_\bR_\bI_\bN_\bT_\b _\bY_\bo_\bu_\b _\bc_\ba_\bn\n"
"          _\bu_\bs_\be_\b _\b\'_\bU_\bS_\bG_\b\'_\b _\bi_\bn_\bs_\bt_\be_\ba_\bd_\b _\bo_\bf_\b _\b\'_\bU_\bS_\bI_\bN_\bG_\b\'_\b.\n"
"\n"
},

{ "INPUT", "COMMAND", 
"_\bI_\bN_\bP_\bU_\bT _\b[_\bp_\br_\bo_\bm_\bp_\bt_\b _\b{_\b,_\b|_\b;_\b}_\b]_\b _\bv_\ba_\br_\b[_\b,_\b _\bv_\ba_\br_\b _\b[_\b,_\b _\b._\b._\b._\b]_\b]", 
"          Reads from \"keyboard\" a text and store it to variable.\n"
"\n"
},

{ "LINPUT", "COMMAND", 
"_\bL_\bI_\bN_\bP_\bU_\bT _\bv_\ba_\br", 
"\n"
},

{ "LINEINPUT", "COMMAND", 
"_\bL_\bI_\bN_\bE_\bI_\bN_\bP_\bU_\bT _\bv_\ba_\br", 
"\n"
},

{ "LINE", "COMMAND", 
"_\bL_\bI_\bN_\bE_\b _\bI_\bN_\bP_\bU_\bT _\bv_\ba_\br", 
"          Reads a whole text line from console.\n"
"\n"
},

{ "CLS", "COMMAND", 
"_\bC_\bL_\bS", 
"          Clears the screen.\n"
"\n"
},

{ "AT", "COMMAND", 
"_\bA_\bT _\bx_\b,_\b _\by", 
"          Moves the console cursor to the specified position. x,y are in\n"
"          pixels\n"
"\n"
},

{ "LOCATE", "COMMAND", 
"_\bL_\bO_\bC_\bA_\bT_\bE _\by_\b,_\b _\bx", 
"          Moves the console cursor to the specified position. x,y are in\n"
"          character cells.\n"
"\n"
"                             A. Interactive Mode\n"
"\n"
"   Like a shell, SB can run interactively. The _\bI_\bn_\bt_\be_\br_\ba_\bc_\bt_\bi_\bv_\be_\b _\bM_\bo_\bd_\be offers\n"
"   an old-style coding taste. Also, it is offers a quick editing/testing\n"
"   tool for console mode versions of SB.\n"
"\n"
"   The _\bI_\bn_\bt_\be_\br_\ba_\bc_\bt_\bi_\bv_\be_\b _\bM_\bo_\bd_\be can be used as a normal command-line _\bs_\bh_\be_\bl_\bl. It\n"
"   executes shell commands as a normal shell, but also, it can\n"
"   store/edit and run SB programs. However we _\bs_\bu_\bg_\bg_\be_\bs_\bt_\b _\bt_\bo_\b _\bu_\bs_\be_\b _\ba_\bn_\b _\be_\bd_\bi_\bt_\bo_\br.\n"
"\n"
"     * We can use the [TAB] for autocompletion (re-edit program lines or\n"
"       filename completition).\n"
"     * We can use [ARROWS] for history.\n"
"     * There is no need to type line numbers, there will be inserted\n"
"       automagically if you use \'+\' in the beginning of the line.\n"
"     * There is no need to type line numbers, use NUM.\n"
"     * Line numbers are not labels, are used only for editing. Use\n"
"       keyword LABEL to define a label.\n"
"     * Line numbers are not saved in files.\n"
"\n"
"A.1 Interactive Mode Commands\n"
"\n"
},

{ "HELP", "COMMAND", 
"_\bH_\bE_\bL_\bP _\b[_\bs_\bb_\b-_\bk_\be_\by_\bw_\bo_\br_\bd_\b]", 
"          Interactive mode help screen. The symbol \'?\' does the same.\n"
"\n"
},

{ "BYE", "COMMAND", 
"_\bB_\bY_\bE", 
"\n"
},

{ "QUIT", "COMMAND", 
"_\bQ_\bU_\bI_\bT", 
"\n"
},

{ "EXIT", "COMMAND", 
"_\bE_\bX_\bI_\bT", 
"          The BYE command ends SmallBASIC and returns the control to the\n"
"          Operating System.\n"
"\n"
},

{ "NEW", "COMMAND", 
"_\bN_\bE_\bW", 
"          The NEW command clears the memory and screen and prepares the\n"
"          computer for a new program. Be sure to save the program that\n"
"          you have been working on before you enter NEW as it is\n"
"          unrecoverable by any means once NEW has been entered.\n"
"\n"
},

{ "RUN", "COMMAND", 
"_\bR_\bU_\bN _\b[_\bf_\bi_\bl_\be_\bn_\ba_\bm_\be_\b]", 
"          The RUN command, which can also be used as a statement, starts\n"
"          program execution.\n"
"\n"
},

{ "CLS", "COMMAND", 
"_\bC_\bL_\bS", 
"          Clears the screen.\n"
"\n"
},

{ "LIST", "COMMAND", 
"_\bL_\bI_\bS_\bT _\b{_\b _\b[_\bs_\bt_\ba_\br_\bt_\b-_\bl_\bi_\bn_\be_\b]_\b _\b-_\b _\b[_\be_\bn_\bd_\b-_\bl_\bi_\bn_\be_\b]_\b _\b}", 
"          The LIST command allows you to display program lines. If LIST\n"
"          is entered with no numbers following it, the entire program in\n"
"          memory is listed. If a number follows the LIST, the line with\n"
"          that number is listed. If a number followed by hyphen follows\n"
"          LIST, that line and all lines following it are listed. If a\n"
"          number preceeded by a hyphen follows LIST, all lines\n"
"          preceeding it and that line are listed. If two numbers\n"
"          separated by a hyphen follow LIST, the indicated lines and all\n"
"          lines between them are listed.\n"
"\n"
},

{ "RENUM", "COMMAND", 
"_\bR_\bE_\bN_\bU_\bM _\b{_\b _\b[_\bi_\bn_\bi_\bt_\bi_\ba_\bl_\b-_\bl_\bi_\bn_\be_\b]_\b _\b[_\b,_\b]_\b _\b[_\bi_\bn_\bc_\br_\be_\bm_\be_\bn_\bt_\b]_\b _\b}", 
"          The RENUM command allows you to reassign line numbers.\n"
"\n"
},

{ "ERA", "COMMAND", 
"_\bE_\bR_\bA _\b{_\b _\b[_\bs_\bt_\ba_\br_\bt_\b-_\bl_\bi_\bn_\be_\b]_\b _\b-_\b _\b[_\be_\bn_\bd_\b-_\bl_\bi_\bn_\be_\b]_\b _\b}", 
"          The ERA command allows you to erase program lines. If ERA is\n"
"          entered with no numbers following it, the entire program in\n"
"          memory is erased. If a number follows the ERA, the line with\n"
"          that number is erased. If a number followed by hyphen follows\n"
"          ERA, that line and all lines following it are erased. If a\n"
"          number preceeded by a hyphen follows ERA, all lines preceeding\n"
"          it and that line are erased. If two numbers separated by a\n"
"          hyphen follow ERA, the indicated lines and all lines between\n"
"          them are erased.\n"
"\n"
},

{ "NUM", "COMMAND", 
"_\bN_\bU_\bM _\b[_\bi_\bn_\bi_\bt_\bi_\ba_\bl_\b-_\bl_\bi_\bn_\be_\b _\b[_\b,_\b _\bi_\bn_\bc_\br_\be_\bm_\be_\bn_\bt_\b]_\b]", 
"          The NUM command sets the values for the autonumbering. If the\n"
"          \'initial-line\' and \'increment\' are not specified, the line\n"
"          numbers start at 10 and increase in increments of 10.\n"
"\n"
},

{ "SAVE", "COMMAND", 
"_\bS_\bA_\bV_\bE _\bp_\br_\bo_\bg_\br_\ba_\bm_\b-_\bn_\ba_\bm_\be", 
"          The SAVE command allows you to copy the program in memory to a\n"
"          file. By using the LOAD command, you can later recall the\n"
"          program into memory.\n"
"\n"
},

{ "LOAD", "COMMAND", 
"_\bL_\bO_\bA_\bD _\bp_\br_\bo_\bg_\br_\ba_\bm_\b-_\bn_\ba_\bm_\be", 
"          The LOAD command loads \'program-name\' file into memory. The\n"
"          program must first have been put on file using the SAVE\n"
"          command. LOAD removes the program currently in memory before\n"
"          loading \'program-name\'.\n"
"\n"
},

{ "MERGE", "COMMAND", 
"_\bM_\bE_\bR_\bG_\bE _\bp_\br_\bo_\bg_\br_\ba_\bm_\b-_\bn_\ba_\bm_\be_\b,_\b _\bl_\bi_\bn_\be_\b-_\bn_\bu_\bm_\bb_\be_\br", 
"          The MERGE command merges lines in \'program-name\' file into the\n"
"          program lines already in the computer\'s memory. Use\n"
"          \'line-number\' to specify the position where the lines will be\n"
"          inserted.\n"
"\n"
},

{ "CD", "COMMAND", 
"_\bC_\bD _\b[_\bp_\ba_\bt_\bh_\b]", 
"          Changed the current directory. Without arguments, displays the\n"
"          current directory.\n"
"\n"
},

{ "DIR", "COMMAND", 
"_\bD_\bI_\bR _\b[_\br_\be_\bg_\be_\bx_\bp_\b]", 
"\n"
},

{ "DIRE", "COMMAND", 
"_\bD_\bI_\bR_\bE _\b[_\br_\be_\bg_\be_\bx_\bp_\b]", 
"\n"
},

{ "DIRD", "COMMAND", 
"_\bD_\bI_\bR_\bD _\b[_\br_\be_\bg_\be_\bx_\bp_\b]", 
"\n"
},

{ "DIRB", "COMMAND", 
"_\bD_\bI_\bR_\bB _\b[_\br_\be_\bg_\be_\bx_\bp_\b]", 
"          Displays the list of files. You can use DIRE for executables\n"
"          only or DIRD for directories only, or DIRB for BASIC sources.\n"
"\n"
},

{ "TYPE", "COMMAND", 
"_\bT_\bY_\bP_\bE _\bf_\bi_\bl_\be_\bn_\ba_\bm_\be", 
"          Displays the contents of the file.\n"
"\n"
"                               B. MySQL Module\n"
"\n"
},

{ "MYSQL.DISCONNECT", "COMMAND", 
"_\bM_\bY_\bS_\bQ_\bL_\b._\bD_\bI_\bS_\bC_\bO_\bN_\bN_\bE_\bC_\bT _\bh_\ba_\bn_\bd_\bl_\be", 
"          Disconnects\n"
"\n"
},

{ "MYSQL.USE", "COMMAND", 
"_\bM_\bY_\bS_\bQ_\bL_\b._\bU_\bS_\bE _\bh_\ba_\bn_\bd_\bl_\be_\b,_\b _\bd_\ba_\bt_\ba_\bb_\ba_\bs_\be", 
"          Changes the current database\n"
"\n"
"   Example:\n"
"\n"
"import mysql\n"
"\n"
"h = mysql.connect(\"localhost\", \"mydatabase\", \"user\", \"password\")\n"
"? \"Handle = \"; h\n"
"? \"DBS    = \"; mysql.dbs(h)\n"
"? \"TABLES = \"; mysql.tables(h)\n"
"? \"Query  = \"; mysql.query(h, \"SELECT * FROM sbx_counters\")\n"
"mysql.disconnect h\n"
"\n"
"                                C. GDBM Module\n"
"\n"
"   Example:\n"
"\n"
"import gdbm\n"
"\n"
"const GDBM_WRCREAT = 2          \' A writer.  Create the db if needed.\n"
"\n"
"\' TEST\n"
"h = gdbm.open(\"dbtest.db\", 512, GDBM_WRCREAT, 0o666)\n"
"? \"Handle = \"; h\n"
"? \"Store returns = \"; gdbm.store(h, \"key1\", \"data1....\")\n"
"? \"Store returns = \"; gdbm.store(h, \"key2\", \"data2....\")\n"
"? \"Fetch returns = \"; gdbm.fetch(h, \"key1\")\n"
"gdbm.close h\n"
"\n"
"                                  D. Limits\n"
"\n"
"D.1 Typical 32bit system\n"
"\n"
"   Bytecode size                4 GB\n"
"   Length of text lines         4095 characters\n"
"   User-defined keyword length  128 characters\n"
"   Maximum number of parameters 256\n"
"   Numeric value range          64 bit FPN (-/+ 1E+308)\n"
"   Maximum string size          2 GB\n"
"   Number of file handles       256\n"
"   Number of array-dimensions   6\n"
"   Number of colors             24 bit (0-15=VGA, <0=RGB)\n"
"   Background sound queue size  256 notes\n"
"   INPUT (console)              1023 characters per call, up to 16 variables\n"
"   COMMAND$                     1023 bytes\n"
"\n"
"   System events are checked every 50ms\n"
"\n"
"D.2 PalmOS (Typical 16bit system)\n"
"\n"
" Length of text lines         511 characters\n"
" Maximum number of parameters 32\n"
" User-defined keyword length  32 characters\n"
" Number of array-dimensions   3\n"
" Maximum string size          <32 KB\n"
" Number of file handles       16\n"
" Number of elements/array     2970 (that means 64KB of memory)\n"
" Bytecode size                <64 KB (by using CHAIN you can run progs > 64KB)\n"
" INPUT (console)              255 characters per call, up to 16 variables\n"
" COMMAND$                     127 bytes\n"
"\n"
"                             E. Writting Modules\n"
"\n"
"   * Modules are working only at Linux for now *\n"
"\n"
"   Modules are dynamic-linked libraries. The modules are \"connected\"\n"
"   with the SmallBASIC with a two-way style. That means, the module can\n"
"   execute functions of SB\'s library.\n"
"\n"
"   Module programmers will need to use variable\'s API to process\n"
"   parameters, and return values. Also, the device\'s API must be used\n"
"   because SB can run in different environments, of course module\n"
"   authors can use other C or other-lib functions to do their jobs.\n"
"\n"
"   Every module must implements the following C functions.\n"
"   _\bi_\bn_\bt_\b _\bs_\bb_\bl_\bi_\bb_\b__\bp_\br_\bo_\bc_\b__\bc_\bo_\bu_\bn_\bt_\b(_\b)\n"
"          Returns the number of procedures of the module.\n"
"   _\bi_\bn_\bt_\b _\bs_\bb_\bl_\bi_\bb_\b__\bf_\bu_\bn_\bc_\b__\bc_\bo_\bu_\bn_\bt_\b(_\b)\n"
"          Returns the number of functions of the module.\n"
"   _\bi_\bn_\bt_\b _\bs_\bb_\bl_\bi_\bb_\b__\bp_\br_\bo_\bc_\b__\bg_\be_\bt_\bn_\ba_\bm_\be_\b(_\bi_\bn_\bt_\b _\bi_\bn_\bd_\be_\bx_\b,_\b _\bc_\bh_\ba_\br_\b _\b*_\bn_\ba_\bm_\be_\b)\n"
"          Fills the \'name\' variable with the name of the \'index\'-th\n"
"          procedure. Returns 1 on success or 0 on error.\n"
"   _\bi_\bn_\bt_\b _\bs_\bb_\bl_\bi_\bb_\b__\bf_\bu_\bn_\bc_\b__\bg_\be_\bt_\bn_\ba_\bm_\be_\b(_\bi_\bn_\bt_\b _\bi_\bn_\bd_\be_\bx_\b,_\b _\bc_\bh_\ba_\br_\b _\b*_\bn_\ba_\bm_\be_\b)\n"
"          Fills the \'name\' variable with the name of the \'index\'-th\n"
"          function. Returns 1 on success or 0 on error.\n"
"   _\bi_\bn_\bt_\b _\bs_\bb_\bl_\bi_\bb_\b__\bp_\br_\bo_\bc_\b__\be_\bx_\be_\bc_\b(_\bi_\bn_\bt_\b _\bi_\bn_\bd_\be_\bx_\b,_\b _\bi_\bn_\bt_\b _\bp_\ba_\br_\ba_\bm_\b__\bc_\bo_\bu_\bn_\bt_\b,_\b _\bs_\bl_\bi_\bb_\b__\bp_\ba_\br_\b__\bt_\b _\b*_\bp_\ba_\br_\ba_\bm_\bs_\b,\n"
"          _\bv_\ba_\br_\b__\bt_\b _\b*_\br_\be_\bt_\bv_\ba_\bl_\b)\n"
"          Executes the \'index\' procedure. Returns 1 on success or 0 on\n"
"          error.\n"
"   _\bi_\bn_\bt_\b _\bs_\bb_\bl_\bi_\bb_\b__\bf_\bu_\bn_\bc_\b__\be_\bx_\be_\bc_\b(_\bi_\bn_\bt_\b _\bi_\bn_\bd_\be_\bx_\b,_\b _\bi_\bn_\bt_\b _\bp_\ba_\br_\ba_\bm_\b__\bc_\bo_\bu_\bn_\bt_\b,_\b _\bs_\bl_\bi_\bb_\b__\bp_\ba_\br_\b__\bt_\b _\b*_\bp_\ba_\br_\ba_\bm_\bs_\b,\n"
"          _\bv_\ba_\br_\b__\bt_\b _\b*_\br_\be_\bt_\bv_\ba_\bl_\b)\n"
"          Executes the \'index\' function. Returns 1 on success or 0 on\n"
"          error.\n"
"\n"
"   The slib_par_t structure contains two fields. The var_p which is a\n"
"   var_t structure (a SB variable), and the byref which is true if the\n"
"   variable can be used as by-reference.\n"
"\n"
"E.1 Variables API\n"
"\n"
"   Variables had 4 types. This type is described in .type field.\n"
"\n"
"   Values of .type\n"
"   V_STR\n"
"          String. The value can be accessed at .v.p.ptr.\n"
"   V_INT\n"
"          Integer. The value can be accessed at .v.i.\n"
"   V_REAL\n"
"          Real-number. The value can be accessed at .v.n.\n"
"   V_ARRAY\n"
"          Array. The .v.a.ptr is the data pointer (sizeof(var_t) *\n"
"          size). The .v.a.size is the number of elements. The\n"
"          .v.a.lbound[MAXDIM] is the lower bound values. The\n"
"          .v.a.ubound[MAXDIM] is the upper bound values. The .v.a.maxdim\n"
"          is the number of dimensions.\n"
"\n"
"   Example:\n"
"\n"
"/*\n"
"*   Displays variable data.\n"
"*   If the variable is an array, then this function\n"
"*   runs recursive, and the \'level\' parameter is used.\n"
"*/\n"
"static void print_variable(int level, var_t *variable)\n"
"{\n"
"    int i;\n"
"\n"
"    /* if recursive; place tabs */\n"
"    for ( i = 0; i < level; i ++ )\n"
"        dev_printf(\"\\t\");\n"
"\n"
"    /* print variable */\n"
"    switch ( variable->type )  {\n"
"    case V_STR:\n"
"        dev_printf(\"String =\\\"%s\\\"\\n\", variable->v.p.ptr);\n"
"        break;\n"
"    case V_INT:\n"
"        dev_printf(\"Integer = %ld\\n\", variable->v.i);\n"
"        break;\n"
"    case V_REAL:\n"
"        dev_printf(\"Real = %.2f\\n\",  variable->v.n);\n"
"        break;\n"
"    case V_ARRAY:\n"
"        dev_printf(\"Array of %d elements\\n\", variable->v.a.size);\n"
"        for ( i = 0; i < variable->v.a.size; i ++ ) {\n"
"            var_t *element_p;\n"
"\n"
"            element_p = (var_t *) (variable->v.a.ptr + sizeof(var_t) * i);\n"
"            print_variable(level+1, element_p);\n"
"            }\n"
"        break;\n"
"        }\n"
"}\n"
"\n"
"  E.1.1 Gereric\n"
"\n"
"   _\bv_\bo_\bi_\bd_\b _\bv_\b__\bf_\br_\be_\be_\b(_\bv_\ba_\br_\b__\bt_\b _\b*_\bv_\ba_\br_\b)\n"
"          This function resets the variable to 0 integer.\n"
"   _\bv_\bo_\bi_\bd_\b _\bv_\b__\bf_\br_\be_\be_\b(_\bv_\ba_\br_\b__\bt_\b _\b*_\bv_\ba_\br_\b)\n"
"          This function deletes the contents of varible var.\n"
"   _\bv_\ba_\br_\b__\bt_\b*_\b _\bv_\b__\bn_\be_\bw_\b(_\b)\n"
"          Creates a new variable and returns it. The returned variable\n"
"          it must be freed with both, v_free() and free() functions.\n"
"   _\bv_\ba_\br_\b__\bt_\b _\b*_\bv_\b__\bc_\bl_\bo_\bn_\be_\b(_\bc_\bo_\bn_\bs_\bt_\b _\bv_\ba_\br_\b__\bt_\b _\b*_\bs_\bo_\bu_\br_\bc_\be_\b)\n"
"          Returns a new variable which is a clone of source. The\n"
"          returned variable it must be freed with both, v_free() and\n"
"          free() functions.\n"
"   _\bv_\bo_\bi_\bd_\b _\bv_\b__\bs_\be_\bt_\b(_\bv_\ba_\br_\b__\bt_\b _\b*_\bd_\be_\bs_\bt_\b,_\b _\bc_\bo_\bn_\bs_\bt_\b _\bv_\ba_\br_\b__\bt_\b _\b*_\bs_\br_\bc_\b)\n"
"          Copies the src to dest.\n"
"\n"
"   Example\n"
"\n"
"void    myfunc()\n"
"{\n"
"        var_t   myvar;\n"
"\n"
"        v_init(&myvar);\n"
"        ...\n"
"        v_free(&myvar);\n"
"}\n"
"\n"
"  E.1.2 Real Numbers\n"
"\n"
"   _\bd_\bo_\bu_\bb_\bl_\be_\b _\bv_\b__\bg_\be_\bt_\br_\be_\ba_\bl_\b(_\bv_\ba_\br_\b__\bt_\b _\b*_\bv_\ba_\br_\bi_\ba_\bb_\bl_\be_\b)\n"
"          Returns the floating-point value of a variable. if variable is\n"
"          string it will converted to double.\n"
"   _\bv_\bo_\bi_\bd_\b _\bv_\b__\bs_\be_\bt_\br_\be_\ba_\bl_\b(_\bv_\ba_\br_\b__\bt_\b _\b*_\bv_\ba_\br_\b,_\b _\bd_\bo_\bu_\bb_\bl_\be_\b _\bn_\bu_\bm_\bb_\be_\br_\b)\n"
"          Sets the number real-number value to var variable.\n"
"\n"
"  E.1.3 Integer Numbers\n"
"\n"
"   _\bd_\bo_\bu_\bb_\bl_\be_\b _\bv_\b__\bi_\bg_\be_\bt_\bn_\bu_\bm_\b(_\bv_\ba_\br_\b__\bt_\b _\b*_\bv_\ba_\br_\bi_\ba_\bb_\bl_\be_\b)\n"
"          Returns the floating-point value of a variable. if variable is\n"
"          string it will converted to double.\n"
"   _\bv_\bo_\bi_\bd_\b _\bv_\b__\bs_\be_\bt_\bi_\bn_\bt_\b(_\bv_\ba_\br_\b__\bt_\b _\b*_\bv_\ba_\br_\b,_\b _\bi_\bn_\bt_\b3_\b2_\b _\bn_\bu_\bm_\bb_\be_\br_\b)\n"
"          Sets the number integer-number value to var variable.\n"
"\n"
"  E.1.4 Strings\n"
"\n"
"   _\bv_\bo_\bi_\bd_\b _\bv_\b__\bt_\bo_\bs_\bt_\br_\b(_\bv_\ba_\br_\b__\bt_\b _\b*_\ba_\br_\bg_\b)\n"
"          Converts variable arg to string.\n"
"   _\bc_\bh_\ba_\br_\b*_\b _\bv_\b__\bg_\be_\bt_\bs_\bt_\br_\b(_\bv_\ba_\br_\b__\bt_\b _\b*_\bv_\ba_\br_\b)\n"
"          Returns the string-pointer of variable var. If the var is not\n"
"          a string, it must be converted to string with the v_tostr()\n"
"          function.\n"
"   _\bv_\bo_\bi_\bd_\b _\bv_\b__\bz_\be_\br_\bo_\bs_\bt_\br_\b(_\bv_\ba_\br_\b__\bt_\b _\b*_\bv_\ba_\br_\b)\n"
"          Resets the variable var to a zero-length string.\n"
"   _\bv_\bo_\bi_\bd_\b _\bv_\b__\bs_\be_\bt_\bs_\bt_\br_\b(_\bv_\ba_\br_\b__\bt_\b _\b*_\bv_\ba_\br_\b,_\b _\bc_\bo_\bn_\bs_\bt_\b _\bc_\bh_\ba_\br_\b _\b*_\bs_\bt_\br_\bi_\bn_\bg_\b)\n"
"          Sets the string value string to the variable var.\n"
"   _\bv_\bo_\bi_\bd_\b _\bv_\b__\bs_\bt_\br_\bc_\ba_\bt_\b(_\bv_\ba_\br_\b__\bt_\b _\b*_\bv_\ba_\br_\b,_\b _\bc_\bo_\bn_\bs_\bt_\b _\bc_\bh_\ba_\br_\b _\b*_\bs_\bt_\br_\bi_\bn_\bg_\b)\n"
"          Adds the string string to string-variable var.\n"
"   _\bv_\bo_\bi_\bd_\b _\bv_\b__\bs_\be_\bt_\bs_\bt_\br_\bf_\b(_\bv_\ba_\br_\b__\bt_\b _\b*_\bv_\ba_\br_\b,_\b _\bc_\bo_\bn_\bs_\bt_\b _\bc_\bh_\ba_\br_\b _\b*_\bf_\bm_\bt_\b,_\b _\b._\b._\b._\b)\n"
"          Sets a string value to variable var using printf() style. The\n"
"          buffer size is limited to 1KB for OS_LIMITED (PalmOS),\n"
"          otherwise 64kB.\n"
"\n"
"  E.1.5 Arrays\n"
"\n"
"   SB arrays are always one-dimension. The multiple dimensions positions\n"
"   are calculated at run-time. Each element of the arrays is a `var_t\'\n"
"   object.\n"
"\n"
"   _\bv_\ba_\br_\b__\bt_\b*_\b _\bv_\b__\be_\bl_\be_\bm_\b(_\bv_\ba_\br_\b__\bt_\b _\b*_\ba_\br_\br_\ba_\by_\b,_\b _\bi_\bn_\bt_\b _\bi_\bn_\bd_\be_\bx_\b)\n"
"          Returns the variable pointer of the element index of the\n"
"          array. index is a zero-based, one dimention, index.\n"
"   _\bi_\bn_\bt_\b _\bv_\b__\ba_\bs_\bi_\bz_\be_\b(_\bv_\ba_\br_\b__\bt_\b _\b*_\ba_\br_\br_\ba_\by_\b)\n"
"          Returns the number of the elements of the array.\n"
"   _\bv_\bo_\bi_\bd_\b _\bv_\b__\br_\be_\bs_\bi_\bz_\be_\b__\ba_\br_\br_\ba_\by_\b(_\bv_\ba_\br_\b__\bt_\b _\b*_\ba_\br_\br_\ba_\by_\b,_\b _\bi_\bn_\bt_\b _\bs_\bi_\bz_\be_\b)\n"
"          Resizes the 1-dimention array.\n"
"   _\bv_\bo_\bi_\bd_\b _\bv_\b__\bt_\bo_\bm_\ba_\bt_\br_\bi_\bx_\b(_\bv_\ba_\br_\b__\bt_\b _\b*_\bv_\ba_\br_\b,_\b _\bi_\bn_\bt_\b _\br_\b,_\b _\bi_\bn_\bt_\b _\bc_\b)\n"
"          Converts the variable var to an array of r rows and c columns.\n"
"   _\bv_\bo_\bi_\bd_\b _\bv_\b__\bt_\bo_\ba_\br_\br_\ba_\by_\b1_\b(_\bv_\ba_\br_\b__\bt_\b _\b*_\bv_\ba_\br_\b,_\b _\bi_\bn_\bt_\b _\bn_\b)\n"
"          Converts the variable var to an array of n elements.\n"
"   _\bv_\bo_\bi_\bd_\b _\bv_\b__\bs_\be_\bt_\bi_\bn_\bt_\ba_\br_\br_\ba_\by_\b(_\bv_\ba_\br_\b__\bt_\b _\b*_\bv_\ba_\br_\b,_\b _\bi_\bn_\bt_\b3_\b2_\b _\b*_\bi_\bt_\ba_\bb_\bl_\be_\b,_\b _\bi_\bn_\bt_\b _\bc_\bo_\bu_\bn_\bt_\b)\n"
"          Makes variable var an integer array of count elements. The\n"
"          values are specified in itable.\n"
"   _\bv_\bo_\bi_\bd_\b _\bv_\b__\bs_\be_\bt_\br_\be_\ba_\bl_\ba_\br_\br_\ba_\by_\b(_\bv_\ba_\br_\b__\bt_\b _\b*_\bv_\ba_\br_\b,_\b _\bd_\bo_\bu_\bb_\bl_\be_\b _\b*_\br_\bt_\ba_\bb_\bl_\be_\b,_\b _\bi_\bn_\bt_\b _\bc_\bo_\bu_\bn_\bt_\b)\n"
"          Makes variable var a real-number array of count elements. The\n"
"          values are specified in rtable.\n"
"   _\bv_\bo_\bi_\bd_\b _\bv_\b__\bs_\be_\bt_\bs_\bt_\br_\ba_\br_\br_\ba_\by_\b(_\bv_\ba_\br_\b__\bt_\b _\b*_\bv_\ba_\br_\b,_\b _\bc_\bh_\ba_\br_\b _\b*_\b*_\bc_\bt_\ba_\bb_\bl_\be_\b,_\b _\bi_\bn_\bt_\b _\bc_\bo_\bu_\bn_\bt_\b)\n"
"          Makes variable var a string array of count elements. The\n"
"          values (which are copied) are specified in ctable.\n"
"\n"
"   Example\n"
"\n"
"void    myfunc()\n"
"{\n"
"        int             c_array[] = { 10, 20, 30 };\n"
"        var_t   myvar;\n"
"\n"
"        v_init(&myvar);\n"
"        v_setintarray(&myvar, c_array, 3);\n"
"        v_free(&myvar);\n"
"}\n"
"\n"
"E.2 Typical Module Source\n"
"\n"
"   This is a typical example of a module with one Function and and one\n"
"   Command. The command \"CMDA\" displays its parameters, and the function\n"
"   \"FUNCA\" returns a string.\n"
"\n"
"   --- mymod.c ---\n"
"\n"
"#include <extlib.h>\n"
"\n"
"/*\n"
"*   Displays variable data.\n"
"*   If the variable is an array, then runs recursive, the\n"
"*   \'level\' parameter is the call level.\n"
"*/\n"
"static void print_variable(int level, var_t *param)\n"
"{\n"
"    int i;\n"
"\n"
"    /* if recursive; place tabs */\n"
"    for ( i = 0; i < level; i ++ )\n"
"        dev_printf(\"\\t\");\n"
"\n"
"    /* print variable */\n"
"    switch ( param->type )  {\n"
"    case V_STR:\n"
"        dev_printf(\"String =\\\"%s\\\"\\n\", param->v.p.ptr);\n"
"        break;\n"
"    case V_INT:\n"
"        dev_printf(\"Integer = %ld\\n\", param->v.i);\n"
"        break;\n"
"    case V_REAL:\n"
"        dev_printf(\"Real = %.2f\\n\",  param->v.n);\n"
"        break;\n"
"    case V_ARRAY:\n"
"        dev_printf(\"Array of %d elements\\n\", param->v.a.size);\n"
"        for ( i = 0; i < param->v.a.size; i ++ ) {\n"
"            var_t *element_p;\n"
"\n"
"            element_p = (var_t *) (param->v.a.ptr + sizeof(var_t) * i);\n"
"            print_variable(level+1, element_p);\n"
"            }\n"
"        break;\n"
"        }\n"
"}\n"
"\n"
"/* typical command */\n"
"void m_cmdA(int param_count, slib_par_t *params, var_t *retval)\n"
"{\n"
"    int i;\n"
"\n"
"    for ( i = 0; i < param_count; i ++ ) {\n"
"        param = params[i].var_p;\n"
"        print_variable(0, param);\n"
"        }\n"
"}\n"
"\n"
"/* typical function */\n"
"int m_funcA(int param_count, slib_par_t *params, var_t *retval)\n"
"{\n"
"    v_setstr(retval, \"funcA() works!\");\n"
"    return 1;        /* success */\n"
"}\n"
"\n"
"/* the node-type of function/procedure tables */\n"
"typedef struct {\n"
"        char *name;  /* the name of the function */\n"
"        int  (*command)(slib_par_t *, int, var_t *);\n"
"        } mod_kw;\n"
"\n"
"/* functions table */\n"
"static mod_kw func_names[] =\n"
"{\n"
"{ \"FUNCA\", m_funcA }, // function A\n"
"{ NULL, NULL }\n"
"};\n"
"\n"
"/* commands table */\n"
"static mod_kw proc_names[] =\n"
"{\n"
"{ \"CMDA\", m_cmdA }, // command A\n"
"{ NULL, NULL }\n"
"};\n"
"\n"
"/* returns the number of the procedures */\n"
"int sblib_proc_count(void)\n"
"{\n"
"    int i;\n"
"\n"
"    for ( i = 0; proc_names[i].name; i ++ );\n"
"    return i;\n"
"}\n"
"\n"
"/* returns the number of the functions */\n"
"int sblib_func_count(void)\n"
"{\n"
"    int i;\n"
"\n"
"    for ( i = 0; func_names[i].name; i ++ );\n"
"    return i;\n"
"}\n"
"\n"
"/* returns the \'index\' procedure name */\n"
"int sblib_proc_getname(int index, char *proc_name)\n"
"{\n"
"    strcpy(proc_name, proc_names[index].name);\n"
"    return 1;\n"
"}\n"
"\n"
"/* returns the \'index\' function name */\n"
"int sblib_func_getname(int index, char *proc_name)\n"
"{\n"
"    strcpy(proc_name, func_names[index].name);\n"
"    return 1;\n"
"}\n"
"\n"
"/* execute the \'index\' procedure */\n"
"int sblib_proc_exec(int index, int param_count,\n"
"                                slib_par_t *params, var_t *retval)\n"
"{ return proc_names[index].command(params, param_count, retval); }\n"
"\n"
"/* execute the \'index\' function */\n"
"int sblib_func_exec(int index, int param_count,\n"
"                                slib_par_t *params, var_t *retval)\n"
"{ return func_names[index].command(params, param_count, retval); }\n"
"\n"
"E.3 Typical Module Makefile\n"
"\n"
"   This is a typical Makefile. In our example the module name is \'mymod\'\n"
"   and its source is the \'mymod.c\'. Also, our module is requires the\n"
"   \'mysqlclient\' library to be linked together.\n"
"\n"
"   The variables of Makefile\n"
"   MODNAME\n"
"          The name of the module\n"
"   MODLIBS\n"
"          The libraries that are required by the module.\n"
"   MODIDIR\n"
"          The SB\'s module directory. There will be installed the module.\n"
"   CINC\n"
"          \'Include\' path. This must points to the SB source files.\n"
"   CFLAGS\n"
"          Compilers flags.\n"
"\n"
"   --- Makefile ---\n"
"\n"
"MODNAME=mymod\n"
"MODLIBS=-lmysqlclient\n"
"MODIDIR=/usr/lib/sbasic/modules\n"
"CINC=-I/opt/sbasic/source\n"
"CFLAGS=-Wall -fPIC $(CINC) -D_UnixOS -DLNX_EXTLIB\n"
"\n"
"all: $(MODIDIR)/$(MODNAME).so\n"
"\n"
"$(MODIDIR)/$(MODNAME).so: $(MODNAME).c\n"
"    -mkdir -p $(MODIDIR)\n"
"    gcc $(CFLAGS) -c $(MODNAME).c -o $(MODNAME).o\n"
"    gcc -shared -Wl,-soname,$(MODNAME).so -o $(MODNAME).so $(MODNAME).o $(MODL\n"
"IBS)\n"
"    mv $(MODNAME).so $(MODIDIR)\n"
"    ldconfig -n $(MODIDIR)\n"
"\n"
"clean:\n"
"    -rm -f *.so *.o $(MODIDIR)/$(MODNAME).so\n"
"\n"
"                                 F. Glossary\n"
"\n"
"   What it could be good to know.\n"
"\n"
"   _\bA_\bN_\bS_\bI\n"
"          The American National Standards Institute. This organization\n"
"          produces many standards, among them the standards for the C\n"
"          and C++ programming languages. See also \"ISO\".\n"
"   _\bP_\br_\bo_\bg_\br_\ba_\bm\n"
"          An program consists of a series of _\bc_\bo_\bm_\bm_\ba_\bn_\bd_\bs, _\bs_\bt_\ba_\bt_\be_\bm_\be_\bn_\bt_\bs, and\n"
"          _\be_\bx_\bp_\br_\be_\bs_\bs_\bi_\bo_\bn_\bs. The program executed by an interpreted language\n"
"          command by command until it ends.\n"
"   _\bS_\bc_\br_\bi_\bp_\bt\n"
"          Another name for an program. ...\n"
"   _\bB_\bi_\bt\n"
"          Short for \"Binary Digit\". All values in computer memory\n"
"          ultimately reduce to binary digits: values that are either\n"
"          zero or one.\n"
"          Computers are often defined by how many bits they use to\n"
"          represent integer values. Typical systems are 32-bit systems,\n"
"          but 64-bit systems are becoming increasingly popular, and\n"
"          16-bit systems are waning in popularity.\n"
"   _\bC_\bh_\ba_\br_\ba_\bc_\bt_\be_\br_\b _\bS_\be_\bt\n"
"          The set of numeric codes used by a computer system to\n"
"          represent the characters (letters, numbers, punctuation, etc.)\n"
"          of a particular country or place. The most common character\n"
"          set in use today is ASCII (American Standard Code for\n"
"          Information Interchange). Many European countries use an\n"
"          extension of ASCII known as ISO-8859-1 (ISO Latin-1).\n"
"   _\bC_\bo_\bm_\bp_\bi_\bl_\be_\br\n"
"          A program that translates human-readable source code into\n"
"          machine-executable object code. The object code is then\n"
"          executed directly by the computer or by a virtual-machine. See\n"
"          also \"Interpreter\".\n"
"   _\bD_\be_\ba_\bd_\bl_\bo_\bc_\bk\n"
"          The situation in which two communicating processes are each\n"
"          waiting for the other to perform an action.\n"
"   _\bE_\bn_\bv_\bi_\br_\bo_\bn_\bm_\be_\bn_\bt_\b _\bV_\ba_\br_\bi_\ba_\bb_\bl_\be_\bs\n"
"          A collection of strings, of the form name=val, that each\n"
"          program has available to it. Users generally place values into\n"
"          the environment in order to provide information to various\n"
"          programs. Typical examples are the environment variables HOME\n"
"          and PATH.\n"
"   _\bE_\bs_\bc_\ba_\bp_\be_\b _\bS_\be_\bq_\bu_\be_\bn_\bc_\be_\bs\n"
"          A special sequence of characters used for describing\n"
"          nonprinting characters, such as `\\n\' for newline or `\\033\' for\n"
"          the ASCII ESC (Escape) character.\n"
"   _\bF_\bl_\ba_\bg\n"
"          A variable whose truth value indicates the existence or\n"
"          nonexistence of some condition.\n"
"   _\bF_\br_\be_\be_\b _\bS_\bo_\bf_\bt_\bw_\ba_\br_\be_\b _\bF_\bo_\bu_\bn_\bd_\ba_\bt_\bi_\bo_\bn\n"
"   _\bF_\bS_\bF\n"
"          A nonprofit organization dedicated to the production and\n"
"          distribution of freely distributable software. It was founded\n"
"          by Richard M. Stallman.\n"
"   _\bG_\bN_\bU_\b _\bG_\be_\bn_\be_\br_\ba_\bl_\b _\bP_\bu_\bb_\bl_\bi_\bc_\b _\bL_\bi_\bc_\be_\bn_\bs_\be\n"
"   _\bG_\bN_\bU_\b _\bG_\bP_\bL\n"
"          This document describes the terms under which binary library\n"
"          archives or shared objects, and their source code may be\n"
"          distributed.\n"
"          With few words, GPL allows source code and binary forms to be\n"
"          used copied and modified freely.\n"
"   _\bG_\bM_\bT\n"
"          \"Greenwich Mean Time\". It is the time of day used as the epoch\n"
"          for Unix and POSIX systems.\n"
"   _\bG_\bN_\bU\n"
"          \"GNU\'s not Unix\". An on-going project of the Free Software\n"
"          Foundation to create a complete, freely distributable,\n"
"          POSIX-compliant computing environment.\n"
"   _\bG_\bN_\bU_\b/_\bL_\bi_\bn_\bu_\bx\n"
"          A variant of the GNU system using the Linux kernel, instead of\n"
"          the Free Software Foundation\'s Hurd kernel. Linux is a stable,\n"
"          efficient, full-featured clone of Unix that has been ported to\n"
"          a variety of architectures. It is most popular on PC-class\n"
"          systems, but runs well on a variety of other systems too. The\n"
"          Linux kernel source code is available under the terms of the\n"
"          GNU General Public License, which is perhaps its most\n"
"          important aspect.\n"
"   _\bH_\be_\bx_\ba_\bd_\be_\bc_\bi_\bm_\ba_\bl\n"
"          Base 16 notation, where the digits are 0--9 and A--F, with `A\'\n"
"          representing 10, `B\' representing 11, and so on, up to `F\' for\n"
"          15. Hexadecimal numbers are written in SB using a leading `0x\'\n"
"          or `&H\', to indicate their base. Thus, 0x12 is 18 (1 times 16\n"
"          plus 2).\n"
"   _\bI_\b/_\bO\n"
"          Abbreviation for \"Input/Output\", the act of moving data into\n"
"          and/or out of a running program.\n"
"   _\bI_\bn_\bt_\be_\br_\bp_\br_\be_\bt_\be_\br\n"
"          A program that reads and executes human-readable source code\n"
"          directly. It uses the instructions in it to process data and\n"
"          produce results.\n"
"   _\bI_\bS_\bO\n"
"          The International Standards Organization. This organization\n"
"          produces international standards for many things, including\n"
"          programming languages, such as C and C++.\n"
"   _\bL_\be_\bs_\bs_\be_\br_\b _\bG_\be_\bn_\be_\br_\ba_\bl_\b _\bP_\bu_\bb_\bl_\bi_\bc_\b _\bL_\bi_\bc_\be_\bn_\bs_\be\n"
"   _\bL_\bG_\bP_\bL\n"
"          This document describes the terms under which binary library\n"
"          archives or shared objects, and their source code may be\n"
"          distributed.\n"
"   _\bO_\bc_\bt_\ba_\bl\n"
"          Base-eight notation, where the digits are 0--7. Octal numbers\n"
"          are written in SB using a leading `&o\', to indicate their\n"
"          base. Thus, &o13 is 11 (one times 8 plus 3).\n"
"   _\bP_\bO_\bS_\bI_\bX\n"
"          The name for a series of standards that specify a Portable\n"
"          Operating System interface. The \"IX\" denotes the Unix heritage\n"
"          of these standards.\n"
"   _\bP_\br_\bi_\bv_\ba_\bt_\be\n"
"          Variables and/or functions that are meant for use exclusively\n"
"          by this level of functions and not for the main program. See\n"
"          LOCAL, \"Nested Functions\".\n"
"   _\bR_\be_\bc_\bu_\br_\bs_\bi_\bo_\bn\n"
"          When a function calls itself, either directly or indirectly.\n"
"   _\bR_\be_\bd_\bi_\br_\be_\bc_\bt_\bi_\bo_\bn\n"
"          Redirection means performing input from something other than\n"
"          the standard input stream, or performing output to something\n"
"          other than the standard output stream.\n"
"          In Unices, you can redirect the output of the print statements\n"
"          to a file or a system command, using the `>\', `>>\', `|\', and\n"
"          `|&\' operators. You can redirect input to the INPUT statement\n"
"          using the `<\', `|\', and `|&\' operators.\n"
"   _\bR_\be_\bg_\bE_\bx_\bp\n"
"   _\bR_\be_\bg_\bu_\bl_\ba_\bt_\b _\bE_\bx_\bp_\br_\be_\bs_\bs_\bi_\bo_\bn\n"
"          Short for _\br_\be_\bg_\bu_\bl_\ba_\br_\b _\be_\bx_\bp_\br_\be_\bs_\bs_\bi_\bo_\bn. A regexp is a pattern that\n"
"          denotes a set of strings, possibly an infinite set. For\n"
"          example, the regexp `R.*xp\' matches any string starting with\n"
"          the letter `R\' and ending with the letters `xp\'.\n"
"   _\bS_\be_\ba_\br_\bc_\bh_\b _\bP_\ba_\bt_\bh\n"
"          In SB, a list of directories to search for SB program files.\n"
"          In the shell, a list of directories to search for executable\n"
"          programs.\n"
"   _\bS_\be_\be_\bd\n"
"          The initial value, or starting point, for a sequence of random\n"
"          numbers.\n"
"   _\bS_\bh_\be_\bl_\bl\n"
"          The command interpreter for Unix, POSIX-compliant systems, DOS\n"
"          and WinNT/2K/XP (CMD). The shell works both interactively, and\n"
"          as a programming language for batch files, or shell scripts.\n"
"   _\bU_\bn_\bi_\bx\n"
"          A computer operating system originally developed in the early\n"
"          1970\'s at AT&T Bell Laboratories. It initially became popular\n"
"          in universities around the world and later moved into\n"
"          commercial environments as a software development system and\n"
"          network server system. There are many commercial versions of\n"
"          Unix, as well as several work-alike systems whose source code\n"
"          is freely available (such as GNU/Linux, NetBSD, FreeBSD, and\n"
"          OpenBSD).\n"
"\n"
"                      G. GNU Free Documentation License\n"
"\n"
"                          Version 1.1, March 2000\n"
"\n"
"Copyright (C) 2000  Free Software Foundation, Inc.\n"
"59 Temple Place, Suite 330, Boston, MA  02111-1307  USA\n"
"\n"
"Everyone is permitted to copy and distribute verbatim copies\n"
"of this license document, but changing it is not allowed.\n"
"\n"
"    1. PREAMBLE\n"
"       The purpose of this License is to make a manual, textbook, or\n"
"       other written document \"free\" in the sense of freedom: to assure\n"
"       everyone the effective freedom to copy and redistribute it, with\n"
"       or without modifying it, either commercially or noncommercially.\n"
"       Secondarily, this License preserves for the author and publisher\n"
"       a way to get credit for their work, while not being considered\n"
"       responsible for modifications made by others.\n"
"       This License is a kind of \"copyleft\", which means that derivative\n"
"       works of the document must themselves be free in the same sense.\n"
"       It complements the GNU General Public License, which is a\n"
"       copyleft license designed for free software.\n"
"       We have designed this License in order to use it for manuals for\n"
"       free software, because free software needs free documentation: a\n"
"       free program should come with manuals providing the same freedoms\n"
"       that the software does. But this License is not limited to\n"
"       software manuals; it can be used for any textual work, regardless\n"
"       of subject matter or whether it is published as a printed book.\n"
"       We recommend this License principally for works whose purpose is\n"
"       instruction or reference.\n"
"    2. APPLICABILITY AND DEFINITIONS\n"
"       This License applies to any manual or other work that contains a\n"
"       notice placed by the copyright holder saying it can be\n"
"       distributed under the terms of this License. The \"Document\",\n"
"       below, refers to any such manual or work. Any member of the\n"
"       public is a licensee, and is addressed as \"you\".\n"
"       A \"Modified Version\" of the Document means any work containing\n"
"       the Document or a portion of it, either copied verbatim, or with\n"
"       modifications and/or translated into another language.\n"
"       A \"Secondary Section\" is a named appendix or a front-matter\n"
"       section of the Document that deals exclusively with the\n"
"       relationship of the publishers or authors of the Document to the\n"
"       Document\'s overall subject (or to related matters) and contains\n"
"       nothing that could fall directly within that overall subject.\n"
"       (For example, if the Document is in part a textbook of\n"
"       mathematics, a Secondary Section may not explain any\n"
"       mathematics.) The relationship could be a matter of historical\n"
"       connection with the subject or with related matters, or of legal,\n"
"       commercial, philosophical, ethical or political position\n"
"       regarding them.\n"
"       The \"Invariant Sections\" are certain Secondary Sections whose\n"
"       titles are designated, as being those of Invariant Sections, in\n"
"       the notice that says that the Document is released under this\n"
"       License.\n"
"       The \"Cover Texts\" are certain short passages of text that are\n"
"       listed, as Front-Cover Texts or Back-Cover Texts, in the notice\n"
"       that says that the Document is released under this License.\n"
"       A \"Transparent\" copy of the Document means a machine-readable\n"
"       copy, represented in a format whose specification is available to\n"
"       the general public, whose contents can be viewed and edited\n"
"       directly and straightforwardly with generic text editors or (for\n"
"       images composed of pixels) generic paint programs or (for\n"
"       drawings) some widely available drawing editor, and that is\n"
"       suitable for input to text formatters or for automatic\n"
"       translation to a variety of formats suitable for input to text\n"
"       formatters. A copy made in an otherwise Transparent file format\n"
"       whose markup has been designed to thwart or discourage subsequent\n"
"       modification by readers is not Transparent. A copy that is not\n"
"       \"Transparent\" is called \"Opaque\".\n"
"       Examples of suitable formats for Transparent copies include plain\n"
"       ASCII without markup, Texinfo input format, LaTeX input format,\n"
"       SGML or XML using a publicly available DTD, and\n"
"       standard-conforming simple HTML designed for human modification.\n"
"       Opaque formats include PostScript, PDF, proprietary formats that\n"
"       can be read and edited only by proprietary word processors, SGML\n"
"       or XML for which the DTD and/or processing tools are not\n"
"       generally available, and the machine-generated HTML produced by\n"
"       some word processors for output purposes only.\n"
"       The \"Title Page\" means, for a printed book, the title page\n"
"       itself, plus such following pages as are needed to hold, legibly,\n"
"       the material this License requires to appear in the title page.\n"
"       For works in formats which do not have any title page as such,\n"
"       \"Title Page\" means the text near the most prominent appearance of\n"
"       the work\'s title, preceding the beginning of the body of the\n"
"       text.\n"
"    3. VERBATIM COPYING\n"
"       You may copy and distribute the Document in any medium, either\n"
"       commercially or noncommercially, provided that this License, the\n"
"       copyright notices, and the license notice saying this License\n"
"       applies to the Document are reproduced in all copies, and that\n"
"       you add no other conditions whatsoever to those of this License.\n"
"       You may not use technical measures to obstruct or control the\n"
"       reading or further copying of the copies you make or distribute.\n"
"       However, you may accept compensation in exchange for copies. If\n"
"       you distribute a large enough number of copies you must also\n"
"       follow the conditions in section 3.\n"
"       You may also lend copies, under the same conditions stated above,\n"
"       and you may publicly display copies.\n"
"    4. COPYING IN QUANTITY\n"
"       If you publish printed copies of the Document numbering more than\n"
"       100, and the Document\'s license notice requires Cover Texts, you\n"
"       must enclose the copies in covers that carry, clearly and\n"
"       legibly, all these Cover Texts: Front-Cover Texts on the front\n"
"       cover, and Back-Cover Texts on the back cover. Both covers must\n"
"       also clearly and legibly identify you as the publisher of these\n"
"       copies. The front cover must present the full title with all\n"
"       words of the title equally prominent and visible. You may add\n"
"       other material on the covers in addition. Copying with changes\n"
"       limited to the covers, as long as they preserve the title of the\n"
"       Document and satisfy these conditions, can be treated as verbatim\n"
"       copying in other respects.\n"
"       If the required texts for either cover are too voluminous to fit\n"
"       legibly, you should put the first ones listed (as many as fit\n"
"       reasonably) on the actual cover, and continue the rest onto\n"
"       adjacent pages.\n"
"       If you publish or distribute Opaque copies of the Document\n"
"       numbering more than 100, you must either include a\n"
"       machine-readable Transparent copy along with each Opaque copy, or\n"
"       state in or with each Opaque copy a publicly-accessible\n"
"       computer-network location containing a complete Transparent copy\n"
"       of the Document, free of added material, which the general\n"
"       network-using public has access to download anonymously at no\n"
"       charge using public-standard network protocols. If you use the\n"
"       latter option, you must take reasonably prudent steps, when you\n"
"       begin distribution of Opaque copies in quantity, to ensure that\n"
"       this Transparent copy will remain thus accessible at the stated\n"
"       location until at least one year after the last time you\n"
"       distribute an Opaque copy (directly or through your agents or\n"
"       retailers) of that edition to the public.\n"
"       It is requested, but not required, that you contact the authors\n"
"       of the Document well before redistributing any large number of\n"
"       copies, to give them a chance to provide you with an updated\n"
"       version of the Document.\n"
"    5. MODIFICATIONS\n"
"       You may copy and distribute a Modified Version of the Document\n"
"       under the conditions of sections 2 and 3 above, provided that you\n"
"       release the Modified Version under precisely this License, with\n"
"       the Modified Version filling the role of the Document, thus\n"
"       licensing distribution and modification of the Modified Version\n"
"       to whoever possesses a copy of it. In addition, you must do these\n"
"       things in the Modified Version:\n"
"         1. Use in the Title Page (and on the covers, if any) a title\n"
"            distinct from that of the Document, and from those of\n"
"            previous versions (which should, if there were any, be\n"
"            listed in the History section of the Document). You may use\n"
"            the same title as a previous version if the original\n"
"            publisher of that version gives permission.\n"
"         2. List on the Title Page, as authors, one or more persons or\n"
"            entities responsible for authorship of the modifications in\n"
"            the Modified Version, together with at least five of the\n"
"            principal authors of the Document (all of its principal\n"
"            authors, if it has less than five).\n"
"         3. State on the Title page the name of the publisher of the\n"
"            Modified Version, as the publisher.\n"
"         4. Preserve all the copyright notices of the Document.\n"
"         5. Add an appropriate copyright notice for your modifications\n"
"            adjacent to the other copyright notices.\n"
"         6. Include, immediately after the copyright notices, a license\n"
"            notice giving the public permission to use the Modified\n"
"            Version under the terms of this License, in the form shown\n"
"            in the Addendum below.\n"
"         7. Preserve in that license notice the full lists of Invariant\n"
"            Sections and required Cover Texts given in the Document\'s\n"
"            license notice.\n"
"         8. Include an unaltered copy of this License.\n"
"         9. Preserve the section entitled \"History\", and its title, and\n"
"            add to it an item stating at least the title, year, new\n"
"            authors, and publisher of the Modified Version as given on\n"
"            the Title Page. If there is no section entitled \"History\" in\n"
"            the Document, create one stating the title, year, authors,\n"
"            and publisher of the Document as given on its Title Page,\n"
"            then add an item describing the Modified Version as stated\n"
"            in the previous sentence.\n"
"        10. Preserve the network location, if any, given in the Document\n"
"            for public access to a Transparent copy of the Document, and\n"
"            likewise the network locations given in the Document for\n"
"            previous versions it was based on. These may be placed in\n"
"            the \"History\" section. You may omit a network location for a\n"
"            work that was published at least four years before the\n"
"            Document itself, or if the original publisher of the version\n"
"            it refers to gives permission.\n"
"        11. In any section entitled \"Acknowledgements\" or \"Dedications\",\n"
"            preserve the section\'s title, and preserve in the section\n"
"            all the substance and tone of each of the contributor\n"
"            acknowledgements and/or dedications given therein.\n"
"        12. Preserve all the Invariant Sections of the Document,\n"
"            unaltered in their text and in their titles. Section numbers\n"
"            or the equivalent are not considered part of the section\n"
"            titles.\n"
"        13. Delete any section entitled \"Endorsements\". Such a section\n"
"            may not be included in the Modified Version.\n"
"        14. Do not retitle any existing section as \"Endorsements\" or to\n"
"            conflict in title with any Invariant Section.\n"
"       If the Modified Version includes new front-matter sections or\n"
"       appendices that qualify as Secondary Sections and contain no\n"
"       material copied from the Document, you may at your option\n"
"       designate some or all of these sections as invariant. To do this,\n"
"       add their titles to the list of Invariant Sections in the\n"
"       Modified Version\'s license notice. These titles must be distinct\n"
"       from any other section titles.\n"
"       You may add a section entitled \"Endorsements\", provided it\n"
"       contains nothing but endorsements of your Modified Version by\n"
"       various parties--for example, statements of peer review or that\n"
"       the text has been approved by an organization as the\n"
"       authoritative definition of a standard.\n"
"       You may add a passage of up to five words as a Front-Cover Text,\n"
"       and a passage of up to 25 words as a Back-Cover Text, to the end\n"
"       of the list of Cover Texts in the Modified Version. Only one\n"
"       passage of Front-Cover Text and one of Back-Cover Text may be\n"
"       added by (or through arrangements made by) any one entity. If the\n"
"       Document already includes a cover text for the same cover,\n"
"       previously added by you or by arrangement made by the same entity\n"
"       you are acting on behalf of, you may not add another; but you may\n"
"       replace the old one, on explicit permission from the previous\n"
"       publisher that added the old one.\n"
"       The author(s) and publisher(s) of the Document do not by this\n"
"       License give permission to use their names for publicity for or\n"
"       to assert or imply endorsement of any Modified Version.\n"
"    6. COMBINING DOCUMENTS\n"
"       You may combine the Document with other documents released under\n"
"       this License, under the terms defined in section 4 above for\n"
"       modified versions, provided that you include in the combination\n"
"       all of the Invariant Sections of all of the original documents,\n"
"       unmodified, and list them all as Invariant Sections of your\n"
"       combined work in its license notice.\n"
"       The combined work need only contain one copy of this License, and\n"
"       multiple identical Invariant Sections may be replaced with a\n"
"       single copy. If there are multiple Invariant Sections with the\n"
"       same name but different contents, make the title of each such\n"
"       section unique by adding at the end of it, in parentheses, the\n"
"       name of the original author or publisher of that section if\n"
"       known, or else a unique number. Make the same adjustment to the\n"
"       section titles in the list of Invariant Sections in the license\n"
"       notice of the combined work.\n"
"       In the combination, you must combine any sections entitled\n"
"       \"History\" in the various original documents, forming one section\n"
"       entitled \"History\"; likewise combine any sections entitled\n"
"       \"Acknowledgements\", and any sections entitled \"Dedications\". You\n"
"       must delete all sections entitled \"Endorsements.\"\n"
"    7. COLLECTIONS OF DOCUMENTS\n"
"       You may make a collection consisting of the Document and other\n"
"       documents released under this License, and replace the individual\n"
"       copies of this License in the various documents with a single\n"
"       copy that is included in the collection, provided that you follow\n"
"       the rules of this License for verbatim copying of each of the\n"
"       documents in all other respects.\n"
"       You may extract a single document from such a collection, and\n"
"       distribute it individually under this License, provided you\n"
"       insert a copy of this License into the extracted document, and\n"
"       follow this License in all other respects regarding verbatim\n"
"       copying of that document.\n"
"    8. AGGREGATION WITH INDEPENDENT WORKS\n"
"       A compilation of the Document or its derivatives with other\n"
"       separate and independent documents or works, in or on a volume of\n"
"       a storage or distribution medium, does not as a whole count as a\n"
"       Modified Version of the Document, provided no compilation\n"
"       copyright is claimed for the compilation. Such a compilation is\n"
"       called an \"aggregate\", and this License does not apply to the\n"
"       other self-contained works thus compiled with the Document, on\n"
"       account of their being thus compiled, if they are not themselves\n"
"       derivative works of the Document.\n"
"       If the Cover Text requirement of section 3 is applicable to these\n"
"       copies of the Document, then if the Document is less than one\n"
"       quarter of the entire aggregate, the Document\'s Cover Texts may\n"
"       be placed on covers that surround only the Document within the\n"
"       aggregate. Otherwise they must appear on covers around the whole\n"
"       aggregate.\n"
"    9. TRANSLATION\n"
"       Translation is considered a kind of modification, so you may\n"
"       distribute translations of the Document under the terms of\n"
"       section 4. Replacing Invariant Sections with translations\n"
"       requires special permission from their copyright holders, but you\n"
"       may include translations of some or all Invariant Sections in\n"
"       addition to the original versions of these Invariant Sections.\n"
"       You may include a translation of this License provided that you\n"
"       also include the original English version of this License. In\n"
"       case of a disagreement between the translation and the original\n"
"       English version of this License, the original English version\n"
"       will prevail.\n"
"   10. TERMINATION\n"
"       You may not copy, modify, sublicense, or distribute the Document\n"
"       except as expressly provided for under this License. Any other\n"
"       attempt to copy, modify, sublicense or distribute the Document is\n"
"       void, and will automatically terminate your rights under this\n"
"       License. However, parties who have received copies, or rights,\n"
"       from you under this License will not have their licenses\n"
"       terminated so long as such parties remain in full compliance.\n"
"   11. FUTURE REVISIONS OF THIS LICENSE\n"
"       The Free Software Foundation may publish new, revised versions of\n"
"       the GNU Free Documentation License from time to time. Such new\n"
"       versions will be similar in spirit to the present version, but\n"
"       may differ in detail to address new problems or concerns. See\n"
"       [25]h\bht\btt\btp\bp:\b:/\b//\b/w\bww\bww\bw.\b.g\bgn\bnu\bu.\b.o\bor\brg\bg/\b/c\bco\bop\bpy\byl\ble\bef\bft\bt/\b/.\n"
"       Each version of the License is given a distinguishing version\n"
"       number. If the Document specifies that a particular numbered\n"
"       version of this License \"or any later version\" applies to it, you\n"
"       have the option of following the terms and conditions either of\n"
"       that specified version or of any later version that has been\n"
"       published (not as a draft) by the Free Software Foundation. If\n"
"       the Document does not specify a version number of this License,\n"
"       you may choose any version ever published (not as a draft) by the\n"
"       Free Software Foundation.\n"
"\n"
"G.1 ADDENDUM: How to use this License for your documents\n"
"\n"
"   To use this License in a document you have written, include a copy of\n"
"   the License in the document and put the following copyright and\n"
"   license notices just after the title page:\n"
"\n"
"  Copyright (C)  year  your name.\n"
"  Permission is granted to copy, distribute and/or modify this document\n"
"  under the terms of the GNU Free Documentation License, Version 1.1\n"
"  or any later version published by the Free Software Foundation;\n"
"  with the Invariant Sections being list their titles, with the\n"
"  Front-Cover Texts being list, and with the Back-Cover Texts being list.\n"
"  A copy of the license is included in the section entitled ``GNU\n"
"  Free Documentation License\'\'.\n"
"\n"
"   If you have no Invariant Sections, write \"with no Invariant Sections\"\n"
"   instead of saying which ones are invariant. If you have no\n"
"   Front-Cover Texts, write \"no Front-Cover Texts\" instead of\n"
"   \"Front-Cover Texts being list\"; likewise for Back-Cover Texts.\n"
"\n"
"   If your document contains nontrivial examples of program code, we\n"
"   recommend releasing these examples in parallel under your choice of\n"
"   free software license, such as the GNU General Public License, to\n"
"   permit their use in free software.\n"
"\n"
},

{ "IF", "FUNCTION", 
"_\bI_\bF _\b(_\be_\bx_\bp_\br_\be_\bs_\bs_\bi_\bo_\bn_\b,_\b _\bt_\br_\bu_\be_\b-_\bv_\ba_\bl_\bu_\be_\b,_\b _\bf_\ba_\bl_\bs_\be_\b-_\bv_\ba_\bl_\bu_\be_\b)", 
"          Returns a value based on the value of an expression.\n"
"\n"
"          Example:\n"
"\n"
"x=0\n"
"PRINT IF(x<>0,\"true\",\"false\") : REM prints false\n"
"\n"
},

{ "LEN", "FUNCTION", 
"_\bL_\bE_\bN _\b(_\bx_\b)", 
"\n"
"        x\n"
"               Any variable.\n"
"\n"
"          If x is a string, returns the length of the string. If x is an\n"
"          array, returns the number of the elements. If x is an number,\n"
"          returns the length of the STR(x).\n"
"\n"
},

{ "EMPTY", "FUNCTION", 
"_\bE_\bM_\bP_\bT_\bY _\b(_\bx_\b)", 
"\n"
"        x\n"
"               Any variable.\n"
"\n"
"          If x is a string, returns true if the len(x) is 0. If x is an\n"
"          integer or a real returns true if the x = 0. If x is an array,\n"
"          returns true if x is a zero-length array (array without\n"
"          elements).\n"
"\n"
},

{ "ISARRAY", "FUNCTION", 
"_\bI_\bS_\bA_\bR_\bR_\bA_\bY _\b(_\bx_\b)", 
"\n"
"        x\n"
"               Any variable.\n"
"\n"
"          Returns true if the x is an array.\n"
"\n"
},

{ "ISNUMBER", "FUNCTION", 
"_\bI_\bS_\bN_\bU_\bM_\bB_\bE_\bR _\b(_\bx_\b)", 
"\n"
"        x\n"
"               Any variable.\n"
"\n"
"          Returns true if the x is a number (or it can be converted to a\n"
"          number)\n"
"\n"
"          Example:\n"
"\n"
"? ISNUMBER(12)          :REM true\n"
"? ISNUMBER(\"12\")        :REM true\n"
"? ISNUMBER(\"12E+2\")     :REM true\n"
"? ISNUMBER(\"abc\")       :REM false\n"
"? ISNUMBER(\"1+2\")       :REM false\n"
"? ISNUMBER(\"int(2.4)\")  :REM false\n"
"\n"
},

{ "ISSTRING", "FUNCTION", 
"_\bI_\bS_\bS_\bT_\bR_\bI_\bN_\bG _\b(_\bx_\b)", 
"\n"
"        x\n"
"               Any variable.\n"
"\n"
"          Returns true if the x is a string (and cannot be converted to\n"
"          a number)\n"
"\n"
"          Example:\n"
"\n"
"? ISSTRING(12)      :REM false\n"
"? ISSTRING(\"12\")    :REM false\n"
"? ISSTRING(\"12E+2\") :REM false\n"
"? ISSTRING(\"abc\")   :REM true\n"
"? ISSTRING(\"1+2\")   :REM true\n"
"\n"
},

{ "FRE", "FUNCTION", 
"_\bF_\bR_\bE _\b(_\bx_\b)", 
"          Returns system information\n"
"\n"
"          Where x:\n"
"\n"
"          QB-standard:\n"
"\n"
"          0  free memory\n"
"          -1 largest block of integers\n"
"          -2 free stack\n"
"          -3 largest free block\n"
"\n"
"          Our standard (it is optional for now):\n"
"\n"
"          -10 total physical memory\n"
"          -11 used physical memory\n"
"          -12 free physical memory\n"
"\n"
"          Optional-set #1:\n"
"\n"
"          -13 shared memory size\n"
"          -14 buffers\n"
"          -15 cached\n"
"          -16 total virtual memory size\n"
"          -17 used virtual memory\n"
"          -18 free virtual memory\n"
"\n"
"          Optional-set #2:\n"
"\n"
"          -40 battery voltage * 1000\n"
"          -41 battery percent\n"
"          -42 critical voltage value (*1000)\n"
"          -43 warning voltage value (*1000)\n"
"\n"
"          The optional values will returns 0 if are not supported.\n"
"\n"
},

{ "TICKS", "FUNCTION", 
"_\bT_\bI_\bC_\bK_\bS", 
"          Returns the system-ticks. The tick value is depended on\n"
"          operating system.\n"
"\n"
},

{ "TICKSPERSEC", "FUNCTION", 
"_\bT_\bI_\bC_\bK_\bS_\bP_\bE_\bR_\bS_\bE_\bC", 
"          Returns the number of ticks per second\n"
"\n"
},

{ "TIMER", "FUNCTION", 
"_\bT_\bI_\bM_\bE_\bR", 
"          Returns the number of seconds from midnight\n"
"\n"
},

{ "TIME", "FUNCTION", 
"_\bT_\bI_\bM_\bE", 
"          Returns the current time as string \"HH:MM:SS\"\n"
"\n"
},

{ "DATE", "FUNCTION", 
"_\bD_\bA_\bT_\bE", 
"          Returns the current day as string \"DD/MM/YYYY\"\n"
"\n"
},

{ "JULIAN", "FUNCTION", 
"_\bJ_\bU_\bL_\bI_\bA_\bN _\b(_\bd_\bm_\by_\b _\b|_\b _\b(_\bd_\b,_\bm_\b,_\by_\b)_\b)", 
"          Returns the Julian date. (dates must be greater than 1/1/100\n"
"          AD)\n"
"\n"
"          Example:\n"
"\n"
"PRINT Julian(DATE)\n"
"PRINT Julian(31, 12, 2001)\n"
"\n"
},

{ "WEEKDAY", "FUNCTION", 
"_\bW_\bE_\bE_\bK_\bD_\bA_\bY _\b(_\bd_\bm_\by_\b _\b|_\b _\b(_\bd_\b,_\bm_\b,_\by_\b)_\b _\b|_\b _\bj_\bu_\bl_\bi_\ba_\bn_\b__\bd_\ba_\bt_\be_\b)", 
"          Returns the day of the week (0 = Sunday)\n"
"\n"
"PRINT WeekDay(DATE)\n"
"PRINT WeekDay(Julian(31, 12, 2001))\n"
"PRINT WeekDay(31, 12, 2001)\n"
"\n"
},

{ "DATEFMT", "FUNCTION", 
"_\bD_\bA_\bT_\bE_\bF_\bM_\bT _\b(_\bf_\bo_\br_\bm_\ba_\bt_\b,_\b _\bd_\bm_\by_\b _\b|_\b _\b(_\bd_\b,_\bm_\b,_\by_\b)_\b _\b|_\b _\bj_\bu_\bl_\bi_\ba_\bn_\b__\bd_\ba_\bt_\be_\b)", 
"          Returns formated date string\n"
"\n"
"          Format:\n"
"\n"
"          D    one or two digits of Day\n"
"          DD   2-digit day\n"
"          DDD  3-char day name\n"
"          DDDD full day name\n"
"          M    1 or 2 digits of month\n"
"          MM   2-digit month\n"
"          MMM  3-char month name\n"
"          MMMM full month name\n"
"          YY   2-digit year (2K)\n"
"          YYYY 4-digit year\n"
"\n"
"PRINT DATEFMT(\"ddd dd, mm/yy\", \"23/11/2001\")\n"
"REM prints \"Fri 23, 11/01\"\n"
"\n"
},

{ "ENV", "FUNCTION", 
"_\bE_\bN_\bV _\b(_\b\"_\bv_\ba_\br_\b\"_\b)", 
"\n"
},

{ "ENVIRON", "FUNCTION", 
"_\bE_\bN_\bV_\bI_\bR_\bO_\bN _\b(_\b\"_\bv_\ba_\br_\b\"_\b)", 
"          Returns the value of a specified entry in the current\n"
"          environment table. If the parameter is empty (\"\") then returns\n"
"          an array of the envirment variables (in var=value form)\n"
"\n"
"        var\n"
"               A string expression of the form \"var\"\n"
"\n"
"          _\bP_\ba_\bl_\bm_\bO_\bS _\bS_\bB_\b _\be_\bm_\bu_\bl_\ba_\bt_\be_\bs_\b _\be_\bn_\bv_\bi_\br_\bo_\bn_\bm_\be_\bn_\bt_\b _\bv_\ba_\br_\bi_\ba_\bb_\bl_\be_\bs_\b.\n"
"\n"
},

{ "RUN", "FUNCTION", 
"_\bR_\bU_\bN _\b(_\b\"_\bc_\bo_\bm_\bm_\ba_\bn_\bd_\b\"_\b)", 
"          RUN() is the function version of the RUN command. The\n"
"          difference is that, the RUN() returns a string with the output\n"
"          of the \'command\' as an array of strings (each text-line is one\n"
"          element).\n"
"\n"
"          _\bP_\ba_\bl_\bm_\bO_\bS _\bT_\bh_\be_\b _\bR_\bU_\bN_\b(_\b)_\b _\bd_\bo_\be_\bs_\b _\bn_\bo_\bt_\b _\bs_\bu_\bp_\bp_\bo_\br_\bt_\be_\bd_\b.\n"
"          _\bW_\bi_\bn_\bd_\bo_\bw_\bs _\bT_\bh_\be_\b _\bs_\bt_\bd_\bo_\bu_\bt_\b _\ba_\bn_\bd_\b _\bs_\bt_\bd_\be_\br_\br_\b _\ba_\br_\be_\b _\bs_\be_\bp_\ba_\br_\ba_\bt_\be_\bd_\b!_\b _\bF_\bi_\br_\bs_\bt_\b _\bi_\bs_\b _\bt_\bh_\be\n"
"          _\bs_\bt_\bd_\bo_\bu_\bt_\b _\bo_\bu_\bt_\bp_\bu_\bt_\b _\ba_\bn_\bd_\b _\bf_\bo_\bl_\bl_\bo_\bw_\bi_\bn_\bg_\b _\bt_\bh_\be_\b _\bs_\bt_\bd_\be_\br_\br_\b.\n"
"\n"
},

{ "MALLOC", "FUNCTION", 
"_\bM_\bA_\bL_\bL_\bO_\bC _\b(_\bs_\bi_\bz_\be_\b)", 
"\n"
},

{ "BALLOC", "FUNCTION", 
"_\bB_\bA_\bL_\bL_\bO_\bC _\b(_\bs_\bi_\bz_\be_\b)", 
"          Allocates a memory block.\n"
"\n"
"          _\b* _\bT_\bh_\be_\b _\bv_\ba_\br_\bi_\ba_\bb_\bl_\be_\b _\bc_\ba_\bn_\b _\bb_\be_\b _\bf_\br_\be_\be_\bd_\b _\bb_\by_\b _\bu_\bs_\bi_\bn_\bg_\b _\bE_\bR_\bA_\bS_\bE_\b.\n"
"\n"
},

{ "VADR", "FUNCTION", 
"_\bV_\bA_\bD_\bR _\b(_\bv_\ba_\br_\b)", 
"          Returns the memory address of the variable\'s data.\n"
"\n"
},

{ "PEEK[{16|32}]", "FUNCTION", 
"_\bP_\bE_\bE_\bK_\b[_\b{_\b1_\b6_\b|_\b3_\b2_\b}_\b] _\b(_\ba_\bd_\bd_\br_\b)", 
"          Returns the byte, word or dword at a specified memory address.\n"
"\n"
},

{ "TXTW", "FUNCTION", 
"_\bT_\bX_\bT_\bW _\b(_\bs_\b)", 
"\n"
},

{ "TEXTWIDTH", "FUNCTION", 
"_\bT_\bE_\bX_\bT_\bW_\bI_\bD_\bT_\bH _\b(_\bs_\b)", 
"          Returns the text width of string s in pixels\n"
"\n"
},

{ "TXTH", "FUNCTION", 
"_\bT_\bX_\bT_\bH _\b(_\bs_\b)", 
"\n"
},

{ "TEXTHEIGHT", "FUNCTION", 
"_\bT_\bE_\bX_\bT_\bH_\bE_\bI_\bG_\bH_\bT _\b(_\bs_\b)", 
"          Returns the text height of string s in pixels\n"
"\n"
},

{ "XPOS", "FUNCTION", 
"_\bX_\bP_\bO_\bS", 
"\n"
},

{ "YPOS", "FUNCTION", 
"_\bY_\bP_\bO_\bS", 
"          Returns the current position of the cursor in \"characters\".\n"
"\n"
},

{ "POINT", "FUNCTION", 
"_\bP_\bO_\bI_\bN_\bT _\b(_\bx_\b _\b[_\b,_\b _\by_\b]_\b)", 
"          Returns the color of the pixel at x,y\n"
"\n"
"          if y does not specified x contains the info-code\n"
"          0 = returns the current X graphics position\n"
"          1 = returns the current Y graphics position\n"
"\n"
},

{ "RGB", "FUNCTION", 
"_\bR_\bG_\bB _\b(_\br_\b,_\b _\bg_\b,_\b _\bb_\b)", 
"          The RGB functions returns the RGB color codes for the\n"
"          specified values The RGB() takes values 0..255 for each of the\n"
"          color.\n"
"\n"
"          The return value is a negative 24bit value to by used by\n"
"          drawing functions.\n"
"\n"
},

{ "RGBF", "FUNCTION", 
"_\bR_\bG_\bB_\bF _\b(_\br_\b,_\b _\bg_\b,_\b _\bb_\b)", 
"          The RGBF functions returns the RGB color codes for the\n"
"          specified values The RGBF() takes values 0..1 for each of the\n"
"          color.\n"
"\n"
"          The return value is a negative 24bit value to by used by\n"
"          drawing functions.\n"
"\n"
},

{ "RND", "FUNCTION", 
"_\bR_\bN_\bD", 
"          Returns a random number from the range 0 to 1\n"
"\n"
},

{ "UBOUND", "FUNCTION", 
"_\bU_\bB_\bO_\bU_\bN_\bD _\b(_\ba_\br_\br_\ba_\by_\b _\b[_\b,_\b _\bd_\bi_\bm_\b]_\b)", 
"          Returns the upper bound of the \'array\'\n"
"\n"
},

{ "LBOUND", "FUNCTION", 
"_\bL_\bB_\bO_\bU_\bN_\bD _\b(_\ba_\br_\br_\ba_\by_\b _\b[_\b,_\b _\bd_\bi_\bm_\b]_\b)", 
"          Returns the lower bound of the \'array\'\n"
"\n"
"          The parameter \'dim\' is the array dimension whose bound is\n"
"          returned\n"
"\n"
"DIM v1(-4 TO 7)\n"
"DIM v2(1 TO 2, 3 TO 4)\n"
"...\n"
"PRINT LBOUND(v1)   : REM -4\n"
"PRINT UBOUND(v1)   : REM 7\n"
"...\n"
"PRINT LBOUND(v2)   : REM 1\n"
"PRINT LBOUND(v2,2) : REM 3\n"
"\n"
},

{ "CINT", "FUNCTION", 
"_\bC_\bI_\bN_\bT _\b(_\bx_\b)", 
"          Converts x to 32b integer Meaningless. Used for compatibility.\n"
"\n"
},

{ "CREAL", "FUNCTION", 
"_\bC_\bR_\bE_\bA_\bL _\b(_\bx_\b)", 
"          Convert x to 64b real number. Meaningless. Used for\n"
"          compatibility.\n"
"\n"
},

{ "CDBL", "FUNCTION", 
"_\bC_\bD_\bB_\bL _\b(_\bx_\b)", 
"          Convert x to 64b real number. Meaningless. Used for\n"
"          compatibility.\n"
"\n"
},

{ "PEN", "FUNCTION", 
"_\bP_\bE_\bN _\b(_\b0_\b._\b._\b1_\b4_\b)", 
"          Returns the PEN/MOUSE data.\n"
"\n"
"          Values:\n"
"\n"
"        0\n"
"               true (non zero) if there is a new pen or mouse event\n"
"        1\n"
"               PEN: last pen down x; MOUSE: last mouse button down x\n"
"        2\n"
"               Same as 1 for y\n"
"        3\n"
"               true if the PEN is down; MOUSE: mouse left button is\n"
"               pressed\n"
"        4\n"
"               PEN: last/current x, MOUSE: the current x position only\n"
"               if the left mouse button is pressed (like PEN is down)\n"
"        5\n"
"               Same as PEN(4) for y\n"
"\n"
"          Mouse specific (non PalmOS):\n"
"\n"
"        10\n"
"               current mouse x pos\n"
"        11\n"
"               current mouse y pos\n"
"        12\n"
"               true if the left mouse button is pressed\n"
"        13\n"
"               true if the right mouse button is pressed\n"
"        14\n"
"               true if the middle mouse button is pressed\n"
"\n"
"          _\b* _\bT_\bh_\be_\b _\bd_\br_\bi_\bv_\be_\br_\b _\bm_\bu_\bs_\bt_\b _\bb_\be_\b _\be_\bn_\ba_\bb_\bl_\be_\bd_\b _\bb_\be_\bf_\bo_\br_\be_\b _\bu_\bs_\be_\b _\bt_\bh_\bi_\bs_\b _\bf_\bu_\bn_\bc_\bt_\bi_\bo_\bn_\b _\b(_\bs_\be_\be_\b _\bP_\be_\bn\n"
"          _\bc_\bo_\bm_\bm_\ba_\bn_\bd_\b)\n"
"\n"
},

{ "FREEFILE", "FUNCTION", 
"_\bF_\bR_\bE_\bE_\bF_\bI_\bL_\bE", 
"          Returns an unused file handle\n"
"\n"
},

{ "EXIST", "FUNCTION", 
"_\bE_\bX_\bI_\bS_\bT _\b(_\bf_\bi_\bl_\be_\b)", 
"          Returns true if the file exists\n"
"\n"
"        file\n"
"               A string expression that follows OS file naming\n"
"               conventions.\n"
"\n"
},

{ "ACCESS", "FUNCTION", 
"_\bA_\bC_\bC_\bE_\bS_\bS _\b(_\bf_\bi_\bl_\be_\b)", 
"          Returns the access rights of the file.\n"
"\n"
"        file\n"
"               A string expression that follows OS file naming\n"
"               conventions.\n"
"\n"
"          The return-value is the permissions of the file as them as\n"
"          specified on GNU\'s manual (chmod() and stat() system calls)\n"
"\n"
"          The bits (in octal):\n"
"\n"
"          04000 set user ID on execution\n"
"          02000 set group ID on execution\n"
"          01000 sticky bit\n"
"          00400 read by owner\n"
"          00200 write by owner\n"
"          00100 execute/search by owner\n"
"          00040 read by group\n"
"          00020 write by group\n"
"          00010 execute/search by group\n"
"          00004 read by others\n"
"          00002 write by others\n"
"          00001 execute/search by others\n"
"\n"
"          _\bP_\ba_\bl_\bm_\bO_\bS _\bT_\bh_\be_\b _\br_\be_\bt_\bu_\br_\bn_\b _\bv_\ba_\bl_\bu_\be_\b _\bi_\bs_\b _\ba_\bl_\bw_\ba_\by_\bs_\b _\b0_\b7_\b7_\b7_\b.\n"
"          _\bD_\bO_\bS _\bT_\bh_\be_\b _\br_\be_\bt_\bu_\br_\bn_\b _\bv_\ba_\bl_\bu_\be_\b _\bi_\bs_\b _\bd_\be_\bp_\be_\bn_\bd_\be_\bd_\b _\bo_\bn_\b _\bD_\bJ_\bG_\bP_\bP_\b\'_\bs_\b _\bs_\bt_\ba_\bt_\b(_\b)_\b _\bf_\bu_\bn_\bc_\bt_\bi_\bo_\bn_\b.\n"
"          _\bP_\bo_\bs_\bs_\bi_\bb_\bl_\be_\b _\bU_\bn_\bi_\bx_\b _\bc_\bo_\bm_\bp_\ba_\bt_\bi_\bb_\bl_\be_\b.\n"
"          _\bW_\bi_\bn_\bd_\bo_\bw_\bs _\bT_\bh_\be_\b _\br_\be_\bt_\bu_\br_\bn_\b _\bv_\ba_\bl_\bu_\be_\b _\bi_\bs_\b _\bd_\be_\bp_\be_\bn_\bd_\be_\bd_\b _\bo_\bn_\b _\bC_\by_\bg_\bn_\bu_\bs_\b\'_\bs_\b _\bs_\bt_\ba_\bt_\b(_\b)\n"
"          _\bf_\bu_\bn_\bc_\bt_\bi_\bo_\bn_\b._\b _\bP_\bo_\bs_\bs_\bi_\bb_\bl_\be_\b _\bU_\bn_\bi_\bx_\b _\bc_\bo_\bm_\bp_\ba_\bt_\bi_\bb_\bl_\be_\b.\n"
"\n"
"IF ACCESS(\"/bin/sh\") AND 0o4 THEN\n"
"    PRINT \"I can read it!\"\n"
"ENDIF\n"
"\n"
},

{ "ISFILE", "FUNCTION", 
"_\bI_\bS_\bF_\bI_\bL_\bE _\b(_\bf_\bi_\bl_\be_\b)", 
"          Returns true if the file is a regular file.\n"
"\n"
},

{ "ISDIR", "FUNCTION", 
"_\bI_\bS_\bD_\bI_\bR _\b(_\bf_\bi_\bl_\be_\b)", 
"          Returns true if the file is a directory.\n"
"\n"
},

{ "ISLINK", "FUNCTION", 
"_\bI_\bS_\bL_\bI_\bN_\bK _\b(_\bf_\bi_\bl_\be_\b)", 
"          Returns true if the file is a link.\n"
"\n"
},

{ "EOF", "FUNCTION", 
"_\bE_\bO_\bF _\b(_\bf_\bi_\bl_\be_\bN_\b)", 
"          Returns true if the file pointer is at end of the file. For\n"
"          COMx and SOCL VFS it returns true if the connection is broken.\n"
"\n"
},

{ "INPUT", "FUNCTION", 
"_\bI_\bN_\bP_\bU_\bT _\b(_\bl_\be_\bn_\b _\b[_\b,_\b _\bf_\bi_\bl_\be_\bN_\b]_\b)", 
"          This function is similar to INPUT. Reads \'len\' bytes from file\n"
"          or console (if fileN is omitted). This function is a low-level\n"
"          function. That means does not convert the data, and does not\n"
"          remove the spaces.\n"
"\n"
},

{ "BGETC", "FUNCTION", 
"_\bB_\bG_\bE_\bT_\bC _\b(_\bf_\bi_\bl_\be_\bN_\b)", 
"          (Binary mode) Reads and returns a byte from file or device.\n"
"\n"
},

{ "SEEK", "FUNCTION", 
"_\bS_\bE_\bE_\bK _\b(_\bf_\bi_\bl_\be_\bN_\b)", 
"          Returns the current file position\n"
"\n"
},

{ "LOF", "FUNCTION", 
"_\bL_\bO_\bF _\b(_\bf_\bi_\bl_\be_\bN_\b)", 
"          Returns the length of file in bytes. For other devices, it\n"
"          returns the number of available data.\n"
"\n"
},

{ "FILES", "FUNCTION", 
"_\bF_\bI_\bL_\bE_\bS _\b(_\bw_\bi_\bl_\bd_\bc_\ba_\br_\bd_\bs_\b)", 
"          Returns an array with the filenames. If there is no files\n"
"          returns an empty array.\n"
"\n"
"? FILES(\"*\")\n"
"\n"
"          _\bP_\ba_\bl_\bm_\bO_\bS _\bR_\be_\bt_\bu_\br_\bn_\bs_\b _\bo_\bn_\bl_\by_\b _\bt_\bh_\be_\b _\bu_\bs_\be_\br_\b-_\bf_\bi_\bl_\be_\bs_\b.\n"
"          _\b* _\bT_\bo_\b _\bu_\bs_\be_\b _\bf_\bi_\bl_\be_\b _\bo_\bn_\b _\bM_\bE_\bM_\bO_\b _\bo_\br_\b _\bP_\bD_\bO_\bC_\b _\bo_\br_\b _\ba_\bn_\by_\b _\bo_\bt_\bh_\be_\br_\b _\bv_\bi_\br_\bt_\bu_\ba_\bl_\b _\bf_\bi_\bl_\be_\b _\bs_\by_\bs_\bt_\be_\bm\n"
"          _\by_\bo_\bu_\b _\bm_\bu_\bs_\bt_\b _\bu_\bs_\be_\b _\bF_\bI_\bL_\bE_\bS_\b(_\b\"_\bV_\bF_\bS_\bx_\b:_\b*_\b\"_\b)\n"
"\n"
"PRINT FILES(\"MEMO:*\")\n"
"\n"
"                                9. Mathematics\n"
"\n"
"   All angles are in radians.\n"
"\n"
},

{ "ABS", "FUNCTION", 
"_\bA_\bB_\bS _\b(_\bx_\b)", 
"          Returns the absolute value of x.\n"
"\n"
},

{ "MAX", "FUNCTION", 
"_\bM_\bA_\bX _\b(_\b._\b._\b._\b)", 
"\n"
},

{ "ABSMAX", "FUNCTION", 
"_\bA_\bB_\bS_\bM_\bA_\bX _\b(_\b._\b._\b._\b)", 
"\n"
},

{ "MIN", "FUNCTION", 
"_\bM_\bI_\bN _\b(_\b._\b._\b._\b)", 
"\n"
},

{ "ABSMIN", "FUNCTION", 
"_\bA_\bB_\bS_\bM_\bI_\bN _\b(_\b._\b._\b._\b)", 
"          Maximum/Minimum value of parameters. Parameters can be\n"
"          anything (arrays, ints, reals, strings). ABSMIN/ABSMAX returns\n"
"          the absolute min/max value.\n"
"\n"
"? MAX(3,4,8)\n"
"? MIN(array(),2,3)\n"
"? MAX(\"abc\",\"def\")\n"
"\n"
},

{ "SEQ", "FUNCTION", 
"_\bS_\bE_\bQ _\b(_\bx_\bm_\bi_\bn_\b,_\b _\bx_\bm_\ba_\bx_\b,_\b _\bc_\bo_\bu_\bn_\bt_\b)", 
"          Returns an array with \'count\' elements. Each element had the x\n"
"          value of its position.\n"
"\n"
"? SEQ(0,1,11)\n"
"\n"
},

{ "POW", "FUNCTION", 
"_\bP_\bO_\bW _\b(_\bx_\b,_\b _\by_\b)", 
"          x raised to power of y\n"
"\n"
},

{ "SQR", "FUNCTION", 
"_\bS_\bQ_\bR _\b(_\bx_\b)", 
"          Square root of x\n"
"\n"
},

{ "SGN", "FUNCTION", 
"_\bS_\bG_\bN _\b(_\bx_\b)", 
"          Sign of x (+1 for positive, -1 for negative and 0 for zero)\n"
"\n"
"9.1 Unit convertion\n"
"\n"
},

{ "DEG", "FUNCTION", 
"_\bD_\bE_\bG _\b(_\bx_\b)", 
"          Radians to degrees\n"
"\n"
},

{ "RAD", "FUNCTION", 
"_\bR_\bA_\bD _\b(_\bx_\b)", 
"          Degrees to radians\n"
"\n"
"9.2 Round\n"
"\n"
},

{ "INT", "FUNCTION", 
"_\bI_\bN_\bT _\b(_\bx_\b)", 
"          Rounds x downwards to the nearest integer\n"
"\n"
},

{ "FIX", "FUNCTION", 
"_\bF_\bI_\bX _\b(_\bx_\b)", 
"          Rounds x upwards to the nearest integer\n"
"\n"
},

{ "FLOOR", "FUNCTION", 
"_\bF_\bL_\bO_\bO_\bR _\b(_\bx_\b)", 
"          Largest integer value not greater than x\n"
"\n"
},

{ "CEIL", "FUNCTION", 
"_\bC_\bE_\bI_\bL _\b(_\bx_\b)", 
"          Smallest integral value not less than x\n"
"\n"
},

{ "FRAC", "FUNCTION", 
"_\bF_\bR_\bA_\bC _\b(_\bx_\b)", 
"          Fractional part of x\n"
"\n"
},

{ "ROUND", "FUNCTION", 
"_\bR_\bO_\bU_\bN_\bD _\b(_\bx_\b _\b[_\b,_\b _\bd_\be_\bc_\bs_\b]_\b)", 
"          Rounds the x to the nearest integer or number with \'decs\'\n"
"          decimal digits.\n"
"\n"
"9.3 Trigonometry\n"
"\n"
},

{ "COS", "FUNCTION", 
"_\bC_\bO_\bS _\b(_\bx_\b)", 
"          Cosine\n"
"\n"
},

{ "SIN", "FUNCTION", 
"_\bS_\bI_\bN _\b(_\bx_\b)", 
"          Sine\n"
"\n"
},

{ "TAN", "FUNCTION", 
"_\bT_\bA_\bN _\b(_\bx_\b)", 
"          Tangent\n"
"\n"
},

{ "ACOS", "FUNCTION", 
"_\bA_\bC_\bO_\bS _\b(_\bx_\b)", 
"          Inverse cosine\n"
"\n"
},

{ "ASIN", "FUNCTION", 
"_\bA_\bS_\bI_\bN _\b(_\bx_\b)", 
"          Inverse sine\n"
"\n"
},

{ "ATAN", "FUNCTION", 
"_\bA_\bT_\bA_\bN _\b(_\bx_\b)", 
"\n"
},

{ "ATN", "FUNCTION", 
"_\bA_\bT_\bN _\b(_\bx_\b)", 
"          Inverse tangent\n"
"\n"
},

{ "ATAN2", "FUNCTION", 
"_\bA_\bT_\bA_\bN_\b2 _\b(_\bx_\b,_\b _\by_\b)", 
"          Inverse tangent (x,y)\n"
"\n"
},

{ "COSH", "FUNCTION", 
"_\bC_\bO_\bS_\bH _\b(_\bx_\b)", 
"\n"
},

{ "SINH", "FUNCTION", 
"_\bS_\bI_\bN_\bH _\b(_\bx_\b)", 
"\n"
},

{ "TANH", "FUNCTION", 
"_\bT_\bA_\bN_\bH _\b(_\bx_\b)", 
"\n"
},

{ "ACOSH", "FUNCTION", 
"_\bA_\bC_\bO_\bS_\bH _\b(_\bx_\b)", 
"\n"
},

{ "ASINH", "FUNCTION", 
"_\bA_\bS_\bI_\bN_\bH _\b(_\bx_\b)", 
"\n"
},

{ "ATANH", "FUNCTION", 
"_\bA_\bT_\bA_\bN_\bH _\b(_\bx_\b)", 
"\n"
},

{ "SEC", "FUNCTION", 
"_\bS_\bE_\bC _\b(_\bx_\b)", 
"          Secant\n"
"\n"
},

{ "CSC", "FUNCTION", 
"_\bC_\bS_\bC _\b(_\bx_\b)", 
"          Cosecant\n"
"\n"
},

{ "COT", "FUNCTION", 
"_\bC_\bO_\bT _\b(_\bx_\b)", 
"          Cotangent\n"
"\n"
},

{ "ASEC", "FUNCTION", 
"_\bA_\bS_\bE_\bC _\b(_\bx_\b)", 
"          Inverse secant\n"
"\n"
},

{ "ACSC", "FUNCTION", 
"_\bA_\bC_\bS_\bC _\b(_\bx_\b)", 
"          Inverse cosecant\n"
"\n"
},

{ "ACOT", "FUNCTION", 
"_\bA_\bC_\bO_\bT _\b(_\bx_\b)", 
"          Inverse cotangent\n"
"\n"
},

{ "SECH", "FUNCTION", 
"_\bS_\bE_\bC_\bH _\b(_\bx_\b)", 
"\n"
},

{ "CSCH", "FUNCTION", 
"_\bC_\bS_\bC_\bH _\b(_\bx_\b)", 
"\n"
},

{ "COTH", "FUNCTION", 
"_\bC_\bO_\bT_\bH _\b(_\bx_\b)", 
"\n"
},

{ "ASECH", "FUNCTION", 
"_\bA_\bS_\bE_\bC_\bH _\b(_\bx_\b)", 
"\n"
},

{ "ACSCH", "FUNCTION", 
"_\bA_\bC_\bS_\bC_\bH _\b(_\bx_\b)", 
"\n"
},

{ "ACOTH", "FUNCTION", 
"_\bA_\bC_\bO_\bT_\bH _\b(_\bx_\b)", 
"\n"
"9.4 Logarithms\n"
"\n"
},

{ "EXP", "FUNCTION", 
"_\bE_\bX_\bP _\b(_\bx_\b)", 
"          Returns the value of e raised to the power of x.\n"
"\n"
},

{ "LOG", "FUNCTION", 
"_\bL_\bO_\bG _\b(_\bx_\b)", 
"          Returns the natural logarithm of x.\n"
"\n"
},

{ "LOG10", "FUNCTION", 
"_\bL_\bO_\bG_\b1_\b0 _\b(_\bx_\b)", 
"          Returns the base-10 logarithm of x.\n"
"\n"
"9.5 Statistics\n"
"\n"
"   Sample standard deviation: SQR(STATSPREADS(array)) Population\n"
"   standard deviation: SQR(STATSPREADP(array))\n"
"\n"
},

{ "SUM", "FUNCTION", 
"_\bS_\bU_\bM _\b(_\b._\b._\b._\b)", 
"          Sum of value\n"
"\n"
},

{ "SUMSQ", "FUNCTION", 
"_\bS_\bU_\bM_\bS_\bQ _\b(_\b._\b._\b._\b)", 
"          Sum of square value\n"
"\n"
},

{ "STATMEAN", "FUNCTION", 
"_\bS_\bT_\bA_\bT_\bM_\bE_\bA_\bN _\b(_\b._\b._\b._\b)", 
"          Arithmetical mean\n"
"\n"
},

{ "STATMEANDEV", "FUNCTION", 
"_\bS_\bT_\bA_\bT_\bM_\bE_\bA_\bN_\bD_\bE_\bV _\b(_\b._\b._\b._\b)", 
"          Mean deviation\n"
"\n"
},

{ "STATSPREADS", "FUNCTION", 
"_\bS_\bT_\bA_\bT_\bS_\bP_\bR_\bE_\bA_\bD_\bS _\b(_\b._\b._\b._\b)", 
"          Sample spread\n"
"\n"
},

{ "STATSPREADP", "FUNCTION", 
"_\bS_\bT_\bA_\bT_\bS_\bP_\bR_\bE_\bA_\bD_\bP _\b(_\b._\b._\b._\b)", 
"          Population spread\n"
"\n"
"9.6 Equations\n"
"\n"
},

{ "LINEQN", "FUNCTION", 
"_\bL_\bI_\bN_\bE_\bQ_\bN _\b(_\ba_\b,_\b _\bb_\b _\b[_\b,_\b _\bt_\bo_\bl_\be_\br_\b]_\b)", 
"          Returns an array with the values of the unknowns. This\n"
"          function solves equations by using the Gauss-Jordan method.\n"
"\n"
"        b\n"
"               equations\n"
"        b\n"
"               results\n"
"        toler\n"
"               tolerance number. (the absolute value of the lowest\n"
"               acceptable number) default = 0 = none...\n"
"               |x| <= toler : x = 0\n"
"\n"
"          _\b* _\bT_\bh_\be_\b _\br_\be_\bs_\bu_\bl_\bt_\b _\bi_\bs_\b _\ba_\b _\bm_\ba_\bt_\br_\bi_\bx_\b _\bN_\bx_\b1_\b._\b _\bF_\bo_\br_\b _\bt_\bh_\be_\b _\bS_\bB_\b _\bt_\bh_\ba_\bt_\b _\ba_\br_\br_\ba_\by_\b _\bi_\bs\n"
"          _\bt_\bw_\bo_\b-_\bd_\bi_\bm_\be_\bn_\bs_\bi_\bo_\bn_\b _\ba_\br_\br_\ba_\by_\b.\n"
"\n"
},

{ "INVERSE", "FUNCTION", 
"_\bI_\bN_\bV_\bE_\bR_\bS_\bE _\b(_\bA_\b)", 
"          returns the inverse matrix of A.\n"
"\n"
},

{ "DETERM", "FUNCTION", 
"_\bD_\bE_\bT_\bE_\bR_\bM _\b(_\bA_\b[_\b,_\b _\bt_\bo_\bl_\be_\br_\b]_\b)", 
"          Determinant of A\n"
"\n"
"          toler = tolerance number (the absolute value of the lowest\n"
"          acceptable number) default = 0 = none\n"
"\n"
"          |x| <= toler : x = 0\n"
"\n"
},

{ "SEGCOS", "FUNCTION", 
"_\bS_\bE_\bG_\bC_\bO_\bS _\b(_\bA_\bx_\b,_\bA_\by_\b,_\bB_\bx_\b,_\bB_\by_\b,_\bC_\bx_\b,_\bC_\by_\b,_\bD_\bx_\b,_\bD_\by_\b)", 
"\n"
},

{ "SEGSIN", "FUNCTION", 
"_\bS_\bE_\bG_\bS_\bI_\bN _\b(_\bA_\bx_\b,_\bA_\by_\b,_\bB_\bx_\b,_\bB_\by_\b,_\bC_\bx_\b,_\bC_\by_\b,_\bD_\bx_\b,_\bD_\by_\b)", 
"          Sinus or cosine of 2 line segments (A->B, C->D).\n"
"\n"
},

{ "PTDISTSEG", "FUNCTION", 
"_\bP_\bT_\bD_\bI_\bS_\bT_\bS_\bE_\bG _\b(_\bB_\bx_\b,_\bB_\by_\b,_\bC_\bx_\b,_\bC_\by_\b,_\bA_\bx_\b,_\bA_\by_\b)", 
"          Distance of point A from line segment B-C\n"
"\n"
},

{ "PTDISTLN", "FUNCTION", 
"_\bP_\bT_\bD_\bI_\bS_\bT_\bL_\bN _\b(_\bB_\bx_\b,_\bB_\by_\b,_\bC_\bx_\b,_\bC_\by_\b,_\bA_\bx_\b,_\bA_\by_\b)", 
"          Distance of point A from line B, C\n"
"\n"
},

{ "PTSIGN", "FUNCTION", 
"_\bP_\bT_\bS_\bI_\bG_\bN _\b(_\bA_\bx_\b,_\bA_\by_\b,_\bB_\bx_\b,_\bB_\by_\b,_\bQ_\bx_\b,_\bQ_\by_\b)", 
"          The sign of point Q from line segment A->B\n"
"\n"
},

{ "SEGLEN", "FUNCTION", 
"_\bS_\bE_\bG_\bL_\bE_\bN _\b(_\bA_\bx_\b,_\bA_\by_\b,_\bB_\bx_\b,_\bB_\by_\b)", 
"          Length of line segment\n"
"\n"
},

{ "POLYAREA", "FUNCTION", 
"_\bP_\bO_\bL_\bY_\bA_\bR_\bE_\bA _\b(_\bp_\bo_\bl_\by_\b)", 
"          Returns the area of the polyline poly.\n"
"\n"
},

{ "SPC", "FUNCTION", 
"_\bS_\bP_\bC _\b(_\bn_\b)", 
"\n"
},

{ "SPACE", "FUNCTION", 
"_\bS_\bP_\bA_\bC_\bE _\b(_\bn_\b)", 
"          returns a string of \'n\' spaces\n"
"\n"
},

{ "BIN", "FUNCTION", 
"_\bB_\bI_\bN _\b(_\bx_\b)", 
"          Returns the binary value of x as string.\n"
"\n"
},

{ "OCT", "FUNCTION", 
"_\bO_\bC_\bT _\b(_\bx_\b)", 
"          Returns the octal value of x as string.\n"
"\n"
},

{ "HEX", "FUNCTION", 
"_\bH_\bE_\bX _\b(_\bx_\b)", 
"          Returns the hexadecimal value of x as string.\n"
"\n"
},

{ "VAL", "FUNCTION", 
"_\bV_\bA_\bL _\b(_\bs_\b)", 
"          Returns the numeric value of string s.\n"
"\n"
},

{ "STR", "FUNCTION", 
"_\bS_\bT_\bR _\b(_\bx_\b)", 
"          Returns the string value of x.\n"
"\n"
},

{ "CBS", "FUNCTION", 
"_\bC_\bB_\bS _\b(_\bs_\b)", 
"\n"
},

{ "BCS", "FUNCTION", 
"_\bB_\bC_\bS _\b(_\bs_\b)", 
"          CBS() - converts (C)-style strings to (B)ASIC-style (S)trings\n"
"\n"
"          BCS() - converts (B)ASIC-style strings to (C)-style (S)trings\n"
"\n"
"          C-Style string means strings with \\ codes\n"
"\n"
"          _\b* _\bO_\bn_\b _\bC_\bB_\bS_\b(_\b)_\b _\bw_\be_\b _\bc_\ba_\bn_\bn_\bo_\bt_\b _\bu_\bs_\be_\b _\bt_\bh_\be_\b _\b\\_\b\"_\b _\bc_\bh_\ba_\br_\ba_\bc_\bt_\be_\br_\b _\bb_\bu_\bt_\b _\bw_\be_\b _\bc_\ba_\bn_\b _\br_\be_\bp_\bl_\ba_\bc_\be\n"
"          _\bi_\bt_\b _\bw_\bi_\bt_\bh_\b _\b\\_\bx_\b2_\b2_\b _\bo_\br_\b _\b\\_\b0_\b4_\b2_\b.\n"
"\n"
},

{ "ASC", "FUNCTION", 
"_\bA_\bS_\bC _\b(_\bs_\b)", 
"          Returns the ASCII code of first character of the string s.\n"
"\n"
},

{ "CHR", "FUNCTION", 
"_\bC_\bH_\bR _\b(_\bx_\b)", 
"          Returns one-char string of character with ASCII code x.\n"
"\n"
},

{ "LOWER", "FUNCTION", 
"_\bL_\bO_\bW_\bE_\bR _\b(_\bs_\b)", 
"\n"
},

{ "LCASE", "FUNCTION", 
"_\bL_\bC_\bA_\bS_\bE _\b(_\bs_\b)", 
"\n"
},

{ "UPPER", "FUNCTION", 
"_\bU_\bP_\bP_\bE_\bR _\b(_\bs_\b)", 
"\n"
},

{ "UCASE", "FUNCTION", 
"_\bU_\bC_\bA_\bS_\bE _\b(_\bs_\b)", 
"          Converts the string s to lower/upper case\n"
"\n"
"? LOWER(\"Hi\"):REM hi\n"
"? UPPER(\"Hi\"):REM HI\n"
"\n"
},

{ "LTRIM", "FUNCTION", 
"_\bL_\bT_\bR_\bI_\bM _\b(_\bs_\b)", 
"          Removes leading white-spaces from string s\n"
"\n"
"? LEN(LTRIM(\"  Hi\")):REM 2\n"
"\n"
},

{ "RTRIM", "FUNCTION", 
"_\bR_\bT_\bR_\bI_\bM _\b(_\bs_\b)", 
"          Removes trailing white-spaces from string s\n"
"\n"
},

{ "TRIM", "FUNCTION", 
"_\bT_\bR_\bI_\bM _\b(_\bs_\b)", 
"          Removes leading and trailing white-spaces from string s. TRIM\n"
"          is equal to LTRIM(RTRIM(s))\n"
"\n"
},

{ "SQUEEZE", "FUNCTION", 
"_\bS_\bQ_\bU_\bE_\bE_\bZ_\bE _\b(_\bs_\b)", 
"          Removes the leading/trailing and duplicated white-spaces\n"
"\n"
"? \"[\"; SQUEEZE(\" Hi  there \"); \"]\"\n"
"\' Result: [Hi there]\n"
"\n"
},

{ "ENCLOSE", "FUNCTION", 
"_\bE_\bN_\bC_\bL_\bO_\bS_\bE _\b(_\bs_\bt_\br_\b[_\b,_\b _\bp_\ba_\bi_\br_\b]_\b)", 
"          Encloses a string. The default pair is \"\"\n"
"\n"
"? enclose(\"abc\", \"()\")\n"
"\' Result: (abc)\n"
"\n"
},

{ "DISCLOSE", "FUNCTION", 
"_\bD_\bI_\bS_\bC_\bL_\bO_\bS_\bE _\b(_\bs_\bt_\br_\b[_\b,_\b _\bp_\ba_\bi_\br_\bs_\b _\b[_\b,_\b _\bi_\bg_\bn_\bo_\br_\be_\b-_\bp_\ba_\bi_\br_\bs_\b]_\b]_\b)", 
"          Discloses a string.\n"
"\n"
"          Default pairs and ignore pairs\n"
"\n"
"First\n"
"non white-space\n"
"character           Check   Ignore\n"
"--------------------------------------------\n"
"\"                   \"\"      \'\'\n"
"\'                   \'\'      \"\"\n"
"(                   ()      \"\"\'\'\n"
"[                   []      \"\"\'\'\n"
"{                   {}      \"\"\'\'\n"
"<                   <>      \"\"\'\'\n"
"\n"
"Otherwise:\n"
"\"                   \"\"      \'\'\n"
"\n"
"s = \"abc (abc)\"\n"
"? s; tab(26); disclose(s, \"()\")\n"
"\' prints abc\n"
"s = \"abc (a(bc))\"\n"
"? s; tab(26); disclose(s, \"()\"); tab(40); disclose(disclose(s, \"()\"), \"()\")\n"
"\' prints a(bc), bc\n"
"s = \"abc (a=\'(bc)\')\"\n"
"? s; tab(26); disclose(s, \"()\", \"\'\'\"); tab(40); &\n"
"    disclose(disclose(s, \"()\", \"\'\'\"), \"()\", \"\'\'\")\n"
"\' prints a=\'(bc)\', nothing\n"
"\n"
},

{ "LEFT", "FUNCTION", 
"_\bL_\bE_\bF_\bT _\b(_\bs_\b _\b[_\b,_\bn_\b]_\b)", 
"\n"
},

{ "RIGHT", "FUNCTION", 
"_\bR_\bI_\bG_\bH_\bT _\b(_\bs_\b[_\b,_\bn_\b]_\b)", 
"          Returns the n number of leftmost/rightmost chars of string s\n"
"          If n is not specified, the SB uses 1\n"
"\n"
},

{ "LEFTOF", "FUNCTION", 
"_\bL_\bE_\bF_\bT_\bO_\bF _\b(_\bs_\b1_\b,_\b _\bs_\b2_\b)", 
"\n"
},

{ "RIGHTOF", "FUNCTION", 
"_\bR_\bI_\bG_\bH_\bT_\bO_\bF _\b(_\bs_\b1_\b,_\b _\bs_\b2_\b)", 
"          Returns the left/right part of s1 at the position of the first\n"
"          occurrence of the string s2 into string s1\n"
"\n"
"          _\b* _\bs_\b2_\b _\bd_\bo_\be_\bs_\b _\bn_\bo_\bt_\b _\bi_\bn_\bc_\bl_\bu_\bd_\be_\bd_\b _\bo_\bn_\b _\bn_\be_\bw_\b _\bs_\bt_\br_\bi_\bn_\bg_\b.\n"
"\n"
},

{ "LEFTOFLAST", "FUNCTION", 
"_\bL_\bE_\bF_\bT_\bO_\bF_\bL_\bA_\bS_\bT _\b(_\bs_\b1_\b,_\b _\bs_\b2_\b)", 
"\n"
},

{ "RIGHTOFLAST", "FUNCTION", 
"_\bR_\bI_\bG_\bH_\bT_\bO_\bF_\bL_\bA_\bS_\bT _\b(_\bs_\b1_\b,_\b _\bs_\b2_\b)", 
"          Returns the left/right part of s1 at the position of the last\n"
"          occurrence of the string s2 into string s1\n"
"\n"
"          _\b* _\bs_\b2_\b _\bd_\bo_\be_\bs_\b _\bn_\bo_\bt_\b _\bi_\bn_\bc_\bl_\bu_\bd_\be_\bd_\b _\bo_\bn_\b _\bn_\be_\bw_\b _\bs_\bt_\br_\bi_\bn_\bg_\b.\n"
"\n"
},

{ "MID", "FUNCTION", 
"_\bM_\bI_\bD _\b(_\bs_\b,_\b _\bs_\bt_\ba_\br_\bt_\b _\b[_\b,_\bl_\be_\bn_\bg_\bt_\bh_\b]_\b)", 
"          Returns the part (length) of the string s starting from\n"
"          \'start\' position\n"
"\n"
"          If the \'length\' parameter is omitted, MID returns the whole\n"
"          string from the position \'start\'.\n"
"\n"
},

{ "INSTR", "FUNCTION", 
"_\bI_\bN_\bS_\bT_\bR _\b(_\b[_\bs_\bt_\ba_\br_\bt_\b,_\b]_\b _\bs_\b1_\b,_\b _\bs_\b2_\b)", 
"          Returns the position of the first occurrence of the string s2\n"
"          into string s1 (starting from the position \'start\')\n"
"\n"
"          If there is no match, INSTR returns 0\n"
"\n"
},

{ "RINSTR", "FUNCTION", 
"_\bR_\bI_\bN_\bS_\bT_\bR _\b(_\b[_\bs_\bt_\ba_\br_\bt_\b,_\b]_\b _\bs_\b1_\b,_\b _\bs_\b2_\b)", 
"          Returns the position of the last occurrence of the string s2\n"
"          into string s1 (starting from the position \'start\')\n"
"\n"
"          If there is no match, RINSTR returns 0\n"
"\n"
},

{ "REPLACE", "FUNCTION", 
"_\bR_\bE_\bP_\bL_\bA_\bC_\bE _\b(_\bs_\bo_\bu_\br_\bc_\be_\b,_\b _\bp_\bo_\bs_\b,_\b _\bs_\bt_\br_\b _\b[_\b,_\b _\bl_\be_\bn_\b]_\b)", 
"          Writes the \'str\' into \'pos\' of \'source\' and returns the new\n"
"          string.\n"
"\n"
"          This function replaces only \'len\' characters. The default\n"
"          value of \'len\' is the length of \'str\'.\n"
"\n"
"s=\"123456\"\n"
"...\n"
"\' Cut\n"
"? replace(s,3,\"\",len(s))\n"
"...\n"
"\' Replace\n"
"? replace(s,2,\"bcd\")\n"
"...\n"
"\' Insert\n"
"? replace(s,3,\"cde\",0)\n"
"...\n"
"\' Replace & insert\n"
"? replace(s,2,\"RRI\",2)\n"
"\n"
},

{ "TRANSLATE", "FUNCTION", 
"_\bT_\bR_\bA_\bN_\bS_\bL_\bA_\bT_\bE _\b(_\bs_\bo_\bu_\br_\bc_\be_\b,_\b _\bw_\bh_\ba_\bt_\b _\b[_\b,_\b _\bw_\bi_\bt_\bh_\b]_\b)", 
"          Translates all occurrences of the string \'what\' found in the\n"
"          \'source\' with the string \'with\' and returns the new string.\n"
"\n"
"? Translate(\"Hello world\", \"o\", \"O\")\n"
"\' displays: HellO wOrld\n"
"\n"
},

{ "CHOP", "FUNCTION", 
"_\bC_\bH_\bO_\bP _\b(_\bs_\bo_\bu_\br_\bc_\be_\b)", 
"          Chops off the last character of the string \'source\' and\n"
"          returns the result.\n"
"\n"
},

{ "STRING", "FUNCTION", 
"_\bS_\bT_\bR_\bI_\bN_\bG _\b(_\bl_\be_\bn_\b,_\b _\b{_\ba_\bs_\bc_\bi_\bi_\b|_\bs_\bt_\br_\b}_\b)", 
"          Returns a string containing \'len\' times of string \'str\' or the\n"
"          character \'ascii\'.\n"
"\n"
},

{ "FORMAT", "FUNCTION", 
"_\bF_\bO_\bR_\bM_\bA_\bT _\b(_\bf_\bo_\br_\bm_\ba_\bt_\b,_\b _\bv_\ba_\bl_\b)", 
"          Returns a formated string.\n"
"\n"
"          Numbers:\n"
"\n"
"        #\n"
"               Digit or space\n"
"        0\n"
"               Digit or zero\n"
"        ^\n"
"               Stores a number in exponential format. Unlike QB\'s USING\n"
"               format this is a place-holder like the #.\n"
"        .\n"
"               The position of the decimal point.\n"
"        ,\n"
"               Separator.\n"
"        -\n"
"               Stores minus if the number is negative.\n"
"        +\n"
"               Stores the sign of the number.\n"
"\n"
"          Strings:\n"
"\n"
"        &\n"
"               Stores a string expression without reformatting it.\n"
"        !\n"
"               Stores only the first character of a string expression.\n"
"        \\ \\\n"
"               Stores only the first n + 2 characters of a string\n"
"               expression, where n is the number of spaces between the\n"
"               two backslashes. Unlike QB, there can be literals inside\n"
"               the \\ \\. These literals are inserted in the final string.\n"
"\n"
"? FORMAT(\"#,##0\", 1920.6) : REM prints 1,921\n"
"? FORMAT(\"\\  - \\\", \"abcde\") : REM prints \"abc-de\"\n"
"\n"
},

{ "CAT", "FUNCTION", 
"_\bC_\bA_\bT _\b(_\bx_\b)", 
"          Returns a console codes\n"
"\n"
"          0  reset\n"
"          1  bold on\n"
"          -1 bold off\n"
"          2  underline on\n"
"          -2 underline off\n"
"          3  reverse on\n"
"          -3 reverse off\n"
"\n"
"          PalmOS only:\n"
"\n"
"          80..87 select system font\n"
"          90..93 select custom font\n"
"\n"
"          Example:\n"
"\n"
"? cat(1);\"Bold\";cat(0)\n"
"\n"
},

{ "INKEY", "FUNCTION", 
"_\bI_\bN_\bK_\bE_\bY", 
"          This function returns the last key-code in keyboard buffer, or\n"
"          an empty string if there are no keys.\n"
"\n"
"          Special key-codes like the function-keys (PC) or the\n"
"          hardware-buttons (PalmOS) are returned as 2-byte string.\n"
"\n"
"          Example:\n"
"\n"
"k=INKEY\n"
"IF LEN(k)\n"
"  IF LEN(k)=2\n"
"    ? \"H/W #\"+ASC(RIGHT(k,1))\n"
"  ELSE\n"
"    ? k; \" \"; ASC(k)\n"
"  FI\n"
"ELSE\n"
"  ? \"keyboard buffer is empty\"\n"
"FI\n"
"\n"
},

{ "MYSQL.CONNECT", "FUNCTION", 
"_\bM_\bY_\bS_\bQ_\bL_\b._\bC_\bO_\bN_\bN_\bE_\bC_\bT _\b(_\bh_\bo_\bs_\bt_\b,_\b _\bd_\ba_\bt_\ba_\bb_\ba_\bs_\be_\b,_\b _\bu_\bs_\be_\br_\b,_\b _\b[_\bp_\ba_\bs_\bs_\bw_\bo_\br_\bd_\b]_\b)", 
"          Connects/reconnects to the server\n"
"\n"
},

{ "MYSQL.QUERY", "FUNCTION", 
"_\bM_\bY_\bS_\bQ_\bL_\b._\bQ_\bU_\bE_\bR_\bY _\b(_\bh_\ba_\bn_\bd_\bl_\be_\b,_\b _\bs_\bq_\bl_\bs_\bt_\br_\b)", 
"          Send command to mysql server\n"
"\n"
},

{ "MYSQL.DBS", "FUNCTION", 
"_\bM_\bY_\bS_\bQ_\bL_\b._\bD_\bB_\bS _\b(_\bh_\ba_\bn_\bd_\bl_\be_\b)", 
"          Get a list of the databases\n"
"\n"
},

{ "MYSQL.TABLES", "FUNCTION", 
"_\bM_\bY_\bS_\bQ_\bL_\b._\bT_\bA_\bB_\bL_\bE_\bS _\b(_\bh_\ba_\bn_\bd_\bl_\be_\b)", 
"          Get a list of the tables\n"
"\n"
},

{ "MYSQL.FIELDS", "FUNCTION", 
"_\bM_\bY_\bS_\bQ_\bL_\b._\bF_\bI_\bE_\bL_\bD_\bS _\b(_\bh_\ba_\bn_\bd_\bl_\be_\b,_\b _\bt_\ba_\bb_\bl_\be_\b)", 
"          Get a list of the fields of a table\n"
"\n"
},

{ NULL, NULL, NULL, NULL } };
