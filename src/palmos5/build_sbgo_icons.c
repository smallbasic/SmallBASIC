#include <stdio.h>

#define	BIGICON "../tmp/palm/Tbmp0c26.bin"
#define	SMALLICON "../tmp/palm/Tbmp0c27.bin"

main()
{
	FILE	*fp, *fo;
	int		c, count;

	fp = fopen(BIGICON, "rb");
	if	( !fp )	{
		fprintf(stderr, "\n\abuild_sbgo_icons: run pilrc first\n");
		return 1;
		}
	fo = fopen("sbgo_icons.c", "wt");
	

	//
	fprintf(fo, "byte sbgo_icon[] = {\n");
	count = 0;
	while ( (c = fgetc(fp)) != -1 )	{
		fprintf(fo, "0x%02x, ", c);
		count ++;
		if	( (count % 16) == 0 )
			fprintf(fo, "\n");
		}
	fprintf(fo, "0 }; // size=%d+1\n\n", count);
	fclose(fp);

	fp = fopen(SMALLICON, "rb");
	fprintf(fo, "byte sbgosmall_icon[] = {\n");
	count = 0;
	while ( (c = fgetc(fp)) != -1 )	{
		fprintf(fo, "0x%02x, ", c);
		count ++;
		if	( (count % 16) == 0 )
			fprintf(fo, "\n");
		}
	fprintf(fo, "0 }; // size=%d+1\n\n", count);
	fclose(fo);
	system("touch sbpad.c");
}
