/**
*	Change-extention
*
*	Nicholas Christopoulos
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>

#define	VER	"1.0.1"

#if !defined(MAX_PATH)
	#define MAX_PATH 1024
#endif

int		debug_flag = 0;

/**
*/
void	chext(char *dest, const char *src, const char *ext)
{
	char	*p;
	const char *e;

	strcpy(dest, src);
	p = strrchr(dest, '.');
	if	( p )	
		*p = '\0';

	if	( ext )	{
		if	( strcmp(ext, ".") != 0 )	{
			if	( *ext == '.' )
				e = ext+1;
			else	
				e = ext;

			strcat(dest, ".");
			strcat(dest, e);
			}
		}
}

/**
*/
int		usrchext(const char *src, const char *ext)
{
	char	dest[MAX_PATH];
	
	chext(dest, src, ext);
	if	( rename(src, dest) != 0 )
		perror("chext");		
}

/**
*/
void	usage(void)
{
	printf("chext %s 1996-02 Nicholas Christopoulos (inachus@freemail.gr)\n", VER);
	printf("change file extention\n");
	printf("usage: chext [-d [-v]] [file [file2 ...]] .new_ext\n");
	exit(1);
}

/**
*/
int		main(int argc, char *argv[])
{
	int	   	i, r;
	struct stat	st;
	char	newext[MAX_PATH];
	
	strcpy(newext, argv[argc-1]);
	if	( *newext != '.' )
		usage();

	for ( i = 1; i < argc; i ++ )	{
		if	( argv[i][0] == '-' )	{
			/*** option ***/
			switch ( argv[i][1] )	{
			case 'd': 
				debug_flag = 1;
				break;
			case 'h': 
			default:
				usage();
				}
			}
		else	{
			/*** file ***/
			if	( i != argc-1 )	{
				stat(argv[i], &st);
		
				if ( !(st.st_mode & S_IFDIR) )	{	/* its not a dir */
					#if defined(_UnixOS)
					if	( (st.st_mode & S_IWUSER) && (st.st_mode & S_IRUSER)  && (st.st_mode & S_IFREG) )
					#endif
						usrchext(argv[i], newext);
					}
				}
			}
		}
	return 0;
}
