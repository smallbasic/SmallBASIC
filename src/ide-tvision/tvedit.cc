/*
 * TVision example: a simple text editor
 *
 * Written by Sergio Sigala <sergio@sigala.it>
 */

#define Uses_MsgBox
#define Uses_TApplication
#define Uses_TBackground
#define Uses_TButton
#define Uses_TChDirDialog
#define Uses_TCheckBoxes
#define Uses_TDeskTop
#define Uses_TDialog
#define Uses_TEditWindow
#define Uses_TEditor
#define Uses_TFileDialog
#define Uses_THistory
#define Uses_TInputLine
#define Uses_TKeys
#define Uses_TLabel
#define Uses_TMenuBar
#define Uses_TMenuItem
#define Uses_TSItem
#define Uses_TStaticText
#define Uses_TStatusDef
#define Uses_TStatusItem
#define Uses_TStatusLine
#define Uses_TSubMenu
#define Uses_TScreen
#define Uses_fpstream

#include <fstream.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <tv.h>
#include "help.h"
#include "tprop.hpp"
#include <cxx_sb.hpp>
#include "doc/sbasic.h"
#include "doc/sbeditor.h"

#if !defined(O_BINARY)
	#define O_BINARY	0
#endif

typedef char *	char_p;

static char *sb_help = "doc/sbasic.hlp";
static char *edit_help = "doc/editor.hlp";

// new command codes; standard commands are defined in views.h

enum menu_cmds {
    cmAbout = 2000,
	cmSBRun,
	cmSBBreak,
	cmOpenPDB,
	cmSavePDB,
	cmHelpEditor,
	cmHelpBasic,
	cmGoTo,
	cmSettings,

	cmSBNIL
};

//
class TSBEditWindow : public TEditWindow	{
public:
	TSBEditWindow(const TRect &r, const char *file, int num);

	char*	textDup() const;							// get a copy of the text (allocated with new[])
	void	setText(const char *text);					// set editor's text
	void	setState(ushort aState, Boolean enable);	// window state (handle active_edit_win)
	void	go(int line);
	};

// last active edit-window
static TSBEditWindow	*active_edit_win = NULL;

// constructor
TSBEditWindow::TSBEditWindow(const TRect &r, const char *file, int num) 
	: TEditWindow(r,file,num), TWindowInit(&TSBEditWindow::initFrame)
{
	active_edit_win = this;
//	editor->tabSize = 4;
}

// get a copy of the text (allocated with new[])
char*	TSBEditWindow::textDup() const
{
	char	*text;

//	size = editor->bufLen;
	text = new char[editor->bufSize+1];
    memcpy(text, editor->buffer, editor->curPtr);
    memcpy(&text[editor->curPtr], editor->buffer+(editor->curPtr + editor->gapLen), editor->bufLen - editor->curPtr);
    memset(&text[editor->bufLen], 0, editor->bufSize - editor->bufLen);
	return text;
}

// set text data
void	TSBEditWindow::setText(const char *text)
{
	int		size = strlen(text);

//	memcpy(&buffer[bufSize - data->length], data->buffer, data->length);
//	setBufLen(data->length);
	editor->setBufSize(size+1);
	memcpy(editor->buffer+(editor->bufSize - size), text, size);
	editor->setBufLen(size+1);
}

// setup selected edit-window
void 	TSBEditWindow::setState(ushort aState, Boolean enable)
{
	TEditWindow::setState(aState, enable);
	if	( aState & sfSelected )	
		active_edit_win = this;
}

// go to line (line starts from 1)
void	TSBEditWindow::go(int line)
{
	if	( line > 0 )	{
		editor->lock();
		editor->setCurPtr(0, 0);
		if	( line > 1 )	
			editor->setCurPtr(editor->lineMove(editor->curPtr, line - 1), 0);
		editor->unlock();
		}
}

// main class of our application
class TSBApp: public TApplication 
{
public:
	char	initDir[1024];
	char 	cmdLine[1024];
	bool	grafMode;
	char	confFile[1024];

