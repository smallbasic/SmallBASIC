/*
*/

#include "sys.h"

int		scan_line;
int		prog_line;
int		opt_quite;
int		opt_usevmt;

extern void hex_dump(const unsigned char *block, int size);

void	usage()
{
	printf("dumpvmt version 0.1\nwritten by Nicholas Christopoulos\n\nCopyrights held by the SmallBASIC team.\nDistributed under the GNU General Public License.\n\n");
	printf("usage: dumpvmt vmt-file\n");
	exit(1);
}

int main(int argc, char *argv[])
{
	char	base[1024];
	char	*p;
	dbt_t	t;
	int		i, count, size;
	char	*data;

	if	( argc != 2 )
		usage();
	strcpy(base, argv[1]);
	p = strrchr(base, '.');
	if	( p )	{
		*p = '\0';
		t = dbt_create(base, 1);

		count = dbt_count(t);
		printf("vmt count=%d\n", count);
		for ( i = 0; i < count; i ++ )	{
			size = dbt_recsize(t, i);
			data = (char *) malloc(size);
			dbt_read(t, i, data, size);
			printf("\nrecord %d, size %d\n", i, size);
			hex_dump(data, size);
			free(data);
			}
		}
	else
		usage();
	return 0;
}
