/*
*/

#include <cxx_sb.hpp>

char *prog= " \
REM SmallBASIC code\n	\
\n			  			\
print \"Hello world!\"\n\
";

int	main()
{
	SmallBASIC	sb;

	sb.exec(prog);
	return 0;
}