	TProp	conf;

private:
    
	TSBEditWindow *clipWindow;
    static TDialog *createFindDialog();
    static TDialog *createReplaceDialog();
    static ushort doEditorDialog(int dialog, ...);
    static ushort doExecute(TDialog *p, void *data);
    static ushort doReplacePrompt(TPoint &cursor);
    void aboutBox();
    void cascade();
    void changeDir();
    void newFile();
    void openFile(char *fileSpec);
    void showClipboard();
    void tile();

	void	readConf();
	void	saveConf();

public:
    TSBApp();
    
	static TMenuBar *initMenuBar(TRect r);
    static TStatusLine *initStatusLine(TRect r);
    
	virtual void handleEvent(TEvent& Event);
    virtual void idle();
	void 	SBRun();

	TPalette&	getPalette() const;

	void	bas2pdb(const char *bas, const char *pdb);
	void	pdb2bas(const char *file, const char *trgfile);

	void	alert(const char *fmt, ...);
	char*	ask(const char *title, const char *prompt, const char *defVal);
	int		iask(const char *title, const char *prompt, int defVal);

	void	dlgSettings();
};

//constructor
TSBApp::TSBApp(): TProgInit(&TSBApp::initStatusLine, &TSBApp::initMenuBar, &TSBApp::initDeskTop)
{
    TEditor::editorDialog = doEditorDialog;

    //create a clipboard
    clipWindow = new TSBEditWindow(deskTop->getExtent(), 0, wnNoNumber);
    if (clipWindow != 0)    {
		//remember who is the clipboard; all editor istances will
		//refer to this one when doing clipboard operations
		TEditor::clipboard = (TEditor *) clipWindow->editor;
	
		//put the clipboard in the background
		clipWindow->hide();
		deskTop->insert(clipWindow);
	    }

	sprintf(confFile, "%s/.tsbide", getenv("HOME"));
	readConf();
}

/**
*	read configuration file
*/
void	TSBApp::readConf()
{
	conf.load(confFile);
	strcpy(initDir,	conf.get(NULL, "initdir", ""));
	strcpy(cmdLine,	conf.get(NULL, "cmdline", ""));
	grafMode = conf.getBool(NULL, "grafmode", false);
}

/**
*	save to configuration file
*/
void	TSBApp::saveConf()
{
	conf.set(NULL, "initdir", initDir);
	conf.set(NULL, "cmdline", cmdLine);
	conf.set(NULL, "grafmode", grafMode);
	conf.save(confFile);
}

/**
*	shows the about dialog
*/
void TSBApp::aboutBox()
{
	int		w = 64, h = 16;

    TDialog *box = new TDialog(TRect(0, 0, w, h), "About");

    box->insert(new TStaticText(TRect(1, 2, w-4, h-4),
	"\003TSBIDE\n"
	"\003SmallBASIC IDE for Console\n\003\n"
	"\003SmallBASIC (c) Nicholas Christopoulos\n\003\n"
	"\003TVision port (c) Sergio Sigala"));

    box->insert(new TButton(TRect(w-14, h-4, w-4, h-2), "O~K~", cmOK,
	bfDefault));
    box->options |= ofCentered;
    execView(box);
}

/**
*	Typical ALERT dialog 
*	(messageBox does not support printf()-style parameters)
*/
void	TSBApp::alert(const char *fmt, ...)
{
	va_list	ap;
	char	buf[1024];

	va_start(ap, fmt);
	vsnprintf(buf, 1024, fmt, ap);
	va_end(ap);

	messageBox(buf, mfError | mfOKButton);
}

