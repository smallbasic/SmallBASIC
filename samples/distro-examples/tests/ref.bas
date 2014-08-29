' reference variable tests

a = "cat"
b = @a

print "cat=", b
a = "dog"
print "dog=", b
print "3=", len(b)
print "1=", not empty(b)
print "a=b", iFF(a==b, "a=b", "a<>b")

b = "goodbye a"
print "a<>b", iFF(a==b, "a=b", "a<>b")

dim rooms
sub addRoom(byref room)
  rooms << @room
end

dim kitchen,hall,toilet
kitchen.name= "kitchen"
hall.name = "hall"
toilet.name ="toilet"

addRoom(kitchen)
addRoom(hall)

print rooms(0)
print rooms(1)

kitchen.name = "Kitchen"
kitchen.fridge = "empty"
print rooms(0)

insert rooms, 0, @toilet
toilet.occupied = true

print rooms(0)
print rooms(1)
print rooms(2)

