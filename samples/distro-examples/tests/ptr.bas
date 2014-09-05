func foo(s)
 foo="foo: " + s
end

sub bar
 ? "in bar"
end

p = @foo
? call(@foo, "called from ptr @foo")
? call(p, "called from ptr p")

pb = @bar
call pb
call @bar

