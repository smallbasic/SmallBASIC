func f(x)
  f=sin(x)
end

ROOT 1, 5, 1000, 1e-6, rs, er USE f(x)
if er = 0
	print "Root = "; rs
else
	print "No root found"
fi
end

