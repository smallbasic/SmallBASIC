' procedure/function test (0.9)
'
' locals

func f(x)
	local q

		func ff(y)
		local f, q

		ff=y
		q = "2... local of ff"
		? q
		end

	q = "1|3. local of f"
	? q
	o=ff(1)
	? q
	f = x^2
end

q="4... global"
z=f(2)
? q