/**
*	Typical string-input dialog
*
*	title = title of the window
*	prompt = text-box's label
*	defVal = default value (returend if cancel)
*/
char*	TSBApp::ask(const char *title, const char *prompt, const char *defVal)
{
	static char	buf[1024];
    TDialog *pd = new TDialog(TRect(20, 6, 60, 16), title);

	strcpy(buf, defVal);
    if ( pd )	{
		TInputLine	*inp = new TInputLine(TRect(3, 3, 37, 4), 1024);
        pd->insert(inp);
        pd->insert(new TLabel (TRect(  2,  2, 24,  3), prompt, inp));
        pd->insert(new TButton(TRect( 14,  6, 24,  8), "~O~K", cmOK, bfDefault));
        pd->insert(new TButton(TRect( 27,  6, 37,  8), "~C~ancel", cmCancel, bfNormal));
        int result = deskTop->execView(pd);
		if	( result != cmCancel )	{
			inp->getData(buf);
			//active_edit_win->go(atoi(buf));
			}
        }
    destroy(pd);
	return buf;
}

/**
*	Typical integer-input dialog
*
*	title = title of the window
*	prompt = text-box's label
*	defVal = default value (returend if cancel)
*/
int		TSBApp::iask(const char *title, const char *prompt, int defVal)
{
	char	buf[32];

	sprintf(buf, "%d", defVal);
	return atoi(ask(title, prompt, buf));
}

/**
*	Environment parameters dialog
*/
void	TSBApp::dlgSettings()
{
	int		y = 2;
    TDialog *pd = new TDialog(TRect(20, 6, 60, 19), "TSBIDE Settings" );

    if ( pd )	{

		TInputLine	*defdir = new TInputLine(TRect(3, y+1, 37, y+2), 1024);
		defdir->setData(initDir);
        pd->insert(defdir);
        pd->insert(new TLabel (TRect(  2,  y, 24,  y+1), "Default directory", defdir));
		y += 3;

		TCheckBoxes *graf = new TCheckBoxes(TRect( 3, y, 37, y+1), new TSItem("~G~raphics mode", 0) );
		if	( grafMode )
			graf->press(0);
        pd->insert(graf);
		y += 2;

		TInputLine	*cmd = new TInputLine(TRect(3, y+1, 37, y+2), 1024);
		cmd->setData(cmdLine);
        pd->insert(cmd);
        pd->insert(new TLabel (TRect(  2,  y, 24,  y+1), "Command$", cmd));
		y += 3;

        pd->insert(new TButton(TRect( 14,  y, 24,  y+2), "~O~K", cmOK, bfDefault));
        pd->insert(new TButton(TRect( 27,  y, 37,  y+2), "~C~ancel", cmCancel, bfNormal));
        int result = deskTop->execView(pd);
		if	( result != cmCancel )	{
			defdir->getData(initDir);
			cmd->getData(cmdLine);
			grafMode = graf->mark(0);
			saveConf();
			}
        }
    destroy(pd);
}

//moves all the windows in a cascade-like fashion
void TSBApp::cascade()
{
    deskTop->cascade(deskTop->getExtent());
}

//changes the current working directory
void TSBApp::changeDir()
{
    TView *d = validView(new TChDirDialog(0, cmChangeDir));

    if (d != 0)    {
		deskTop->execView(d);
		destroy(d);
	    }
}

//creates a find dialog and then returns his address
TDialog *TSBApp::createFindDialog()
{
    TInputLine *p;
    TDialog *d = new TDialog(TRect(0, 0, 38, 12), "Find");
    if (!d) return 0;

    d->options |= ofCentered;

    d->insert(p = new TInputLine(TRect(3, 3, 32, 4), 80));
    d->insert(new TLabel(TRect(2, 2, 15, 3), "~T~ext to find", p));
    d->insert(new THistory(TRect(32, 3, 35, 4), p, 10));

    d->insert(new TCheckBoxes(TRect(3, 5, 35, 7),
	new TSItem("~C~ase sensitive",
	new TSItem("~W~hole words only", 0))));

    d->insert(new TButton(TRect(14, 9, 24, 11), "O~K~", cmOK, bfDefault));
    d->insert(new TButton(TRect(14+12, 9, 24+12, 11), "Cancel", cmCancel,
	bfNormal));
    d->selectNext(False);
    return d;
}

