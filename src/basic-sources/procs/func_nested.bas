' procedure/function test (0.9)
'
' nested functions

func z
	z = "global z() is available inside fc()"
end

func f
	func fc
		func fcfc
			fcfc="fc's fc()"
		end
		fc = fcfc
		? z
	end
	f = fc
end

func q
	func f
		f="q's f()"
	end

	q = f
end

' main
? q
? f

