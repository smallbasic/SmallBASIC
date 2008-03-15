'
'	SPLIT, DISCLOSE and WSPLIT examples
'

CLS
?
? ">>> ";cat(1);"SPLIT example";cat(-1)
? string(80, "_")
?

s="192.168..1"
split s,".",a
? "Source: [";s;"]"
? "Result: ";a;tab(40);"(simple)"
?

s="comp.  u n i   x  .  misc "
split s,".",a
? "Source: [";s;"]"
? "Result: ";a;tab(40);"(simple)"
?

s="comp.  u n i   x  .  misc "
split s,".",a use squeeze(x)
? "Source: [";s;"]"
? "Result: ";a;tab(40);"(using squeeze)"
?

s="a,'b,c'"
split s,",",a 
? "Source: [";s;"]"
? "Result: ";a; " len=";len(a);tab(40);"(simple)"
?

s="a,'b,c'"
split s,",",a,"''"
? "Source: [";s;"]"
? "Result: ";a;" len=";len(a);tab(40);"(using pairs)"
?

sub fspl(x)
	if isarray(x)
		for y in a
			split y," ",b,"''()" use squeeze(x)
			? b
			if len(b)>1
				fspl b
			fi
		next
	else
		split x," ",a,"''()" use squeeze(x)
		? a
		fspl a
	fi
end

s="abc ( 'xyz'   )"
fspl s

?
?
? ">>> "; cat(1);"Enclose/Disclose example";cat(-1)
? string(80, "_")
?
? "Original";tab(26);"Disclose 1";tab(40);"Disclose 2";tab(54);"Disclose 3"
? string(80, "_")
?
s="abc ( abc )"
? s;tab(26);disclose(s, "()")
s="abc ( a ( bc ) )"
? s;tab(26);disclose(s, "()");tab(40);disclose(disclose(s, "()"), "()")
s="abc (a='(bc)')"
? s;tab(26);disclose(s, "()", "''");tab(40);disclose(disclose(s, "()", "''"), "()", "''")
s="abc {(a='(bc)')}"
? s;tab(26);disclose(s, "(){}", "''");tab(40);disclose(disclose(s, "(){}", "''"), "(){}", "''")
s="abc {(a=}('(bc)')"
? s;tab(26);disclose(s, "(){}", "''"); &
	tab(40);disclose(disclose(s, "(){}", "''"), "(){}", "''"); &
	tab(54);disclose(disclose(disclose(s, "(){}", "''"), "(){}", "''"), "(){}", "''")