//creates a replace dialog and then returns his address
TDialog *TSBApp::createReplaceDialog()
{
    TInputLine *p;
    TDialog *d = new TDialog(TRect(0, 0, 40, 16), "Replace");
    if (!d) return 0;

    d->options |= ofCentered;

    d->insert(p = new TInputLine(TRect(3, 3, 34, 4), 80));
    d->insert(new TLabel(TRect(2, 2, 15, 3), "~T~ext to find", p));
    d->insert(new THistory(TRect(34, 3, 37, 4), p, 10));

    d->insert(p = new TInputLine(TRect(3, 6, 34, 7), 80));
    d->insert(new TLabel(TRect(2, 5, 12, 6), "~N~ew text", p));
    d->insert(new THistory(TRect(34, 6, 37, 7), p, 11));

    d->insert(new TCheckBoxes(TRect(3, 8, 37, 12),
	new TSItem("~C~ase sensitive",
	new TSItem("~W~hole words only",
	new TSItem("~P~rompt on replace",
	new TSItem("~R~eplace all", 0))))));

    d->insert(new TButton(TRect(17, 13, 27, 15), "O~K~", cmOK, bfDefault));
    d->insert(new TButton(TRect(28, 13, 38, 15), "Cancel", cmCancel,
	bfNormal));
    d->selectNext(False);
    return d;
}

//This is a function used by TEditor objects to display various dialog boxes.
ushort TSBApp::doEditorDialog(int dialog, ...)
{
    va_list ap;
    void *info = 0;

    va_start(ap, dialog);	//initializes the variable argument list
    info = va_arg(ap, void *);	//get first parameter (if any)
    va_end(ap);

    switch (dialog)    {
    case edOutOfMemory:
		return messageBox("Not enough memory for this operation", mfError + mfOKButton);
    case edReadError:
		return messageBox(mfError + mfOKButton, "Error reading file %s", info);
    case edWriteError:
		return messageBox(mfError + mfOKButton, "Error writing file %s", info);
    case edCreateError:
		return messageBox(mfError + mfOKButton, "Error creating file %s", info);
    case edSaveModify:
		return messageBox(mfInformation + mfYesNoCancel, "%s has been modified. Save?", info);
    case edSaveUntitled:
		return messageBox("Save untitled file?", mfInformation + mfYesNoCancel);
    case edSaveAs:
		return doExecute(new TFileDialog("*.bas", "Save file as", "~N~ame", fdOKButton, 101), info);
    case edFind:
		return doExecute(createFindDialog(), info);
    case edSearchFailed:
		return messageBox("Search string not found", mfError + mfOKButton);
    case edReplace:
		return doExecute(createReplaceDialog(), info);
    case edReplacePrompt:
    	va_start(ap, dialog);
		TPoint *cursor = va_arg(ap, TPoint *);
		va_end(ap);
		return doReplacePrompt(*cursor);
		}
}

//executes a dialog in modal state; similar to TProgram::execute(), but this
//version is fully static
ushort TSBApp::doExecute(TDialog *p, void *data)
{
    ushort result = cmCancel;
    if (1) //validView(p))
    {
	if (data) p->setData(data);
	result = deskTop->execView(p);
	if (result != cmCancel && data) p->getData(data);
	destroy(p);
    }
    return result;
}

//
ushort TSBApp::doReplacePrompt(TPoint &cursor)
{
    TRect r(0, 2, 40, 9);

    r.move((deskTop->size.x - r.b.x) / 2, 0);
    TPoint lower = deskTop->makeGlobal(r.b);
    if (cursor.y <= lower.y) r.move(0, (deskTop->size.y - r.b.y - 2));

    return messageBoxRect(r, "Replace this occurence?",
	mfYesNoCancel + mfInformation);
}

