/**
*	SBIDE for FLTK
*
*	Nicholas Christopoulos
*/

#include "sbfui.h"
#include <FL/Fl.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Double_Window.H>
#include <FL/fl_ask.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Editor.H>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/stat.h>
#include "cxx_sb.hpp"

typedef char * char_p;

Fl_Window 		*window   = NULL;
Fl_Text_Buffer	*textbuf  = NULL;
Fl_Text_Buffer	*stylebuf = NULL;
Fl_Text_Buffer	*helpbuf  = NULL;
UserInterface	ui;

bool	loading = false;
bool	changed = false;
bool	file_is_pdb = false;
char	filename[1024];

#include "kwp.cxx"

// Syntax highlighting stuff...
Fl_Text_Display::Style_Table_Entry
                   styletable[] = {	// Style table
		     { FL_BLACK,      FL_COURIER,        12 }, // A - Plain
		     { FL_DARK_GREEN, FL_COURIER_ITALIC, 12 }, // B - Line comments
		     { FL_DARK_GREEN, FL_COURIER_ITALIC, 12 }, // C - Block comments
		     { FL_BLUE,       FL_COURIER,        12 }, // D - Strings
		     { FL_DARK_RED,   FL_COURIER,        12 }, // E - Directives
		     { FL_DARK_BLUE,       FL_COURIER_BOLD,   12 }, // F - Functions
		     { FL_DARK_BLUE,       FL_COURIER_BOLD,   12 }, // G - Procedures
		     { FL_BLACK,      FL_COURIER_BOLD,   12 }  // H - Keywords
		   };

int		LoadSBPDB(const char *fname, char_p *rtext);
int		SaveSBPDB(const char *fname, const char *text);

// convert: bas -> pdb
void	bas2pdb(const char *bas, const char *pdb)
{
	struct stat st;
	int		h;
	char	*txt;

	stat(bas, &st);
	h = open(bas, O_RDWR);
	if	( h != -1 )	{
		txt = (char *) malloc(st.st_size+1);
		read(h, txt, st.st_size);
		txt[st.st_size] = '\0';
		close(h);
		SaveSBPDB(pdb, txt);
		free(txt);
		}
}

// convert: pdb -> bas
void	pdb2bas(const char *file, const char *trgfile)
{
	struct stat st;
	int		h;
	char	*txt;

	switch ( LoadSBPDB(file, &txt) )	{
	case -1:
		fl_alert("Can't open file \'%s\':\n%s.", file, strerror(errno));
		break;
	case -2:
		fl_alert("File read error \'%s\':\n%s.", file, strerror(errno));
		break;
	case -3:
		fl_alert("Section > 32KB \'%s\':\n%s.", file, strerror(errno));
		break;
	case -4:
		fl_alert("Bad signature \'%s\':\n%s.", file, strerror(errno));
		break;
	default:
		h = open(trgfile, O_CREAT | O_TRUNC | O_RDWR );
		if	( h != -1 )	{
			write(h, txt, strlen(txt));
			close(h);
			}
		else
			fl_alert("Error writing to file \'%s\':\n%s.", trgfile, strerror(errno));
		free(txt);
		}
}

//
bool	is_pdb(const char *file)
{
	const char *p = strrchr(file, '.');

	if	( p )	{
		if	( strcasecmp(p, ".pdb") == 0 )
			return true;
		}
	return false;
}

// 'compare_keywords()' - Compare two keywords...
int	compare_keywords(const void *a, const void *b) 
{
	return (strcasecmp(*((const char **)a), *((const char **)b)));
}

