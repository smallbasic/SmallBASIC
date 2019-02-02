rem reference variables not supported
a = "dog"
b = @a
a = "cat"
print "cat=", a
print "dog=", b

rem complex pseudo class method references
func C
  func f(j)
    return j
  end
  local r = {}
  r.f = @f
  return r
end
func Q
  local r = {}
  r.c = C()
  return r
end
c.m = Q()
j = [1,2,3]
for n in c.m.c.f(j)
  print n
next n
