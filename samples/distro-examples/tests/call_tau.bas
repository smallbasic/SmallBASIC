' example: using a unit, 0.9
'

Import Tau, PreDef

? "Tau's exported variable: ", tau.expvar
? "Function fooF          : ", tau.fooF("Hi")
? "Procedure fooP         : "
tau.fooP "Hi"

' reverse var-update 
tau.expvar = "message from main"
tau.print_expvar
? tau.expvar
tau.build_ta
? tau.ta
'tau.cerr

rem check system-variables
predef.prsys

x=PI
foyer.name= "PI"
tau.addRoom(foyer,x)

sub addRoom(the_thing, d)
  print the_thing.name, d
end

addRoom(foyer,x)
