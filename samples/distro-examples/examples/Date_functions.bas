REM ****
REM DATE
REM ****
? date$
? julian(date$)
? weekday(date$)
? weekday(julian(date$)-1)
? weekday(julian(date$))
? weekday(julian(date$)+1)
? datefmt("yyyy mmm dd ddd", date$)
? datefmt("yyyy mmm dd ddd", julian(date$))
? datefmt("yyyy mmmm dd dddd", julian(date$))
? datefmt("ddd dd, mm/yy", date$)
DateDMY date$, d, m, y
? d;"/";m;"/";y


