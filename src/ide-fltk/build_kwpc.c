/**
*	builds the kwp.cxx (keword tables)
*/
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>

char	line[256];
char	keyword[256];

typedef struct { char key[64]; } kw_t;

kw_t	ktable[2048];
int		kcount;

void	add_key(const char *key)
{
	strcpy(ktable[kcount].key, key);
	kcount ++;
}

int		key_cmp(const void *a, const void *b)
{
	return strcasecmp( ((kw_t*) a)->key, ((kw_t*) b)->key );
}

void	sort_ktable()
{
	qsort(ktable, kcount, sizeof(kw_t), key_cmp);
}

int		line2keyword(const char *src, char *buf)
{
	char	*p, *k;

	k = buf;
	p = (char *) src;
	while ( *p )	{
		if	( *p == '\n' )
			break;
		*k ++ = *p ++;
		}
	*k = '\0';

	k = buf;
	while ( *k )	{
		*k = tolower(*k);
		k ++;
		}

	return strlen(buf);
}

void	print_ktable(FILE *fp)
{
	int		i;

	for ( i = 0; i < kcount; i ++ )	{
		fprintf(fp, "\"%s\"", ktable[i].key);
		if	( i != kcount - 1)
			fprintf(fp, ", ");
		if	( ((i+1) % 8) == 0 )	
			fprintf(fp, "\n");
		}
}

int	main(int argc, char *argv[])
{
	FILE	*fp, *fo;
	int		mode = 0, ignore;
	int		mode_count = 0;
	
	system("sbasic -pkw > kwp");
	fp = fopen("kwp", "rt");
	fo = fopen("kwp.cxx", "wt");

	//	
	fprintf(fo, "/* automagicaly generated file */\n");
	fprintf(fo, "const char *code_keywords[] = { // List of basic level keywords\n");
	kcount = 0;
	
	while ( fgets(line, 256, fp) )	{
		if	( line2keyword(line, keyword) )	{
			if	( strcmp(keyword, "and") == 0 )		{
				mode = 1;
				mode_count = 0;
				}
			if	( strcmp(keyword, "$$$-functions") == 0 )	{
				mode = 2;
				sort_ktable();
				print_ktable(fo);
			
				fprintf(fo, "};\n\nconst char *code_functions[] = { // functions\n");
			
				kcount = 0;
				mode_count = 0;
				}
			if	( strcmp(keyword, "$$$-procedures") == 0 )	{
				mode = 2;
				sort_ktable();
				print_ktable(fo);
			
				fprintf(fo, "};\n\nconst char *code_procedures[] = { // functions\n");
			
				kcount = 0;
				mode_count = 0;
				}

			ignore = (!(isalpha(keyword[0]))) || (mode == 0);
			if	( !ignore )	
				add_key(keyword);
			}
		}
		
	//
	sort_ktable();
	print_ktable(fo);
	fprintf(fo, "};\n");
		
	fclose(fo);
	fclose(fp);
	return 0;
}
