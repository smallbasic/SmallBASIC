/*-----------------------------------------------------*/
/*                                                     */
/*   Turbo Vision 1.0                                  */
/*   Turbo Vision TVHC header file                     */
/*   Copyright (c) 1991 by Borland International       */
/*                                                     */
/*-----------------------------------------------------*/

/*
 * Modified by Sergey Clushin <serg@lamport.ru>, <Clushin@deol.ru>
 */

#if !defined( __TVHC_H )
#define __TVHC_H

#define Uses_fstream
#define Uses_TSortedCollection
#define Uses_TObject
#define Uses_TPoint
#include <tv.h>

#include "helpbase.h"


#define MAXSIZE		80
#define MAXSTRSIZE	256
char commandChar[] = ".";
#define bufferSize 4096

typedef enum State { undefined, wrapping, notWrapping };

class TProtectedStream : public fstream
{

public:

    TProtectedStream( char *aFileName, ushort  aMode );

private:

    char  fileName[MAXSIZE];
    ushort mode;

};

// Topic Reference

struct TFixUp
{

    long pos;
    TFixUp *next;

};

union Content
{

//  ushort value;
    int value;		// SC: must be the same type as ÙCrossRef::ref!
    TFixUp *fixUpList;

};

struct TReference 
{

    char *topic;
    Boolean resolved;
    Content val;

};

class TRefTable : public TSortedCollection
{

public:

    TRefTable( ccIndex aLimit, ccIndex aDelta );

    virtual int compare( void *key1,void *key2 );
    virtual void freeItem( void *item );
    TReference *getReference( char *topic );
    virtual void *keyOf( void *item );

private:

    virtual void *readItem( ipstream& ) { return 0; };
    virtual void writeItem( void *, opstream& ) {};

};

struct TCrossRefNode
{

    char *topic;
    int offset;
    uchar length;
    TCrossRefNode *next;

};

class TTopicDefinition : public TObject
{

public:

    TTopicDefinition(char *aTopic, ushort aValue);
    ~TTopicDefinition(void);

    char *topic;
    ushort value;
    TTopicDefinition *next;

};

char* helpName;
uchar buffer[bufferSize];
int ofs;
TRefTable *refTable = 0;
TCrossRefNode  *xRefs;
char line[MAXSTRSIZE] = "";
Boolean lineInBuffer = False;
int lineCount = 0;

#endif  // __TVHC_H