// 'style_parse()' - Parse text and produce style data.
void style_parse(const char *text, char *style, int length) 
{
	char	current;
	int		col, last;
	char	buf[255], *bufptr;
	const char *temp;

	for ( current = *style, col = 0, last = 0; length > 0; length --, text ++ ) {
		if (current == 'A') {	// Check for directives, comments, strings, and keywords...
			if (col == 0 && *text == '#') 
				current = 'E';		// Set style to directive
			else if (strncmp(text, "REM", 3) == 0) 
		        current = 'B';		// comments
			else if (strncmp(text, "'", 1) == 0) 
		        current = 'B';
/*
			else if (strncmp(text, "/*", 2) == 0)
		        current = 'C';
			else if (strncmp(text, "\\\"", 2) == 0) {
		        // Quoted quote...
				*style++ = current;
				*style++ = current;
				text ++;
				length --;
				col += 2;
				continue;
				}
*/
		else if (*text == '\"') 
        	current = 'D';
		else if ( !last && isalnum(*text) ) {
  			// Might be a keyword...
			for (temp = text, bufptr = buf; isalnum(*temp) && bufptr < (buf + sizeof(buf) - 1); *bufptr++ = *temp++);
			if (!isalnum(*temp)) {
				*bufptr = '\0';
				bufptr = buf;

				if ( bsearch(&bufptr, code_functions, sizeof(code_functions) / sizeof(code_functions[0]), sizeof(code_functions[0]), compare_keywords)) {
				    while (text < temp) {
				      	*style++ = 'F';
				      	text ++;
				      	length --;
				      	col ++;
						}

				    text --;
				    length ++;
				    last = 1;
				    continue;
					}
			  else if (bsearch(&bufptr, code_procedures, sizeof(code_procedures) / sizeof(code_procedures[0]), sizeof(code_procedures[0]), compare_keywords)) {
				    while (text < temp) {
					*style++ = 'G';
					text ++;
					length --;
					col ++;
				    }

			    text --;
			    length ++;
			    last = 1;
			    continue;
				}
			else if (bsearch(&bufptr, code_keywords, sizeof(code_keywords) / sizeof(code_keywords[0]), sizeof(code_keywords[0]), compare_keywords)) {
			    while (text < temp) {
			      	*style++ = 'H';
			      	text ++;
			      	length --;
			      	col ++;
					}

			    text --;
			    length ++;
			    last = 1;
			    continue;
				}
			}
		}
    } 
else if (current == 'C' && strncmp(text, "*/", 2) == 0) {
      // Close a C comment...
      *style++ = current;
      *style++ = current;
      text ++;
      length --;
      current = 'A';
      col += 2;
      continue;
    } else if (current == 'D') {
      // Continuing in string...
      if (strncmp(text, "\\\"", 2) == 0) {
        // Quoted end quote...
	*style++ = current;
	*style++ = current;
	text ++;
	length --;
	col += 2;
	continue;
      } else if (*text == '\"') {
        // End quote...
	*style++ = current;
	col ++;
	current = 'A';
	continue;
      }
    }

    // Copy style info...
    if (current == 'A' && (*text == '{' || *text == '}')) *style++ = 'G';
    else *style++ = current;
    col ++;

    last = isalnum(*text) || *text == '.';

    if (*text == '\n') {
      // Reset column and possibly reset the style
      col = 0;
      if (current == 'B' || current == 'E') current = 'A';
    }
  }
}

// 'style_update()' - Update the style buffer...
void
style_update(int        pos,		// I - Position of update
             int        nInserted,	// I - Number of inserted chars
	     int        nDeleted,	// I - Number of deleted chars
             int        /*nRestyled*/,	// I - Number of restyled chars
	     const char * /*deletedText*/,	// I - Text that was deleted
             void       *cbArg) {	// I - Callback data
  int	start,				// Start of text
	end;				// End of text
  char	last,				// Last style on line
	*style,				// Style data
	*text;				// Text data


  // If this is just a selection change, just unselect the style buffer...
  if (nInserted == 0 && nDeleted == 0) {
    stylebuf->unselect();
    return;
  }

  // Track changes in the text buffer...
  if (nInserted > 0) {
    // Insert characters into the style buffer...
    style = new char[nInserted + 1];
    memset(style, 'A', nInserted);
    style[nInserted] = '\0';

    stylebuf->replace(pos, pos + nDeleted, style);
    delete[] style;
  } else {
    // Just delete characters in the style buffer...
    stylebuf->remove(pos, pos + nDeleted);
  }

  // Select the area that was just updated to avoid unnecessary
  // callbacks...
  stylebuf->select(pos, pos + nInserted - nDeleted);

  // Re-parse the changed region; we do this by parsing from the
  // beginning of the line of the changed region to the end of
  // the line of the changed region...  Then we check the last
  // style character and keep updating if we have a multi-line
  // comment character...
  start = textbuf->line_start(pos);
  end   = textbuf->line_end(pos + nInserted - nDeleted);
  text  = textbuf->text_range(start, end);
  style = stylebuf->text_range(start, end);
  last  = style[end - start - 1];

  style_parse(text, style, end - start);

  stylebuf->replace(start, end, style);
  ((Fl_Text_Editor *)cbArg)->redisplay_range(start, end);

  if (last != style[end - start - 1]) {
    // The last character on the line changed styles, so reparse the
    // remainder of the buffer...
    free(text);
    free(style);

    end   = textbuf->length();
    text  = textbuf->text_range(start, end);
    style = stylebuf->text_range(start, end);

    style_parse(text, style, end - start);

    stylebuf->replace(start, end, style);
    ((Fl_Text_Editor *)cbArg)->redisplay_range(start, end);
  }

  free(text);
  free(style);
}

//
void	style_init()
{
	char *style = new char[textbuf->length() + 1];
	char *text = textbuf->text();
  
	memset(style, 'A', textbuf->length());
	style[textbuf->length()] = '\0';

	if ( !stylebuf )
		stylebuf = new Fl_Text_Buffer(textbuf->length());

	style_parse(text, style, textbuf->length());

	stylebuf->text(style);
	delete[] style;
	free(text);
}

