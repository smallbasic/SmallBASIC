REM ckurtz11@home.net

    rem scramble: a program to scramble a string

10  rem get a new string, check it, save it, and print it as is
    print "What would you like to scramble"
    print "(a string of 3 to 12 characters) ";:input ao$
    if len(ao$) > 12 then 10
    if len(ao$) < 3 then 10

    randomize timer

20  rem scramble (again)
    print:print ao$
    a$ = ao$
    an$=a$
    count = 1

30  rem generate a random number between 1 and len(ao$)
    rem and check to see if that letter is available
    pick = int(rnd*len(ao$) + 1)
    ag$ = mid$(a$, pick, 1)
    if ag$ = "." then 30:rem letter is taken

40  rem place period claiming letter
    if pick = 1 then a$ = "." + right$(a$,len(ao$) - 1):goto 50
    if pick = len(ao$) then a$ = left$(a$,len(ao$) - 1) + ".":goto 50
    a$ = left$(a$,pick-1) + "." + right$(a$,len(ao$) - pick)

50  rem place letter in an$
    if count = 1 then an$ = ag$ + right$(an$,len(ao$) - 1):goto 70
    if count = len(ao$) then an$ = left$(an$,len(ao$) - 1) + ag$:goto 70

    an$ = left$(an$,count - 1) + ag$ + right$(an$,len(ao$) - count)

70  count = count + 1
    if count > len(ao$) then 100
    goto 30

100 print "..........":print an$

110 rem does the user want to scramble again
    print:print "again(y/n) ";:input ag$
    if ag$ = "Y" or ag$ = "y" then 20
    if ag$ = "N" or ag$ = "n" then 120
    goto 130

120 rem does the user want to input a new string
    print:print:print "another word(y/n) ";:input ag$
    if ag$ = "Y" or ag$ = "y" then 10

130 print:print:print "Thank you for using 'scramble'."
    print "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^"

    end
