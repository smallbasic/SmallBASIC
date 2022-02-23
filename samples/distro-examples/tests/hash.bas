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

m = array("{a:10,b:{c:20}}")
ma = {
  a:10,
  b: {
    c:20
  }
}
if (m != ma) then
  throw "array error"
endif

### fix bug in bc.c, fix regression in var_eval.c
f = {"refresh":func,"doEvents":func,"focus":6,"inputs":[{"backgroundColor":0,"color":3,"label":"[<<]","x":0,"ID":0,"value":"__bn_close__","y":0,"type":"link"},{"backgroundColor":0,"color":3,"label":"[View]","x":-3,"ID":1,"value":"__bn_view__","y":0,"type":"link"},{"backgroundColor":0,"color":3,"label":"[Rename]","x":-3,"ID":2,"value":"__bn_rename__","y":0,"type":"link"},{"backgroundColor":0,"color":3,"label":"[New]","x":-3,"ID":3,"value":"__bn_new__","y":0,"type":"link"},{"backgroundColor":0,"color":3,"label":"[Delete]","x":-3,"ID":4,"value":"__bn_delete__","y":0,"type":"link"},{"backgroundColor":0,"color":3,"label":"[Save-As]","x":-3,"ID":5,"value":"__bn_saveas__","y":0,"type":"link"},{"help":"Enter file name, and then click New.","y":17,"type":"text","color":"white","width":636,"ID":6,"value":"","x":0,"resizable":1},{"height":448,"value":"","y":32,"type":"list","color":2,"resizable":1,"selectedIndex":0,"help":"No .bas files in /home/chrisws/src/SmallBASIC/ide/android/assets/","width":636,"x":0,"ID":7}],"value":0,"close":func}

if (not isarray(f.inputs)) then
  throw "pre: inputs not a map"
endif

f.inputs[6].value= "123"

if (not isarray(f.inputs)) then
  throw "post: inputs not a map"
endif

'
' multi-layer access
'
dim lights(1)
shadow.vertices = []
shadow.vertices << 1
lights[0].shadows = []
lights[0].shadows << shadow
lights[0].shadows[0].vertices[0] = [1,2,3,4]

'
' parse "true" and "false" as boolean fields
'
a= array("{\"stringT\", \"true\", \"stringF\", \"false\", \"booleanT\": true, \"booleanF\": false}")
if (a.stringT <> "true") then throw "not true"
if (a.stringF <> "false") then throw "not false"
if (a.booleanT <> 1) then throw "not true"
if (a.booleanF <> 0) then throw "not false"
