#!/usr/bin/sbasic -q
rem
rem ref.txt -> .tvh
rem

#unit-path: "../../doc"
Import Ref

rem ---------------------------------
rem main: load and analyse
rem ---------------------------------

print "Creating TVision help"
st=ticks
print "Loading..."
tLoad "../../doc/ref.txt", fileLines
Ref.BuildTables fileLines, true

print "Writting..."

rem ---------------------------------
rem save & quit
rem ---------------------------------

tf = tvh_headers
for l in tf do final << l

tf = tvh_body
for l in tf do final << l

tSave "sbasic.txt", final
'inf_inst="/usr/share/info/sbasic.info"
'run "makeinfo --force --no-split sbasic.texi"
'copy "sbasic.info", inf_inst
'chmod inf_inst, 0o666

se=ticks
? (se-st) / ticksPerSec
end

rem ------------------
func tchi(x)
	if "++/--/p=" in x 
		x = "pop_p"
	elif " <<" in x 
		x = "pop_ii"
	else
		x = translate(x,  " ", "0")
		x = translate(x,  "-", "1")
		x = translate(x,  "'", "2")
		x = translate(x,  "#", "3")
		x = translate(x,  "/", "4")
		x = translate(x, "()", "5")
	fi
	x = "_" + x
	if len(x) > 32 then x = right(x,32)
	tchi = x
end

rem --------------------------
func tmn(x)
	x = translate(x, "[", "\[")
	x = translate(x, "]", "\]")
	x = translate(x, "|", "|")
	x = translate(x, "{", "\{")
	x = translate(x, "}", "\}")
	tmn = x
end

rem ------------------
func tvh_headers
local r, p

r << ".topic Index"
r << "SmallBASIC"
r << "Overview Documentation for SmallBASIC"
r << " "
for p in Ref.bookAuthors do r << p
r << " "
r << "SmallBASIC site http://smallbasic.sourceforge.net"
r << " "
r << "Copyright (C) 2000-2002 Nicholas Christopoulos"
r << " "
r << " Use letters and arrow-keys"
r << " "
for chap in Ref.Chapters
	rem r << " {Command reference:cmds}"
	r << " {"+chap(0)+":"+tchi(chap(0))+"}"
next
r << " "
tvh_headers = r
end

rem --------------------------
func tvh_body
local r

for chap in Ref.Chapters
	sections = chap(1)

	chap_name = chap(0)
	r << ".topic "+tchi(chap_name)
	r << chap_name

	rem sections - menu
	r << ""
	for sect in sections
		name = sect(0)
		if leftOf(name," ") != "Overview"
			r << " {"+name+":"+tchi(name)+"}"
		endif
	next

	rem sections - text
	for sect in sections
		name = sect(0)
		mark = sect(1)
		keywords = sect(2)
		links = sect(3)

		stx = sect(4)
		xmp = sect(5)

		strs = sect(6)

		rem section
		if leftOf(name," ") != "Overview"
			r << ".topic "+tchi(name)
			r << name
		fi

		rem syntax
		if !empty(stx)
			r << "Syntax"
			for t in stx
				r << "  "+tmn(t)
			next
		fi

		rem text
		if !empty(strs)
			if !empty(stx) do r << "" << "Description"
			for t in strs
				if	len(trim(t)) 
					if left(t, 40) <> string(40,"-")
						r << "  "+tmn(t)
					fi
				else
					r << ""
				fi
			next
			r << ""
		fi
				
		rem examples
		if !empty(xmp)
			r << "" << "Example"
			r << ""
			for t in xmp
				r << "  "+tmn(t)
			next
			r << "" 
		fi
				
		rem store links
		if !empty(links)
			r << "" << "See also"
			for cref in links
				r << "  {"+cref+":"+tchi(cref)+"}"
			next
		fi
	next
next

tvh_body = r
end