// 
bool	check_save()
{
	void save_cb();	// declaration

	if ( !changed )
		return true;

	int r = fl_choice(
					"The current file has not been saved.\n"
                    "Would you like to save it now?",
                    "Cancel", "Save", "Discard");

	if ( r == 1 ) {
		save_cb(); // Save the file...
		return !changed;
		}

	return (r == 2) ? true : false;
}

// save file
void save_file(char *newfile)
{
	int		r;

	if	( file_is_pdb )	{
		char	temp[1024];

		sprintf(temp, "%s/sbfltk.%d.tmp", getenv("HOME"), getpid());
		r = textbuf->savefile(temp);
		bas2pdb(temp, newfile);
		remove(temp);
		}
	else
		r = textbuf->savefile(newfile);

	if	( r )
		fl_alert("Error writing to file \'%s\':\n%s.", newfile, strerror(errno));
	else
		strcpy(filename, newfile);

	changed = false;
	textbuf->call_modify_callbacks();
}

// load file
void load_file(const char *newfile, int ipos) 
{
	bool	insert = (ipos != -1);
	int		r;
	
	loading = true;
	changed = insert;
	
	if ( !insert ) 
		strcpy(filename, "");
	
	if	( file_is_pdb )	{
		char	temp[1024];

		sprintf(temp, "%s/sbfltk.%d.tmp", getenv("HOME"), getpid());
		pdb2bas(newfile, temp);

		if ( !insert )
			r = textbuf->loadfile(temp);
		else 
			r = textbuf->insertfile(temp, ipos);

		remove(temp);
		}
	else	{
		if ( !insert )
			r = textbuf->loadfile(newfile);
		else 
			r = textbuf->insertfile(newfile, ipos);
		}
	
	if (r)
		fl_alert("Error reading from file \'%s\':\n%s.", newfile, strerror(errno));
	else	{
		if (!insert) 
			strcpy(filename, newfile);
		}
		
	loading = false;
	textbuf->call_modify_callbacks();
}

// set window title
void set_title()
{
	char	title[1024];

	if (filename[0] == '\0')
		strcpy(title, "Untitled");
	else {
    	char *slash;

	    slash = strrchr(filename, '/');
		#if defined(_WinOS)
		if (slash == NULL)
			slash = strrchr(filename, '\\');
		#endif
		if ( slash != NULL )
			strcpy(title, slash + 1);
		else
			strcpy(title, filename);
		}

	if ( changed )
		strcat(title, " (modified)");

	window->label(title);
}

// menu-new callback
void new_cb()
{
	if ( !check_save() ) 
		return;

	filename[0] = '\0';
	textbuf->select(0, textbuf->length());
	textbuf->remove_selection();
	changed = false;
	textbuf->call_modify_callbacks();
}

// menu-save-as callback
void saveas_cb() 
{
	char *newfile;

	newfile = fl_file_chooser("Save file", "SB Source Files (*.bas)\tSB PDB Files(*.pdb)\tAll Files (*)", filename);
	if ( newfile )	{
		file_is_pdb = is_pdb(newfile);
		save_file(newfile);
		}
}

// menu-save callback
void save_cb()
{
	if ( filename[0] == '\0' ) {
    	// No filename - get one!
		saveas_cb();
	    return;
		}

	save_file(filename);
}

// menu-open callback
void open_cb() 
{
	char	*newfile;

	if ( !check_save() )
		return;

	newfile = fl_file_chooser("Open file", "SB Source Files (*.bas)\tSB PDB Files(*.pdb)\tAll Files (*)", filename);
	if ( newfile )	{
		file_is_pdb = is_pdb(newfile);
		load_file(newfile, -1);
		}
}

// menu-quit callback
void quit_cb() 
{
	if ( changed && !check_save() )
    	return;
	exit(0);
}

//
void changed_cb(int, int nInserted, int nDeleted,int, const char*, void* v) 
{
	if ((nInserted || nDeleted) && !loading) changed = true;
	set_title();
	if (loading) ui.editor->show_insert_position();
}

/*
*/
int	main(int argc, char *argv[])
{
	textbuf = new Fl_Text_Buffer;
	style_init();
	window = ui.make_window();
	ui.editor->buffer(textbuf);
	ui.editor->highlight_data(stylebuf, styletable, sizeof(styletable) / sizeof(styletable[0]), 'A', NULL, 0);

//	if	( access("../doc/xs-intro.html", R_OK) == 0 )
//		ui.helpvw->load("../doc/xs-intro.html");
//	else
//		ui.helpvw->load("doc/xs-intro.html");

	helpbuf = new Fl_Text_Buffer;
	if	( access("../doc/ref.txt", R_OK) == 0 )
		helpbuf->loadfile("../doc/ref.txt");
	else
		helpbuf->loadfile("doc/ref.txt");
	ui.helpvw->buffer(helpbuf);

	textbuf->add_modify_callback(style_update, ui.editor);
	textbuf->add_modify_callback(changed_cb, window);
 	window->show(1, argv);
	if (argc > 1) 
		load_file(argv[1], -1);
	else
		new_cb();
	return Fl::run();
}
