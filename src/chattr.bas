#!/usr/bin/sbasic

sub chattr(f)
local attr, fl, x

attr=access(f)
if isdir(f)
	chdir f
	fl=files("*")
	for x in fl do chattr x
	chdir ".."
else
	if attr & 0o100
		chmod f, 0o777
	else
		chmod f, 0o666
	fi
fi
end


' main
fl=files("*")
for f in fl do chattr f