/**
*	run SB program
*/
void TSBApp::SBRun()
{
	TSBEditWindow *wnd = active_edit_win;
	
	if	( wnd )	{
		SmallBASIC	sb;
		char	*text, getbuf[128];
		bool	ok;
					
	 	text = wnd->textDup();

	 	TScreen::suspend();

		if	( grafMode )
			opt_graphics = 2;
		else
			opt_graphics = 0;

		ok = sb.exec(text);

		if	( ok && !grafMode )	{
		 	printf("\nPress [ENTER] to continue...\n");
		 	gets(getbuf);
			}

	 	delete[] text;
		TScreen::resume();
		redraw();

		if	( !ok )	{
			wnd->go(sb.errLine);
			alert("Line %d:\n%s", sb.errLine, sb.errMessage);
			}
		}
}

/**
*	convert file: bas -> pdb
*/
void	TSBApp::bas2pdb(const char *bas, const char *pdb)
{
	struct stat st;
	int		h;
	char	*txt;
	int		SaveSBPDB(const char *fname, const char *text);

	stat(bas, &st);
	h = ::open(bas, O_RDWR);
	if	( h != -1 )	{
		txt = (char *) malloc(st.st_size+1);
		::read(h, txt, st.st_size);
		txt[st.st_size] = '\0';
		::close(h);
		SaveSBPDB(pdb, txt);
		free(txt);
		}
}

/**
*	convert file: pdb -> bas
*/
void	TSBApp::pdb2bas(const char *file, const char *trgfile)
{
	struct stat st;
	int		h;
	char	*txt;
	int		LoadSBPDB(const char *fname, char_p *rtext);

	switch ( LoadSBPDB(file, &txt) )	{
	case -1:
		alert("Can't open file \'%s\':\n%s.", file, strerror(errno));
		break;
	case -2:
		alert("File read error \'%s\':\n%s.", file, strerror(errno));
		break;
	case -3:
		alert("Section > 32KB \'%s\':\n%s.", file, strerror(errno));
		break;
	case -4:
		alert("Bad signature \'%s\':\n%s.", file, strerror(errno));
		break;
	default:
		h = ::open(trgfile, O_CREAT | O_TRUNC | O_RDWR );
		if	( h != -1 )	{
			::write(h, txt, strlen(txt));
			::close(h);
			}
		else
			alert("Error writing to file \'%s\':\n%s.", trgfile, strerror(errno));
		free(txt);
		}
}

