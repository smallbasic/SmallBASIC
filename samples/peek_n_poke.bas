'
'	Low-level commands
'
'	MALLOC(size)/BALLOC(size)
'
'	PEEK[{16|32}](addr)
'	POKE[{16|32}] addr, val
'
'	ERASE var	' free(var)
'
'	BCOPY source_addr, dest_addr, size	' memcpy
'
m=malloc(20)
? "Address = 0x";hex(vadr(m))
poke vadr(m),0x22
? peek(vadr(m))
erase m



