/**
*      Unix to dos - text file convertion
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>

#if !defined(MAX_PATH)
       #define MAX_PATH 1024
#endif

#define VER "1.0.1"

void   usage(void)
{
	printf("u2d %s 1998-99 Nicholas Christopoulos (wired_gr@yahoo.com)\n", VER);
	printf("usage: u2d file [file2 ...]\n");
	exit(1);
}

/*
*	convert it
*/
int    u2d(char *src_file, char *dst_file)
{
       FILE    *fi, *fo;
       int     i, c;
       char    tmp_file[MAX_PATH+1];
       char    bak_file[MAX_PATH+1];

       sprintf(tmp_file, "d2u.tmp");

       fi = fopen(src_file, "rt");
       if      ( fi )  {
	   		#if defined(_UnixOS)
			fo = fopen(tmp_file, "wb");
			#else
			fo = fopen(tmp_file, "wt");
			#endif

			if      ( fo )  {
				while ( (c = fgetc(fi)) != EOF )        {
					fputc(c, fo);
				   	#if defined(_UnixOS)
					if ( c == '\n' ) 
						fputc('\r', fo);
					#endif
					}
        
				fclose(fo);
				fclose(fi);

                strcpy(bak_file, src_file);
                strcat(bak_file, ".bak");
                        
				if      ( rename(src_file, bak_file) != -1 )    {
					rename(tmp_file, dst_file);
					remove(bak_file);
					}
				else 
					perror("u2d-rename");
				}
			else    {
				fclose(fi);
				fprintf(stderr, "u2d-fopen(%s): ", tmp_file);
				perror("");
				}
			}
       else    {
			fprintf(stderr, "u2d-fopen(%s): ", src_file);
			perror("");
            }
       return 0;
}

/*
*	returns true if the file is a valid unix-text file
*/
int    isUnixTextFile(const char *file)
{
	FILE    *fp;
	int     c, ret = -1;

	if ( (fp = fopen(file, "rb")) == NULL )	{
		perror("d2u");
		return 0;
		}

	while ( (c = fgetc(fp)) != -1 ) {
		if ( c == '\r' )   {
			ret = 0;
			break;
			}
		}

	fclose(fp);
	return ret;
}

/*
*/
int    main(int argc, char *argv[])
{
    int     i;
 
	if      ( argc == 1 ) 
		usage();
 
	for ( i = 1; i < argc; i ++ )   {
		struct  stat    st;

		stat(argv[i], &st);
		if      ( !(st.st_mode & S_IFDIR) )     {
			if      ( isUnixTextFile(argv[i]) )
				u2d(argv[i], argv[i]);
       		else
       			printf("not unix: %s\n", argv[i]);
			}
		}
	return 0;
}