/**
*	handles application's events
*/
void TSBApp::handleEvent(TEvent &event)
{
	static int	helpInUse;

    TApplication::handleEvent(event);

    if (event.what == evCommand)	{
		switch (event.message.command) {
		case cmAbout:
			aboutBox();
			break;
	    case cmCascade:
			cascade();
			break;
	    case cmChangeDir:
			changeDir();
			break;
	    case cmNew:
			newFile();
			break;
	    case cmOpen:
			openFile("*.bas");
			break;
	    case cmShowClip:
			showClipboard();
			break;
	    case cmTile:
			tile();
			break;

		case cmSettings:
			dlgSettings();
			break;

		case cmOpenPDB:
			{
    		TFileDialog *d = (TFileDialog *) validView(new TFileDialog("*.pdb", "Open a PDB File", "~N~ame", fdOpenButton, 100));

		    if (d != 0 && deskTop->execView(d) != cmCancel)  {
				char	fileName[PATH_MAX];
				char	temp[PATH_MAX], *p;

				d->getFileName(fileName);
				strcpy(temp, fileName);
				p = strrchr(temp, '.');
				if	( p )
					*p = '\0';
				strcat(temp, ".bas");
				pdb2bas(fileName, temp);

				TView *w = validView(new TSBEditWindow(deskTop->getExtent(), temp, wnNoNumber));
				if (w != 0) deskTop->insert(w);
		    	}
		    destroy(d);
			}
			break;

		case cmSavePDB:
			{
    		TFileDialog *d = (TFileDialog *) validView(new TFileDialog("*.pdb", "Save as PDB File", "~N~ame", fdOKButton, 101));

		    if (d != 0 && deskTop->execView(d) != cmCancel)  {
				char	temp[PATH_MAX];
				char	source[PATH_MAX];
				char	*text;
				int		h;

				d->getFileName(temp);
				sprintf(source, "/tmp/tsbide-%d.bas", getpid());
				h = ::open(source, O_CREAT | O_TRUNC | O_BINARY | O_RDWR);
				if	( h != -1 )	{
				 	text = active_edit_win->textDup();
					::write(h, text, strlen(text));
					::close(h);
					delete[] text;
					bas2pdb(source, temp);
					::remove(source);
					}
		    	}
		    destroy(d);
			}
			break;

		case cmGoTo:
			active_edit_win->go(iask("Go to line", "Line", 0));
			break;

		case cmSBRun:
			SBRun();
			break;

		case cmHelpEditor:
			{
			if	( helpInUse )
				break;

			deskTop->helpCtx = hcEditor;

			fpstream &helpStream = *new fpstream(edit_help, ios::in|ios::binary);
			THelpFile	*helpFile = new THelpFile(helpStream);

			if	( !helpStream )	{
				alert("Could not open EDITOR help file");
				delete helpFile;
				}
			else	{
				helpInUse = 1;
				TWindow *window = (TWindow *) validView(
					new THelpWindow(helpFile, getHelpCtx())
					);
				if	( window ) {
					execView(window);
					destroy(window);
					helpInUse = 0;
					}
				}
			}
			break;

		case cmHelpBasic:
			{
			if	( helpInUse )
				break;

			deskTop->helpCtx = hcIndex;

			fpstream &helpStream = *new fpstream(sb_help, ios::in|ios::binary);
			THelpFile	*helpFile = new THelpFile(helpStream);

			if	( !helpStream )	{
				alert("Could not open SBASIC help file");
				delete helpFile;
				}
			else	{
				helpInUse = 1;
				TWindow *window = (TWindow *) validView(
					new THelpWindow(helpFile, getHelpCtx())
					);
				if	( window ) {
					execView(window);
					destroy(window);
					helpInUse = 0;
					}
				}
			}
			break;

	    default:
			return;
		    }
		};

    clearEvent(event);
}

//returns true if and only if the view at address `p' is tileable and visible
static Boolean isTileable(TView *p, void *)
{
    if ((p->options & ofTileable) != 0 &&
	(p->state & sfVisible) != 0) return True;
    else return False;
}

//called when in idle state
void TSBApp::idle()
{
    TApplication::idle();
    if (deskTop->firstThat(isTileable, 0) != 0)    {
		enableCommand(cmTile);
		enableCommand(cmCascade);
		enableCommand(cmSavePDB);
		enableCommand(cmSBRun);
		enableCommand(cmSBBreak);
	    }
    else     {
		disableCommand(cmSave);
		disableCommand(cmSaveAs);
		disableCommand(cmSavePDB);
		disableCommand(cmSBRun);
		disableCommand(cmSBBreak);
		disableCommand(cmUndo);
		disableCommand(cmCut);
		disableCommand(cmCopy);
		disableCommand(cmPaste);
		disableCommand(cmClear);
		disableCommand(cmFind);
		disableCommand(cmReplace);
		disableCommand(cmSearchAgain);
		disableCommand(cmTile);
		disableCommand(cmCascade);
	    }
}

