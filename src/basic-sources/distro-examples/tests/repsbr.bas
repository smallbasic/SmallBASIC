#!/usr/bin/sbasic

' replaces the /sbrun with /sbasic

flist=files("*.bas")
for f in flist
	tload f, t
	if ( instr(t(0),"/sbrun") )	
		pars = rightoflast(t(0), "/sbrun")
		t(0) = "#!/usr/bin/sbasic"+pars
		tsave f, t
	fi
next


