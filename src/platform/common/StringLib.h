// This file is part of SmallBASIC
//
// Copyright(C) 2001-2013 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#ifndef STRINGLIB_H
#define STRINGLIB_H

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

//--String----------------------------------------------------------------------
 
struct String {
  String();
  String(const char *s);
  String(const char *s, int len);
  String(const String &s);
  virtual ~String();

  const String &operator=(const String &s);
  const String &operator=(const char *s);
  const String &operator=(const char c);
  const void operator+=(const String &s);
  const void operator+=(const char *s);
  const void operator+=(int i);
  const char operator[] (int index);
  const String operator+(const String &s);
  const String operator+(int i);
  const String & operator=(int i);
  String &append(const String &s);
  String &append(const String *s);
  String &append(int i);
  String &append(char c);
  String &append(double d);
  String &append(int i, int padding);
  String &append(const char *s);
  String &append(const char *s, int numCopy);
  String &append(FILE *fp, long len);
  operator const char *() const { return _buffer; } 
  const char *toString() const { return _buffer; }
  int length() const { return (_buffer == 0 ? 0 : strlen(_buffer)); }
  void replaceAll(char a, char b);
  void toUpperCase();
  void toLowerCase();
  int toInteger() const { return (_buffer == 0 ? 0 : atoi(_buffer)); }
  double toNumber() const { return (_buffer == 0 ? 0 : atof(_buffer)); }
  bool equals(const String &s, bool ignoreCase = true) const;
  bool equals(const char *s, bool ignoreCase = true) const;
  bool equalsIgnoreCase(const char *s) const;
  bool startsWith(const char *s, bool ignoreCase = true) const;
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
  String replaceAll(const char *srch, const char *repl);
  const char *c_str() const { return toString(); };

protected:
  char *_buffer;
};

//--List------------------------------------------------------------------------

#define List_each(type, itr, v) \
  for (strlib::List<type>::TP itr = (v).begin(); itr != (v).end(); itr++)

template<typename T>
struct List {
  typedef T* TP;

  List(int growSize = 20) :
    _head(0),
    _growSize(growSize),
    _count(0),
    _size(0) {
    init();
  }

  virtual ~List() {
    for (int i = 0; i < _count; i++) {
      delete _head[i];
    }
    free(_head);
  }

  /**
   * Removes the list and the list contents 
   */
  void removeAll() {
    for (int i = 0; i < _count; i++) {
      delete _head[i];
    }
    emptyList();
  }

  /**
   * Empties the list without deleteing the list objects 
   */
  void emptyList() {
    free(_head);
    init();
  }

  /**
   * Returns the number of items in the list
   */
  int size() const { return _count; } 

  /**
   * Returns T at the given index
   */
  T operator[] (const int index) const { return index < _count ? _head[index] : NULL; }

  /**
   * Returns T at the given index
   */
  T get(const int index) const { return index < _count ? _head[index] : NULL; }

  /**
   * Adds T to the list
   */
  void add(T object) {
    if (++_count > _size) {
      _size += _growSize;
      _head = (TP) realloc(_head, sizeof(TP) * _size);
    }
    _head[_count - 1] = object;
  }

  /** 
   * Removes the element pointed to by TP i.
   */
  void remove(TP i) {
    TP e = end();
    while(i != (e-1)) {
      *i = *(i+1);
      i++;
    }
    _count--;
  }

  /** 
   * Returns an TP pointing to the first element of the List.
   */
  TP begin() { return _head; }

  /**
   * Returns an TP pointing beyond the last element of the List.
   */
  TP end() { return _head + _count; }

  /**
   * String specialisation - Add a String to the list
   */
  void add(const char *s) {
    add(new String(s, strlen(s))); 
  }

  void sort(int(*compareFunc)(const void *p1, const void *p2)) {
    if (_size > 1) {
      qsort(_head, _count, sizeof(TP), compareFunc);
    }
  }

protected:
  void init() {
    _count = 0;
    _size = _growSize;
    _head = (TP) malloc(sizeof(TP) * _size);
  }

  TP _head;
  int _growSize;
  int _count;
  int _size;
};

//--Stack-----------------------------------------------------------------------

template<typename T> 
struct Stack : public List<T> {
  Stack() : List<T>() {}
  Stack(int growSize) : List<T>(growSize) {}
  T peek() { return !this->_count ? 0 : this->_head[this->_count - 1]; }
  T pop() { return !this->_count ? 0 : this->_head[--this->_count]; }
  void push(T o) { add(o); }
};
 
//--Properties------------------------------------------------------------------

struct Properties : public List<String *> {
  Properties() : List<String *>() {}
  Properties(int growSize) : List<String *>(growSize) {}
  virtual ~Properties() {}
  
  void load(const char *s);
  void load(const char *s, int len);
  String *get(const char *key);
  String *get(int i) const;
  String *getKey(int i) const;
  int length() const { return _count / 2; } 
  void get(const char *key, List<String *> *arrayValues);
  void operator=(Properties &p);
  void put(String &key, String &value);
  void put(const char *key, const char *value);
  String toString();
};

}

#endif

