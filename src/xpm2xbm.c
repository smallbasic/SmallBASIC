/**
*	XPM to XBM convertor
*
*	1999 Nicolas Christopoulos (wired_gr@yahoo.com)
*
*	99/06/29 - ndc - first release (v 1.0.0)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* cl-switches */
int		sw_invert = 0;
char	sw_names[2][1024];
int		sw_name_count = 0;

/**
*	usage()!
*/
int		usage()
{
	fprintf(stderr, "usage: xpm2xbm <source> <target> [-i]\n");
	return 1;
}

/**
*	names_rq		names (file-names required)
*	max_names		maximum names
*	return 0 for ok
*/
int		parsopts (int argc, char *argv[], int names_rq, int max_names)
{
	int		i;
	int		name_c = 0;

	sw_name_count = 0;

	for ( i = 1; i < argc; i++ ) {
		if	( argv[i][0] == '-' )	{
			/* switch */
			if ( (strcmp(argv[i], "-i") == 0) || (strcmp(argv[i], "--invert") == 0) ) 
				sw_invert = 1;
			else	
				return usage();
			}
		else	{
			/* name */
			name_c ++;
			if	( name_c > max_names )	
				return usage();

			strcpy(sw_names[sw_name_count], argv[i]);
			sw_name_count ++;
			}
		}

	// names
	if	( name_c < names_rq )
		return usage();

	return 0;
}

/**
*	add extention 'ext' to the filename 'file'
*/
void	add_ext(char *file, char *ext)
{
	char	*p = strrchr(file, '.');

	if	( p == NULL )
		strcat(file, ext);
	else	{
		if	( strcasecmp(p, ext) != 0 )
			strcat(file, ext);
		}
}

/**
*	change/appned the extention 'new_ext' to file-name 'file'.
*/
void	change_ext(char *file, char *old_ext, char *new_ext)
{
	char	*p = strrchr(file, '.');
	
	if	( p == NULL )
		strcat(file, new_ext);
	else	{
		if	( strcasecmp(p, old_ext) != 0 )
			strcat(file, new_ext);
		else	{
			*p = '\0';
			strcat(file, new_ext);
			}
		}
}

/**
*/
int 	main(int argc, char *argv[])
{
	FILE	*fin, *fout;
	char	buff[1024], *p, *p_start;
	char	var_name[128];
	int		width, height, colors;
	int		symbol[2], count, byte, bit, count_bytes;

	/*
	*	check options
	*/
	if	( parsopts(argc, argv, 1, 2) )	
		return 255;

	add_ext(sw_names[0], ".xpm");

	if	( sw_name_count == 1 )	{
		strcpy(sw_names[1], sw_names[0]);
		change_ext(sw_names[1], ".xpm", ".xbm");
		}
		
	/*
	*	open xpm (source)
	*/
	if ( (fin = fopen(sw_names[0], "rt")) == NULL )	{
		fprintf(stderr, "%s: source file does not exists\n", sw_names[0]);
		return 1;
		}

	/*
	*	check file signature
	*/
	fgets(buff, 1024, fin);	
	if	( memcmp(buff, "/* XPM */", 9) != 0 )	{
		fprintf(stderr, "%s: bad signature\n", argv[1]);
		return 1;
		}
		
	/*
	*	get variable name
	*/
	fgets(buff, 1024, fin);
	if	( memcmp(buff, "static char*", 12) != 0 )	{
		fprintf(stderr, "%s: unknown variable name\n", argv[1]);
		return 1;
		}
	else	{
		p = buff;	p += 12;
		while ( *p && *p != '[' && *p != ' ' )	p ++;
		*p = '\0';
		p = buff;	p += 12;
		strcpy(var_name, p);
		}

	/*
	*	get image header
	*/
	fgets(buff, 1024, fin);

	/* width */
	p = buff; p ++;
	p_start = p;
	while ( *p && *p != ' ' )	p ++;
	*p = '\0';
	width = atoi(p_start);

	/* heigth */
	p ++;
	p_start = p;
	while ( *p && *p != ' ' )	p ++;
	*p = '\0';
	height = atoi(p_start);
	
	/* colors */
	p ++;
	p_start = p;
	while ( *p && *p != ' ' )	p ++;
	*p = '\0';
	colors = atoi(p_start);
	if	( colors > 2 )	{
		fprintf(stderr, "%d: only two colors images can be used.\n", colors);
		return 1;
		}

	/* get first symbol */
	fgets(buff, 1024, fin);
	symbol[0] = buff[1];

	/* get second symbol */
	fgets(buff, 1024, fin);
	symbol[1] = buff[1];

	/* */
	if ( (fout = fopen(argv[2], "wt")) == NULL )	{
		fprintf(stderr, "%s: cannot create destination file\n", argv[2]);
		return 1;
		}

	/* write the header */
	fprintf(fout, "#define %s_width %d\n", var_name, width);
	fprintf(fout, "#define %s_height %d\n", var_name, height);
	fprintf(fout, "static unsigned char %s_bits[] = {\n", var_name);

	/* init */
	count = count_bytes = 0;
	byte = 0;
	bit = 0;

	/* loop */
	while ( fgets(buff, 1024, fin) )	{

		if	( buff[0] != '\"' )
			break;	/* it must be an error */

		p = buff;
		p ++;
		while ( *p && *p != '\"' )	{

			/* get the bit mask */
			bit = (*p == symbol[sw_invert]) ? 0 : 0x80;

			if	( count > 7 )	{
				/* I have 8 bits, so write them */
				fprintf(fout, "0x%02X, ", byte);

				/* wrap */
				count_bytes ++;
				if	( count_bytes > 15 )	{
					fprintf(fout, "\n");
					count_bytes = 0;
					}

				/* reset this byte */
				count = 0;
				byte = 0;
				}

			/* add the bit! */
			byte = byte >> 1;
			byte = byte | bit;

			/* next bit, next char */
			count ++;
			p ++;
			}
		};

	/* the last byte */
	if	( count )	/*  */
		fprintf(fout, "0x%02X };\n", byte);
	else
		fprintf(fout, "0x00 };\n");

	/* bye, bye ... */
	fclose(fin);
	fclose(fout);

	return 0;
}
