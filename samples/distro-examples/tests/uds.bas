'
' This is a test for the MAP variable type
'

? "start of test"

sub passByRef(byref udsRef)
  if (udsRef.field1 != "initField1") then
     ? "error in passByRef 1"
  fi
  if (udsRef.field2 != "initField2") then
     ? "error in passByRef 2"
  fi
  if (udsRef.field3 != 3.0) then
     ? "error in passByRef 3"
  fi
  udsRef.field1 = "updatedField1"
  udsRef.field2 = "updatedField2"
  udsRef.field10 = "unused"
end

sub passByVal(udsVal)
  if (udsVal.field1 != "updatedField1") then
     ? "error in passByRef 4"
  fi
  if (udsVal.field2 != "updatedField2") then
     ? "error in passByRef 5"
  fi
  udsVal.field1 = "localUpdatedField1"
  udsVal.field2 = "localUpdatedField2"
  udsVal.field10 = "unused"
end

'test with formal+actual arguments having differing names
dim _udsRef
_udsRef.field1 = "initField1"
_udsRef.field2 = "initField2"
_udsRef.field3 = 3.0
_udsRef.field4 = "unused"
_udsRef.field5 = "unused"

passByRef _udsRef
passByVal _udsRef

'should still be equal to passByRef result
if (_udsRef.field1 != "updatedField1") then
   ? "error in passByRef 6"
fi
if (_udsRef.field2 != "updatedField2") then
   ? "error in passByRef 7"
fi

'test with formal+actual arguments with global names
dim udsRef
udsRef.field1 = "initField1"
udsRef.field2 = "initField2"
udsRef.field3 = 3.0
udsRef.field4 = "unused"
udsRef.field5 = "unused"

passByRef udsRef
passByVal udsRef

'should still be equal to passByRef result
if (udsRef.field1 != "updatedField1") then
   ? "error in passByRef 8"
fi
if (udsRef.field2 != "updatedField2") then
   ? "error in passByRef 9"
fi

'test complex structures - this is pretty cool :)
dim animal
animal.pet.cat.legs = 4
animal.pet.cat.color = "black"

my_pet = animal.pet.cat
if (my_pet.legs != 4) then
   ? "error assigning structure 10"
fi
if (my_pet.color != "black") then
   ? "error assigning structure 11"
fi 

my_pet.x = 10
if ((my_pet.x + 1) < 10) then
  ? "wrong !"
fi

a.xcat = "cat"
a.xdog = "dog"
a.xfish.big = "big"
a.xfish.small = "small"

? "a:"
? a

? "In a:"
for i in a
 ? "a." + i + "="; a(i)
next i

? "In a.xfish:"
for i in a.xfish
 ? "a.xfish." + i + "="; a.xfish(i)
next i

? len(a)
? len(a.xfish)

inner.foo="bar"
m.b << inner
s = " " + m.b(0).foo

rem ------------------------------------

dim bar.x(2), foo(2)
bar.x(0).barx = 1024
foo(0).fooY = 999
if (bar.x(0).barx <> 1024) then
  throw "Error 1"
fi

if (foo(0).fooY <> 999) then
  throw "Error 2"
fi

if (1024 <> bar.x(0).barx) then
  throw "Error 3"
fi

if (999 <> foo(0).fooY) then
  throw "Error 4"
fi

bar.x(1).barx=bar.x(0).barx+976
bar.x(1).barx/=1000
if bar.x(1).barx <> 2 then throw "Yuck, I can't read red print on black background."
