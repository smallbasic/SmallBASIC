/*
 * help.cc
 *
 * Turbo Vision - Version 2.0
 *
 * Copyright (c) 1994 by Borland International
 * All Rights Reserved.
 *
 * Modified by Sergio Sigala <ssigala@globalnet.it>
 */
 
// SET: moved the standard headers before tv.h
#include <ctype.h>
#include <stdio.h>
#include <limits.h>
#define Uses_string
#include <sys/stat.h>

#define Uses_MsgBox
#define Uses_TStreamableClass
#define Uses_TPoint
#define Uses_TStreamable
#define Uses_ipstream
#define Uses_opstream
#define Uses_fpstream
#define Uses_TRect
#define Uses_TScrollBar
#define Uses_TScroller
#define Uses_TDrawBuffer
#define Uses_TEvent
#define Uses_TWindow
#define Uses_TKeys
#define Uses_TPalette
#include <tv.h>

#if !defined( __HELP_H )
#include "help.h"
#endif  // __HELP_H

#if !defined( __UTIL_H )
#include "util.h"
#endif  // __UTIL_H

// THelpViewer

THelpViewer::THelpViewer( const TRect& bounds, TScrollBar* aHScrollBar,
    TScrollBar* aVScrollBar, THelpFile *aHelpFile, ushort context )
    : TScroller( bounds, aHScrollBar, aVScrollBar )
{
	THelpPos	posNode;

    options = (options | ofSelectable);
    growMode = gfGrowHiX | gfGrowHiY;
    hFile = aHelpFile;

	posTail = 0;
	posNode.x = posNode.y = 0;
	posNode.context = context;
	push(&posNode);

    topic = aHelpFile->getTopic(mainContext = thisContext = context);
    topic->setWidth(size.x);
    setLimit(78, topic->numLines());
    selected = 1;
}

THelpViewer::~THelpViewer()
{
    delete hFile;
    delete topic;
}

void	THelpViewer::pop(THelpPos *node)	
{
	if	( posTail )
		posTail --;
	memcpy(node, &posStack[posTail], sizeof(THelpPos));
}

void	THelpViewer::push(THelpPos *node)	
{
	memcpy(&posStack[posTail], node, sizeof(THelpPos));
	posTail ++;
	if	( posTail == THelpPosNodes )	{
		int		shift = THelpPosNodes / 3;
		int		remain = THelpPosNodes - shift;
		int		i;

		for ( i = 0; i < remain; i ++ )
			posStack[i] = posStack[i+shift];
		posTail = remain;
		}
}


void THelpViewer::changeBounds( const TRect& bounds )
{
    TScroller::changeBounds(bounds);
    topic->setWidth(size.x);
    setLimit(limit.x, topic->numLines());
}

void THelpViewer::draw()
{
    TDrawBuffer b;
    char line[256];
    char buffer[256];
    char *bufPtr;
    int i, j, l;
    int keyCount;
    ushort normal, keyword, selKeyword, c;
    TPoint keyPoint;
    uchar keyLength;
    int keyRef;

    normal = getColor(1);
    keyword = getColor(2);
    selKeyword = getColor(3);
    keyCount = 0;
    keyPoint.x = 0;
    keyPoint.y = 0;
    topic->setWidth(size.x);

    if (topic->getNumCrossRefs() > 0)  {
        do  {
            topic->getCrossRef(keyCount, keyPoint, keyLength, keyRef);
            ++keyCount;
            } while ( (keyCount < topic->getNumCrossRefs()) &&
                      (keyPoint.y <= delta.y));
        }

    for (i = 1; i <= size.y; ++i)
        {
        b.moveChar(0, ' ', normal, size.x);
        strcpy(line, topic->getLine(i + delta.y, buffer, sizeof( buffer )));
//        if (strlen(line) > delta.x) /* XXX */
        if ((int)strlen(line) > delta.x) /* XXX */
            {
            bufPtr = line + delta.x;
            strncpy(buffer, bufPtr, size.x);
            buffer[size.x] = 0;
            b.moveStr(0, buffer, normal);
            }
        else
            b.moveStr(0, "", normal);
        while (i + delta.y == keyPoint.y)
            {
            l = keyLength;
            if (keyPoint.x < delta.x )
                {
                l -= (delta.x - keyPoint.x);
                keyPoint.x = delta.x;
                }
            if (keyCount == selected)
                c = selKeyword;
            else
                c = keyword;
            for(j = 0; j < l; ++j)
                b.putAttribute((keyPoint.x - delta.x + j),c);
            if (keyCount < topic->getNumCrossRefs())
                {
                topic->getCrossRef(keyCount, keyPoint, keyLength, keyRef);
                keyCount++;
                }
            else
                keyPoint.y = 0;
            }
        writeLine(0, i-1, size.x, 1, b);
        }
}

TPalette& THelpViewer::getPalette() const
{
    static TPalette palette( cHelpViewer, sizeof( cHelpViewer)-1 );
    return palette;
}

void THelpViewer::makeSelectVisible( int selected, TPoint& keyPoint,
         uchar& keyLength, int& keyRef )
{
    TPoint d;

    topic->getCrossRef(selected, keyPoint, keyLength, keyRef);
    d = delta;
    if (keyPoint.x < d.x)
        d.x = keyPoint.x;
    if (keyPoint.x > d.x + size.x)
        d.x = keyPoint.x - size.x;
    if (keyPoint.y <= d.y)
        d.y = keyPoint.y-1;
    if (keyPoint.y > d.y + size.y)
        d.y = keyPoint.y - size.y;
    if ((d.x != delta.x) || (d.y != delta.y))
         scrollTo(d.x, d.y);
}

