#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "fb2p.h"

typedef char * char_p;

int		LoadSBPDB(const char *fname, char_p *rtext);
int		SaveSBPDB(const char *fname, const char *text);

Fl_Window		*fm;
Fl_Text_Buffer	*f_src_buf;
Fl_Text_Buffer	*f_out_buf;

char		srcfile[1024];
char		trgfile[1024];

void	convert()
{
	int		from_pdb = 0;
	int		h;
	char	*txt = NULL;
	char	*ext;

	// check if srcfile is PalmOS file
	ext = strrchr(srcfile, '.');
	if	( ext )	{
		if	( strcasecmp(ext, ".pdb") == 0 )
			from_pdb = 1;
		}

	f_out_buf->append(srcfile);
	f_out_buf->append("->");
	f_out_buf->append(trgfile);
	f_out_buf->append(": ");

	if	( from_pdb )	{
		// convert: pdb -> bas
		switch ( LoadSBPDB(srcfile, &txt) )	{
		case -1:
			f_out_buf->append("can't open file!\n");
			break;
		case -2:
			f_out_buf->append("file i/o error!\n");
 			break;
		case -3:
			f_out_buf->append("section > 32KB\n");
 			break;
		case -4:
			f_out_buf->append("bad signature!\n");
 			break;
		default:
			h = open(trgfile, O_CREAT | O_TRUNC | O_RDWR );
			if	( h != -1 )	{
				write(h, txt, strlen(txt));
				close(h);
				}
			f_out_buf->append("done\n");
			free(txt);
			}
		}
	else	{
		// convert: bas -> pdb
		struct stat st;

		stat(srcfile, &st);
		h = open(srcfile, O_RDWR);
		if	( h != -1 )	{
			txt = (char *) malloc(st.st_size+1);
			read(h, txt, st.st_size);
			txt[st.st_size] = '\0';
			close(h);
			SaveSBPDB(trgfile, txt);
			free(txt);
			f_out_buf->append("done\n");
			}
		else	{
			f_out_buf->append("can't open file!\n");
			}
		}
}

void	src_update()
{
	int		from_pdb = 0;
	char	*ext;

	// check if the srcfile is PalmOS file
	ext = strrchr(srcfile, '.');
	if	( ext )	{
		if	( strcasecmp(ext, ".pdb") == 0 )
			from_pdb = 1;
		}

	f_out_buf->append(srcfile);
	f_out_buf->append(": ");

	if	( from_pdb )	{
		// show it
		char	*txt = NULL;

		switch ( LoadSBPDB(srcfile, &txt) )	{
		case -1:
			f_out_buf->append("can't open file!\n");
			break;
		case -2:
			f_out_buf->append("file i/o error!\n");
 			break;
		case -3:
			f_out_buf->append("section > 32KB\n");
 			break;
		case -4:
			f_out_buf->append("bad signature!\n");
 			break;
		default:
			f_src_buf->select(0, f_src_buf->length());
			f_src_buf->remove_selection();
			f_src_buf->append(txt);
			free(txt);
			f_out_buf->append("loaded...\n");
			}
		}
	else	{
		f_src_buf->loadfile(srcfile);
		f_out_buf->append("loaded...\n");
		}
}

/*
*/
main(int argc, char *argv[])
{
	srcfile[0] = trgfile[0] = '\0';

	fm = make_window();
	f_src_buf = new Fl_Text_Buffer();
	f_out_buf = new Fl_Text_Buffer();
	f_dispcode->buffer(f_src_buf);
	f_console->buffer(f_out_buf);
	f_out_buf->append("PDB<=>BAS utility version 1.0\n\n");
	fm->show(argc, argv);
	return Fl::run();
}
