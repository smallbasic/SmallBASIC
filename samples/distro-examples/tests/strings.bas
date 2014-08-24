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



