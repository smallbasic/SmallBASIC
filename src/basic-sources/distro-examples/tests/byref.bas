sub psa(v)
? "psa:"; v
v=-1
end

sub psb(byref v)
? "psb:"; v
if isarray(v)
	if	isarray(v(0))
		v(0)(0)=-1
	else
		v(0)=-1
	fi
else
	v=-1
fi
end

'==========================
? "* simple"
v=1
psa v:? v
psb v:? v

?
? "* array"

v << 1
v << 2
v << 3
psa v:? v
psb v:? v

?
? "* nested array #1"

v(0)=[2,3,4]
psa v:? v
psb v:? v

?
? "* nested array #2"

v(0)=[2,3,4]
psa v(0):? v(0)
psb v(0):? v(0)
? v(0)(1)

