#!/usr/bin/sbasic -q
'	convert ref.txt to C/C++ text
'
'	The section symbol is the x
'	.....
'	>x>>>
'	.....
'

' return the filename
func getfn(c)
local i

for i=0 to len(ti)-1
	if c=ti(i)(0)
		getfn=ti(i)(1)
		exit func
	fi
next
getfn="garbage.txt"
end

' reformat text to 72 chars
sub reformat(byref A)
local ret, txt
local wlist, w
local ntxt

for txt in A
	if left(txt,4) <> "#def"
		if len(txt)>4
			if mid(txt,3,2)=">>"
				txt=mid(txt,4)
			fi
		fi

		if left(txt,4) = "____" 
			if len(txt) > 30
				ret << string(71, "_")
			else
				ret << txt
			fi
		else
			ret << txt	' var-font
		fi
	elif left(txt, 7) = "#defkey"
		split squeeze(mid(txt,8)), " ", kw_l
		kw_buf = ""
		if len(kw_l)
			for kw_e in kw_l
				if len(kw_e) do kw_buf += "${key:"+lower(kw_e)+"}"
			next
		fi
		if len(kw_buf) do ret << kw_buf
	fi
next

A=ret
end

'
SUB CREATE_C_SOURCE(byref csrc,b)
local	rl

csrc = "char *help_page_"+pcat+"="+chr(10)
for rl in b
	if "\" in rl then rl = translate(rl, "\", "\\")
	if chr(34) in rl then rl = translate(rl, chr(34), "\"+chr(34))
	csrc += chr(34)+rl+"\n"+chr(34)+chr(10)
next
csrc += ";"
END

' section <-> filename
ti=[ &
["a","x-intro.c"], &
["b","x-commands.c"], &
["c","x-system.c"], &
["d","x-graphics.c"], &
["e","x-miscellaneous.c"], &
["f","x-filesystem.c"], &
["g","x-mathematics.c"], &
["h","x-strings.c"], &
["i","x-console.c"] &
]

st=ticks
TLOAD "../../doc/ref.txt", A
fn="garbage.txt"
pcat="-"
for l in A
	if len(l) > 3 
		c = mid(l,2,1)
		if c != ">"
			if left(l,1)=">" and mid(l,3,1) = ">"
				? getfn(c)
				if c="-"
					ignore=1
				elif fn != getfn(c)
					' change file
					ignore=0
					REFORMAT b
					CREATE_C_SOURCE csrc, b
					TSAVE fn, csrc
					fn = getfn(c)
					pcat=c
					erase b, csrc
				fi
			fi
		fi
	fi

	if !ignore then b << l
next
REFORMAT b
CREATE_C_SOURCE csrc, b
TSAVE fn, csrc
se=ticks
? (se-st) / tickspersec


