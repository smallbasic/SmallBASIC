#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>

#define	VER	"1.0.2"

#if !defined(MAX_PATH)
	#define MAX_PATH 1024
#endif

/**
*/
int		debug_flag = 0;

/**
*/
void	usage(void)
{
	printf("d2u %s 1998-99 Nicholas Christopoulos (wired_gr@yahoo.com)\n", VER);
	printf("usage: d2u [-d [-v]] [file [file2 ...]]\n");
	exit(1);
}

/*
*	convert it
*/
int	d2u(char *src_file, char *dst_file)
{
	FILE	*fi, *fo;
	int	i, c;
	char	tmp_file[PATH_MAX+1];
	char	bak_file[PATH_MAX+1];
	
	if	( debug_flag )
		printf("d2u: %s -> %s\n", src_file, dst_file);

	#if defined(_UnixOS)
		sprintf(tmp_file, "/tmp/d2u.%d.%d.tmp", getuid(), getpid());
	#elif defined(_DOS)
		sprintf(tmp_file, "d2u.tmp");
	#else /* windows */
		sprintf(tmp_file, "d2u%d.tmp", getpid());
	#endif
	
	if	( debug_flag )
		printf("d2u: create temporary file: %s\n", tmp_file);
	
	fi = fopen(src_file, "rt");
	if	( fi )	{
		fo = fopen(tmp_file, "wt");

		if	( fo )	{
			while ( (c = fgetc(fi)) != EOF )	{
				if	( c != '\r' && c != 0x1a )	/*** not 13 or 26 (EOT used by old DOS editors (like brief)) ***/
					fputc(c, fo);
				}
		
			fclose(fo);
			fclose(fi);

			strcpy(bak_file, src_file);
			strcat(bak_file, ".bak");
			if	( debug_flag )
				printf("d2u: create backup file: %s\n", bak_file);
				
			if	( rename(src_file, bak_file) != -1 )	{
				rename(tmp_file, dst_file);
				remove(bak_file);
				}
			else	
				perror("d2u-rename");
			}
		else	{
			fclose(fi);
			fprintf(stderr, "d2u-fopen(%s): ", tmp_file);
			perror("");
			}
		}
	else	{
		fprintf(stderr, "d2u-fopen(%s): ", src_file);
		perror("");
		}
}

/**
*	main()
*/
int main(int argc, char *argv[])
{
	int	   		i, r;
	struct stat	st;
	
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
				return 1;
				}
			}
		else	{
			/*** file ***/
			stat(argv[i], &st);
		
			if ( !(st.st_mode & S_IFDIR) )	{	/* its not a dir */
				#if defined(_UnixOS)
				if	( (st.st_mode & S_IWUSER) && (st.st_mode & S_IRUSER)  && (st.st_mode & S_IFREG) )
				#endif
					d2u(argv[i], argv[i]);
				}
			}
		}
	return 0;
}
