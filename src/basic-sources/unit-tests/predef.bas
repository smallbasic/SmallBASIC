unit predef

export prsys

sub prsys
? cat(1);"Predefined Variables";cat(0)
? "OS VER =0x"; HEX$(osver)
? "OS NAME="; osname
? "SB VER =0x"; HEX$(sbver)
? "PI     ="; pi
? "XMAX   ="; xmax
? "YMAX   ="; ymax
? "CWD    ="; CWD
? "HOME   ="; HOME
end