void THelpViewer::switchToTopic( int keyRef )
{
    if (topic != 0)
        delete topic;
    
	topic = hFile->getTopic(thisContext = keyRef);
    topic->setWidth(size.x);
    scrollTo(0, 0);
    setLimit(limit.x, topic->numLines());
    selected = 1;
    drawView();
}

void THelpViewer::handleEvent( TEvent& event )
{

    TPoint keyPoint, mouse;
    uchar keyLength;
    int keyRef;
    int keyCount;
	THelpPos		posNode;

    TScroller::handleEvent(event);
    switch (event.what)
        {

        case evKeyDown:
            switch (event.keyDown.keyCode)
                {
                case kbAltF1:
					posNode.context = thisContext;
					posNode.x = keyPoint.x;
					posNode.y = keyPoint.y;
					push(&posNode);
					switchToTopic(mainContext);
                    break;

				case '\x08':
				case '\x7F':
				case kbLeft:
					pop(&posNode);
					switchToTopic(posNode.context);
                    break;

                case kbTab:
				case kbDown:
                    ++selected;
                    if (selected > topic->getNumCrossRefs())
                        selected = 1;
                    if ( topic->getNumCrossRefs() != 0 )
                        makeSelectVisible(selected-1,keyPoint,keyLength,keyRef);
                    break;

                case kbShiftTab:
				case kbUp:
                    --selected;
                    if (selected == 0)
                        selected = topic->getNumCrossRefs();
                    if ( topic->getNumCrossRefs() != 0 )
                        makeSelectVisible(selected-1,keyPoint,keyLength,keyRef);
                    break;

                case kbEnter:
                    if (selected <= topic->getNumCrossRefs())
                        {
                        topic->getCrossRef(selected-1, keyPoint, keyLength, keyRef);

						posNode.context = thisContext;
						posNode.x = keyPoint.x;
						posNode.y = keyPoint.y;
						push(&posNode);

                        switchToTopic(keyRef);
                        }
                    break;

                case kbEsc:
                    event.what = evCommand;
                    event.message.command = cmClose;
                    putEvent(event);
                    break;

                default:

//					fprintf(stderr, "%d\n", event.keyDown.keyCode);
//					fflush(stderr);

					if	( event.keyDown.keyCode <= 26 )	{
						int		i, count;
						int		ch = (event.keyDown.keyCode - 1) + 'a';
						
						i = selected + 1;
						count = topic->getNumCrossRefs();

						for ( ; i <= count; i ++ )	{
							char	buf[128];

							topic->getCrossRef(i-1, keyPoint, keyLength, keyRef);
							memset(buf, 0, 128);
							topic->getLine(keyPoint.y, buf, 128);

//							fprintf(stderr, "%c==[%s]\n", ch, buf);
//							fflush(stderr);

							if	( tolower(buf[1]) == ch )	{
								selected = i;
								break;
								}
							}

		                if (selected > count )
	                        selected = 1;
	                    if ( topic->getNumCrossRefs() != 0 )	{
	                        makeSelectVisible(selected-1, keyPoint, keyLength, keyRef);
							break;
							}
						}
                    return;
                }
            drawView();
            clearEvent(event);
            break;

        case evMouseDown:
            mouse = makeLocal(event.mouse.where);
            mouse.x += delta.x;
            mouse.y += delta.y;
            keyCount = 0;

            do
            {
                ++keyCount;
                if (keyCount > topic->getNumCrossRefs())
                    return;
                topic->getCrossRef(keyCount-1, keyPoint, keyLength, keyRef);
            } while (!((keyPoint.y == mouse.y+1) && (mouse.x >= keyPoint.x) &&
                  (mouse.x < keyPoint.x + keyLength)));
            selected = keyCount;
            drawView();
            if (event.mouse.doubleClick)	{
				posNode.context = thisContext;
				posNode.x = keyPoint.x;
				posNode.y = keyPoint.y;
				push(&posNode);

                switchToTopic(keyRef);
				}
            clearEvent(event);
            break;

        case evCommand:
            if ((event.message.command == cmClose) && ((owner->state & sfModal) != 0))
                {
                endModal(cmClose);
                clearEvent(event);
                }
            break;
        }
}

// THelpWindow

THelpWindow::THelpWindow( THelpFile *hFile, ushort context ):
       TWindow( TRect(0,0,76,22), helpWinTitle, wnNoNumber ),
       TWindowInit( &THelpWindow::initFrame)
{
    TRect r(0, 0, 76, 22);
    options = (options | ofCentered);
    r.grow(-2,-1);
    insert(new THelpViewer (r,
      standardScrollBar(sbHorizontal | sbHandleKeyboard),
      standardScrollBar(sbVertical | sbHandleKeyboard), hFile, context));
}

TPalette& THelpWindow::getPalette() const
{
    static TPalette palette( cHelpWindow, sizeof(cHelpWindow)-1 );
    return palette;
}

const char * THelpWindow::helpWinTitle = "Help";
const char * THelpFile::invalidContext =
    "\n No help available in this context.";

