// -*- c-file-style: "java" -*-
// $Id: StringLib.h,v 1.7 2005-03-28 23:17:52 zeeb90au Exp $
// This file is part of EBjLib
//
// Copyright(C) 2001-2004 Chris Warren-Smith. Gawler, South Australia
// cwarrens@twpo.com.au
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#ifndef STRINGLIB__H
#define STRINGLIB__H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>

typedef unsigned char U8;
typedef unsigned short U16;
typedef signed short S16;
typedef unsigned long U32;

#ifndef null
#define null 0
#endif

#ifndef max
#define max(a,b) ((a<b) ? (b) : (a))
#endif
#ifndef min
#define min(a,b) ((a>b) ? (b) : (a))
#endif

#ifndef isWhite
#define isWhite(c) (c == ' '|| c == '\n' || c == '\r' || c == '\t')
#endif

namespace strlib {

//--Object----------------------------------------------------------------------

struct Object {
    Object();
    virtual ~Object();
};

//--String----------------------------------------------------------------------

struct String : public Object {
    virtual ~String();
    String();
    String(const char* s);
    String(const String& s);
    const String& operator=(const String& s);
    const String& operator=(const char* s);
    const String& operator=(const char c);
    const void operator+=(const String& s);
    const void operator+=(const char* s);
    const String operator+(const String& s);
    const String operator+(int i);
    const String& operator=(int i);
    void append(const String& s);
    void append(const String* s);
    void append(int i);
    void append(double d);
    void append(int i, int padding);
    void append(const char *s);
    void append(const char *s, int numCopy);
    void append(FILE *fp, long len);
    operator const char* () const {return buffer;}
    const char* toString() const;
    int length() const;
    void replaceAll(char a, char b);
    void toUpperCase();
    void toLowerCase();
    int toInteger() const;
    double toNumber() const;
    bool equals(const String &s) const;
    bool equals(const char* s) const;
    bool equalsIgnoreCase(const char* s) const;
    bool startsWith(const String &s) const;
    bool startsWithIgnoreCase(const String &s) const;
    int indexOf(const String &s, int fromIndex) const;
    int lastIndexOf(char chr, int untilIndex) const;
    void empty();
    void trim();
    char charAt(int i) const;
    int indexOf(char chr, int fromIndex) const;
    String lvalue();
    String rvalue();
    String substring(int beginIndex) const;
    String substring(int beginIndex, int endIndex) const;
    String replaceAll(const char* srch, const char* repl);
    String getPath(const char* fileName);
    
    protected:
    void init() {buffer=0;owner=true;}
    char* buffer;
    bool owner;
};

//--List------------------------------------------------------------------------

struct List {
    List(int growSize=20);
    virtual ~List();
    void removeAll(); // Removes the list and the list contents
    void emptyList(); // Empties the list without deleteing the list objects
    int length() const {return count;}
    Object* operator[](const int index) const;
    Object* get(const int index) const;
    void iterateInit(int ibegin=0);
    bool hasNext() const;
    Object* next();
    void append(Object* object);
    Object** getList() {return head;}

    // convert the String contents into a char* array - you are
    // responsible for ensuring the array is deleted
    const char** toArray();
    
    protected:
    void init();
    Object** head;
    int growSize;
    int count;
    int size;
    int iterator;
};

//--Stack-----------------------------------------------------------------------

struct Stack : public List{
    Stack();
    Stack(int growSize);
    Object* peek();
    Object* pop();
    void push(Object* o);
};

//--Properties------------------------------------------------------------------

struct Properties : public List {
    Properties();
    Properties(int growSize);
    virtual ~Properties();

    void load(const char* s);
    void load(const char* s, int len);
    String* get(const char* key);
    String* get(int i) const;
    String* getKey(int i) const;
    int length() const {return count/2;}
    void get(const char* key, List* arrayValues);
    void operator=(Properties& p);
    void put(String& key, String& value);
    void put(const char* key, const char* value);
    String toString();
};

}

#endif

//--EndOfFile-------------------------------------------------------------------
