foo = array("{}")
key="blah"
foo(key) = "something"
foo("other") = 123
foo(100) = "cats"

? foo(key)
? foo("other")
? foo

REM added 5/2/2017

func create()
  func pop(f)
    if (self.data[0] != 123) then
      throw "invalid self data"
    fi
    pop = "pop-result"
  end

  sub push()
    if (self.data[0] != 123) then
      throw "invalid self data"
    fi
  end

  local f
  f.pop = @pop
  f.push = @push
  f.data = [123]
  create = f
end

f = create()
f.push()
n = f.pop(1)
if (n != "pop-result") then
  throw "invalid pop result"
fi

