func f(x)
  f=sin(x)
end

deriv 0, 1000, 1e-6, rs, er USE f(x)
if er=0 then
	? rs
else
	? "error"
endif

