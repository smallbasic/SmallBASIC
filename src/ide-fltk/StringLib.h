// -*- c-file-style: "java" -*-
// $Id: StringLib.h,v 1.1 2005-03-07 22:34:17 zeeb90au Exp $
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

////////////////////////////////////////////////////////////////////////////////
// class Object

struct Object {
    Object();
    virtual ~Object();
};

////////////////////////////////////////////////////////////////////////////////
// class String

class String : public Object {
    public:
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
    void append(int i);
    void append(double d);
    void append(int i, int padding);
    void append(const char *s);
    void append(const char *s, int numCopy);
    void append(FILE *fp, long len);
    operator const char* () const {return buffer;}
    const char* toString() const;
    inline int length() const;
    String substring(int beginIndex) const;
    String substring(int beginIndex, int endIndex) const;
    String replace(char a, char b) const;
    String replaceAll(const char* srch, const char* repl);
    String toUpperCase() const;
    String toLowerCase() const;
    String trim() const;
    int toInteger() const;
    double toNumber() const;
    bool equals(const String &s) const;
    bool equals(const char* s) const;
    bool equalsIgnoreCase(const char* s) const;
    bool startsWith(const String &s) const;
    bool startsWithIgnoreCase(const String &s) const;
    int indexOf(const String &s, int fromIndex) const;
    void empty();
    char charAt(int i) const;
    int indexOf(char chr, int fromIndex) const;
    String lvalue();
    String rvalue();
    
    protected:
    void init() {buffer=0;owner=true;}
    char* buffer;
    bool owner;
};

////////////////////////////////////////////////////////////////////////////////
// class File

class File : public Object {
    public:
    File();
    virtual ~File();

    typedef enum {readMode,writeMode,closedMode} mode;
    bool open(const char* fileName, mode openMode);
    bool open(const char* fileName, bool readWrite);
    bool open(const char* fileName);
    void close();
    String readLine();
    void writeLine(String s);
    bool ready();
    bool isMode(mode curMode) {return curMode == openMode;}
    U32  length();
    bool write(void* buffer, U32 length);
    bool read(void* buffer, U32 length);
    bool seek(unsigned offs);
    static bool exists(const char *file);
    bool isEOF() {return (feof(filep) > 0);}
    
    private:
    void init();
    mode openMode;
    FILE* filep;
};

////////////////////////////////////////////////////////////////////////////////
// class List

struct List {
public:    
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

    // convert the String contents into a char* array - you are
    // responsible for ensuring the array is deleted
    const char** toArray();

private:
    void init();
    Object** head;
    int growSize;
    int count;
    int size;
    int iterator;
};

////////////////////////////////////////////////////////////////////////////////
// class Properties

struct Properties {
    Properties();
    Properties(int growSize);
    virtual ~Properties();

    void load(const char* s);
    void load(const char* s, int len);
    String* get(const char* key);
    String* get(int i) const {return (String*)list[(i*2)+1];}
    String* getKey(int i) const {return (String*)list[i*2];}
    void get(const char* key, List* arrayValues);
    void operator=(Properties& p);
    void put(String& key, String& value);
    void put(const char* key, const char* value);
    void removeAll() {list.removeAll();}
    int length() {return list.length();}
    String toString();
    private:
    List list;
};

#endif
