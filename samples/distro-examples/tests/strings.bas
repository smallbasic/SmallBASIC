' test1.bas
' 04/02/2001

' GAC - Test string functionality

' Print the name of any function that fails to execute as expected.
' A clean run will only display the standard '* DONE *'

' No way to test these except print them!
? "String functions"
rem ? date$,time$

' Comparisons
if "abc" != "abc" then:? "!=":fi
if "abc" <> "abc" then:? "<>":fi
if "abc" = "ABC" then:? "=":fi
if "a" < "a" then:? "<":fi
if "b" <= "a" then:? "<=":fi
if "a" > "a" then:? ">":fi
if "a" >= "b" then:? ">=":fi

' Functions that return int or float
if val("1.2345") <> 1.2345 then:? "val":fi
if asc("1.2345") <> 49 then:? "asc":fi
if len("1.2345") <> 6 then:? "len":fi

' Functions that return a string

if chr$(48) <> "0" then:?"chr$":fi
if hex$(18) <> "12" then:? "hex$":fi
if oct$(10) <> "12" then:? "oct$":fi
if lcase$("ABC") <> "abc" then:? "lcase$":fi
if ucase$("abc") <> "ABC" then:? "ucase$":fi
if ltrim$("  x  ") <> "x  " then:? "ltrim$":fi
if ltrim$("    ") <> "" then:? "ltrim$ all spaces":fi
if rtrim$("  x  ") <> "  x" then:? "rtrim$":fi
if rtrim$("    ") <> "" then:? "rtrim$ all spaces":fi
if string$(3,"a") <> "aaa" then:? "string$(s,s)":fi
if string$(3,97) <> "aaa" then:? "string$(s,n)":fi
if left$("abc",2) <> "ab" then:? "left$":fi
if right$("abc",2) <> "bc" then:? "right$":fi
if instr("abc","b") <> 2 then:? "instr":fi
if mid$("abc",2,1) <> "b" then:? "mid$(s,n,n)":fi
if mid$("abc",2) <> "bc" then:? "mid$(s,n)":fi
if space$(3) <> "   " then:? "space$":fi
' Test $ bug
a="No$"
a$="$"
if a=a$ then ? "$ bug is alive: variables"
if chr(48) <> "0" then ? "$ bug is alive: functions"

' replace
s="1234567"
if replace$(s, 2, "bcdefg")!="1bcdefg" then ? "replace() 1 error"
if replace$(s, 2, "b")!="1b34567" then ? "replace() 2 error"
if replace$(s, 2, "b", 2)!="1b4567" then ? "replace() 3 error"
if replace$(s, 2, "b", 5)!="1b7" then ? "replace() 4 error"
if replace$(s, 2, "", len(s))!="1" then ? "replace() 5 error"
if replace$(s, 2, "bc", 0)!="1bc234567" then ? "replace() 6 error"
if replace$(s, 2, "bcI", 2)!="1bcI4567" then ? "replace() 7 error"

if leftof$(s, "23")!="1" then ? "leftof() error"
if rightof$(s, "23")!="4567" then ? "rightof() error"
if leftoflast$(s, "23")!="1" then ? "leftoflast() error"
if rightoflast$(s, "23")!="4567" then ? "rightoflast() error"

if (0 != instr("qwerty","")) then throw "instr err1"
if (0 != instr(3,"qwerty","")) then throw "instr err2"
for i = 1 to 6
  if (0 != instr(i, "qwerty", "querty")) then throw "instr err3"
next i

'==11600== Conditional jump or move depends on uninitialised value(s)
'==11600==    at 0x437B4E: v_strlen (var.c:97)
'==11600==    by 0x438C4F: v_set (var.c:448)
'==11600==    by 0x417FCA: eval_call_udf (eval.c:1160)
'==11600==    by 0x418498: eval (eval.c:1285)
' note: when first arg="" problem does not occur
Def lset(s) = Replace(" ", 1, s)
s=lset(".")

func test_str(s1,s2)
  if (s1 <> s2) then
    throw "sprint err1"
  endif
  if (len(s1) <> len(s2)) then
    throw "sprint err2"
  endif
end

sprint s1;using"aaa###bbb";123;

test_str(s1, "aaa123bbb")
test_str("   ", spc(3))

s1= """\
hello\
hello\
hello\
"""
s2="hello"
   "hello"
   "hello"
s3 = "hellohellohello"
if (s1 <> s2 || s1 <> s3) then
  ? "s1=" + s1
  ? "s2=" + s2
  ? "s3=" + s3
  throw "not equal"
endif

s4="""
this
is a
multiline
string"""

rem ------------------------------------------------
rem test case insensitive translate
if "food" != translate("foo bar", " BAR", "d", true) then
  throw "bad translate"
fi

rem ------------------------------------------------
rem fix issues reported by MGA July 2017
if (99 != VAL("99 bottles of beer")) then
  throw "VAL input error"
endif
if ("" != MID("tooShort", 10, 1)) then
  throw "MID input error"
endif
if ("" != LEFT("blah", -20)) then
  throw "LEFT input error"
fi
if ("" != RIGHT("blah", -20)) then
  throw "RIGHT input error"
endif
if (0 != INSTR(10, "test", "t")) then
  throw "INSTR input error"
endif
if (1 != INSTR(-5, "test", "t")) then
  throw "INSTR input error"
endif
if ("blahx" != REPLACE("blah", 55, "x")) then
  throw "REPLACE input error 1"
endif
if ("xlah" != REPLACE("blah", -5, "x")) then
  throw "REPLACE input error 2"
endif

rem --- test line numbers are correctly maintained when using multiline strings
if (PROGLINE != 152) then
  throw "invalid line no"
endif

rem -- invalid input
sub cmd_str1(n)
 x=chr(n)
 x=str(n)
 x=oct(n)
 x=bin(n)
 x=hex(n)
 x=lcase(n)
 x=ucase(n)
 x=ltrim(n)
 x=rtrim(n)
 x=trim(n)
 x=cat(n)
 x=tab(n)
end
cmd_str1("")
cmd_str1(0)

expect = [0,0.26179938779915,0.5235987755983,0.78539816339745,1.0471975511966,1.30899693899575,1.5707963267949,1.83259571459405,2.0943951023932,2.35619449019234,2.61799387799149,2.87979326579064,3.14159265358979,3.40339204138894,3.66519142918809,3.92699081698724,4.18879020478639,4.45058959258554,4.71238898038469,4.97418836818384,5.23598775598299,5.49778714378214,5.75958653158129,6.02138591938044,6.28318530717958]
if expect != seq(0, 2*pi, 360/15+1) then throw "SEQ error"

s="Hello\033There"
if (27 != asc(mid(s, 6, 1))) then throw "err"