//creates a new menu bar
TMenuBar *TSBApp::initMenuBar(TRect r)
{
    TSubMenu& sub1 = *new TSubMenu("~\360~", 0, hcNoContext) +
    *new TMenuItem("~A~bout...", cmAbout, kbNoKey, hcNoContext);

    TSubMenu& sub2 = *new TSubMenu("~F~ile", 0, hcNoContext) +
    *new TMenuItem("~O~pen...", cmOpen, kbF3, hcNoContext, "F3") +
    *new TMenuItem("~N~ew", cmNew, kbNoKey, hcNoContext, "") +
    *new TMenuItem("~S~ave", cmSave, kbF2, hcNoContext, "F2") +
    *new TMenuItem("S~a~ve as...", cmSaveAs, kbNoKey, hcNoContext, "") +
	newLine() +
    *new TMenuItem("Open PDB...", cmOpenPDB, kbNoKey, hcNoContext, "") +
    *new TMenuItem("Save PDB...", cmSavePDB, kbNoKey, hcNoContext, "") +
	newLine() +
    *new TMenuItem("~C~hange dir...", cmChangeDir, kbNoKey, hcNoContext, "") +
    *new TMenuItem("E~x~it", cmQuit, kbAltX, hcNoContext, "Alt-X");

    TSubMenu& sub3 = *new TSubMenu("~E~dit", 0, hcNoContext) +
    *new TMenuItem("~U~ndo", cmUndo, kbNoKey, hcNoContext, "") +
    newLine() +
    *new TMenuItem("Cu~t~", cmCut, kbShiftDel, hcNoContext, "Shift-Del") +
    *new TMenuItem("~C~opy", cmCopy, kbCtrlIns, hcNoContext, "Ctrl-Ins") +
    *new TMenuItem("~P~aste", cmPaste, kbShiftIns, hcNoContext, "Shift-Ins") +
    *new TMenuItem("~S~how clipboard", cmShowClip, kbNoKey, hcNoContext, "") +
    newLine() +
    *new TMenuItem("~C~lear", cmClear, kbCtrlDel, hcNoContext, "Ctrl-Del") +
    newLine() +
    *new TMenuItem("~G~o to line", cmGoTo, kbCtrlG, hcNoContext, "Ctrl-G");

    TSubMenu& sub4 = *new TSubMenu("~S~earch", 0, hcNoContext) +
    *new TMenuItem("~F~ind...", cmFind, kbNoKey, hcNoContext) +
    *new TMenuItem("~R~eplace...", cmReplace, kbNoKey, hcNoContext) +
    *new TMenuItem("~S~earch again", cmSearchAgain, kbNoKey, hcNoContext);

    TSubMenu& sub5 = *new TSubMenu("~W~indows", 0, hcNoContext) +
    *new TMenuItem("~S~ize/move", cmResize, kbCtrlF5, hcNoContext, "Ctrl-F5") +
    *new TMenuItem("~Z~oom", cmZoom, kbF5, hcNoContext, "F5") +
    *new TMenuItem("~N~ext", cmNext, kbF6, hcNoContext, "F6") +
    *new TMenuItem("~P~revious", cmPrev, kbShiftF6, hcNoContext, "Shift-F6") +
    *new TMenuItem("~C~lose", cmClose, kbAltF3, hcNoContext, "Alt-F3") +
    *new TMenuItem("~T~ile", cmTile, kbNoKey, hcNoContext) +
    *new TMenuItem("C~a~scade", cmCascade, kbNoKey, hcNoContext);

    TSubMenu& sub6 = *new TSubMenu("~P~rogram", 0, hcNoContext) +
    *new TMenuItem("~R~un", cmSBRun, kbF9, hcNoContext, "F9") +
    *new TMenuItem("~S~ettings", cmSettings, kbNoKey, hcNoContext);

    TSubMenu& sub7 = *new TSubMenu("~H~elp", 0, hcNoContext) +
    *new TMenuItem("~E~ditor", cmHelpEditor, kbF1, hcNoContext, "F1") +
    *new TMenuItem("Small ~B~ASIC", cmHelpBasic, kbCtrlF1, hcNoContext, "Ctrl-F1");
 
    r.b.y =  r.a.y + 1;
    return new TMenuBar(r, sub1 + sub2 + sub3 + sub4 + sub5 + sub6 + sub7);
}

