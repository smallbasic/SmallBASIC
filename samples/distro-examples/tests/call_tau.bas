' example: using a unit, 0.9
'

Import Tau as taz, PreDef

? "Tau's exported variable: ", taz.expvar
? "Function fooF          : ", taz.fooF("Hi")
? "Procedure fooP         : "
taz.fooP "Hi"

' reverse var-update 
taz.expvar = "message from main"
taz.print_expvar
? taz.expvar
taz.build_ta
? taz.ta
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
  taz.addRoom(foyer,x)
  jj = taz.calcRoomSize(foyer,x)
next i

sub addRoom(the_thing, d)
  print the_thing.name, d
end

addRoom(foyer,x)

if (Taz.LIGHTGRAY != rgb(200, 200, 200)) then
  print "Error importing inline assigned export variable"
endif
if (Taz.YELLOW != rgb(253, 249, 0)) then
  print "Error importing inline assigned export variable"
endif
