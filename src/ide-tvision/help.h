/*
 * help.h
 *
 * Turbo Vision - Version 2.0
 *
 * Copyright (c) 1994 by Borland International
 * All Rights Reserved.
 *
 * Modified by Sergio Sigala <ssigala@globalnet.it>
 */

#if !defined( __HELP_H )
#define __HELP_H

#define Uses_TStreamable
#define Uses_ipstream
#define Uses_opstream
#define Uses_fpstream
#define Uses_TObject
#define Uses_TPoint
#define Uses_TRect
#define Uses_TEvent
#define Uses_TScroller
#define Uses_TScrollBar
#define Uses_TWindow
#include <tv.h>

#include <helpbase.h>

#define	THelpPosNodes	64

class THelpPos	{
public:
	int		context;
	int		x, y;
	};

// THelpViewer

class THelpViewer : public TScroller
{
public:

    THelpViewer( const TRect&, TScrollBar*, TScrollBar*, THelpFile*, ushort );
    ~THelpViewer();

    virtual void changeBounds( const TRect& );
    virtual void draw();
    virtual TPalette& getPalette() const;
    virtual void handleEvent( TEvent& );
    void makeSelectVisible( int, TPoint&, uchar&, int& );
    void switchToTopic( int );

    THelpFile *hFile;
    THelpTopic *topic;
    int selected;

public:
	int mainContext;
	int thisContext;

	THelpPos	posStack[THelpPosNodes];
	int			posTail;

	void		pop(THelpPos *node);
	void		push(THelpPos *node);
};

// THelpWindow

class THelpWindow : public TWindow
{

    static const char * helpWinTitle;

public:

    THelpWindow( THelpFile*, ushort );

    virtual TPalette& getPalette() const;
};


extern void notAssigned( opstream& s, int value );

extern TCrossRefHandler crossRefHandler;

#endif  // __HELP_H
