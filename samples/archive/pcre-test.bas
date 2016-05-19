option match pcre caseless

a="Hello, world"
? "hello", a like "hello"
? "HiThere", a like "HiThere"
? "A", a like "A"
? "[Hh]+", a like "[Hh]+"
? "[Aa]+", a like "[Aa]+"
? "hello,\wworld", a like "hello,\wworld"
? "hello,\sworld", a like "hello,\sworld"

