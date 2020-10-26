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
dim foyer
foyer.name= "my name is PI"
? "test"
?  foyer("NAME")
?  foyer("name")
?  foyer.Name
?  foyer.name

? "end"

for i = 0 to 1000
  tau.addRoom(foyer,x)
  jj = tau.calcRoomSize(foyer,x)
next i

sub addRoom(the_thing, d)
  print the_thing.name, d
end

addRoom(foyer,x)

if (Tau.LIGHTGRAY != rgb(200, 200, 200)) then
  print "Error importing inline assigned export variable"
endif
if (Tau.YELLOW != rgb(253, 249, 0)) then
  print "Error importing inline assigned export variable"
endif
