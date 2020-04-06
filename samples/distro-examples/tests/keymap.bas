sub foo
  ? "f"
end

definekey 2, foo
definekey 1, foo
definekey 1, foo
definekey 1, foo
definekey 1, 1
definekey 1, 2
definekey 1, 3
definekey 1, 2
definekey 1, 3
definekey 2, 0
definekey 2, 0
definekey 2, 0
definekey 2, 0

definekey asc("x"), foo
definekey asc("x"), foo
definekey asc("y"), foo
definekey asc("z"), foo
definekey asc("f"), foo
definekey asc("x"), 0
definekey asc("y"), 0
definekey asc("z"), 0

if (instr(sbver, "SDL") <> 0) then
  while 1
    ? ".";
    delay 10
  wend
endif

