'
' This tests the shortcircuit evaluation changes
' make to ceval.c and eval.c
'

? "start of test"

func f1
 f1 = 0
end
func f2
 f2 = 1
end
func f3
 ? "**error 1**"
 f3 = xx
end
func f4
 ? "**error 2**"
 f4 = xx
end

if (f1 and f3) then
  ? "**error 3**"
fi

if not (f2 or f4) then
  ? "**error 4**"
fi

if ((f1 and f3 and f4) or f1) then
  ? "**error 5**"
fi

if not ((f1 and f3) or (f2 or f4)) then
  ? "**error 6**"
fi

? "end of test"
