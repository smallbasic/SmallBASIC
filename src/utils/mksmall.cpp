/**
*	a stupid utility that converts file names to lower-case names
*	Nicholas Christopoulos (wired_gr@yahoo.com)
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

#if defined(STRLWR)
char*    strlwr(char *source)
{
        char    *p = source;

        while ( *p )    {
                if      ( *p >= 'A' && *p <= 'Z' )
                        *p = (*p + 32);
                p ++;
                }
        return source;
}
#endif

main(int argc, char **argv)
{
	int		i;
	char	msg[1024];

	if	( argc > 1 )	{
		for ( i = 1; i < argc; i ++ )	{
                        struct  stat    st;

                        stat(argv[i], &st);
                        if      ( !(st.st_mode & S_IFDIR) )     {
        			char	*source = argv[i];
        			char	*dest = new char[strlen(source)+1];
	
        			strcpy(dest, source);
        			strlwr(dest);
                                if      ( strcmp(source, dest) != 0 )        {
                                        printf("rename %s -> %s\n", source, dest);
                			if	( rename(source, dest) )	{
                				sprintf(msg, "mksmall:%s", source);
                				perror(msg);
                				}
                                        }
        			delete[] dest;
                                }
			}
		}
	else
		printf("mksmall ver 1.0.1\nusage: mksmall [file1 [file2 [...]]]\n");
	return 0;
}
