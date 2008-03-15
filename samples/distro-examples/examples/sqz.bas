' squeeze

tload "../README", a
for x in a
	? "[";x;"]"
	? "[";squeeze(x);"]"
	? "..."
	pause
next