//creates a new status line
TStatusLine *TSBApp::initStatusLine(TRect r)
{
    r.a.y = r.b.y - 1;
    return new TStatusLine(r,
    *new TStatusDef(0, 50) +
    *new TStatusItem("~Alt-X~ Exit", kbAltX, cmQuit) +
    *new TStatusItem("~F2~ Save", kbF2, cmSave) +
    *new TStatusItem("~F3~ Open", kbF3, cmOpen) +
    *new TStatusItem("~Alt-F3~ Close", kbAltF3, cmClose) +
    *new TStatusItem("~F5~ Zoom", kbF5, cmZoom) +
    *new TStatusItem("~F6~ Next", kbF6, cmNext) + 
    *new TStatusItem("~F10~ Menu", kbF10, cmMenu) +
    *new TStatusItem("", kbCtrlF5, cmResize));
}

//Creates a new edit window, with no assigned file name.
//The window title will be `Untitled'.
void TSBApp::newFile()
{
    TView *w = validView(new TSBEditWindow(deskTop->getExtent(), 0, wnNoNumber));
    if (w != 0) deskTop->insert(w);
}

//opens an existing file, whose file name is passed as a parameter
//in `fileSpec'
void TSBApp::openFile(char *fileSpec)
{
	char	wc[1024];

	if	( strlen(initDir) )	
		sprintf(wc, "%s/%s", initDir, fileSpec);
	else
		strcpy(wc, fileSpec);

    TFileDialog *d = (TFileDialog *) validView(new TFileDialog(wc, "Open a File", "~N~ame", fdOpenButton, 100));

    if (d != 0 && deskTop->execView(d) != cmCancel)  {
		char fileName[PATH_MAX];

		d->getFileName(fileName);
		TView *w = validView(new TSBEditWindow(deskTop->getExtent(), fileName, wnNoNumber));
		if (w != 0) deskTop->insert(w);
    	}
    destroy(d);
}

//Shows the clipboard. The user can't really destroy the clipboard: every
//cmClose command will simply move the clipboard window in the background,
//by hiding it.

void TSBApp::showClipboard()
{
    if (clipWindow != 0)  {
		clipWindow->select();
		clipWindow->show();
		}
}

//moves all the windows in a tile-like fashion

void TSBApp::tile()
{
    deskTop->tile(deskTop->getExtent());
}

// colors
#define cpAppColor \
    "\x71\x70\x78\x74\x20\x28\x24" \
	"\x19\x13\x1A\x31\x31\x1F" \
	"\x71\x1F\x37\x3F\x3A\x13\x13\x3E\x21\x3F\x70\x7F\x7A\x13\x13\x70\x7F\x7E" \
    "\x70\x7F\x7A\x13\x13\x70\x70\x7F\x7E\x20\x2B\x2F\x78\x2E\x70\x30" \
    "\x3F\x3E\x1F\x2F\x1A\x20\x72\x31\x31\x30\x2F\x3E\x31\x13\x38\x00" \
    "\x17\x1F\x1A" \
	"\x71\x71\x1E" \
	"\x17\x1F\x1E" \
	"\x20\x2B\x2F\x78\x2E\x10\x30" \
    "\x3F\x3E\x70\x2F\x7A\x20\x12\x31\x31\x30\x2F\x3E\x31\x13\x38\x00" \
    "\x37\x3F\x3A\x13\x13\x3E\x30\x3F\x3E\x20\x2B\x2F\x78\x2E\x30\x70" \
    "\x7F\x7E\x1F\x2F\x1A" \
	"\x20\x32\x31\x71\x70\x2F\x7E\x71\x13\x78\x00" \
    "\x37\x3F\x3A\x13\x13\x30\x3E\x1E"    // VGA attributes

//
TPalette&	TSBApp::getPalette() const
{
	static TPalette pal(cpAppColor, sizeof(cpAppColor)-1);
	static bool palset = false;

	if ( palset )
		return pal;

	palset = true;
	return pal;
}

/**
*	main
*/
int main(int argc, char *argv[])
{
    TSBApp app;

//	app.deskTop->helpCtx = hcNoContext;
	app.deskTop->helpCtx = hcIndex;
	app.run();
    return 0;
}
