// This file is part of SmallBASIC
//
// Copyright(C) 2001-2014 Chris Warren-Smith.
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

#ifndef MAX
#define MAX(a,b) ((a<b) ? (b) : (a))
#endif
#ifndef MIN
#define MIN(a,b) ((a>b) ? (b) : (a))
#endif

#ifndef IS_WHITE
#define IS_WHITE(c) (c == ' '|| c == '\n' || c == '\r' || c == '\t')
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
  String &append(const String &s);
  String &append(const String *s);
  String &append(int i);
  String &append(char c);
  String &append(const char *s);
  String &append(const char *s, int numCopy);
  String &append(FILE *fp, long len);
  operator const char *() const { return _buffer; } 
  const char *c_str() const { return _buffer; };
  void   empty();
  bool   equals(const String &s, bool ignoreCase = true) const;
  bool   equals(const char *s, bool ignoreCase = true) const;
  int    indexOf(char chr, int fromIndex) const;
  int    indexOf(const char *s, int fromIndex) const;
  int    lastIndexOf(char chr, int untilIndex) const;
  int    length() const { return (_buffer == NULL ? 0 : strlen(_buffer)); }
  String leftOf(char ch) const;
  int    toInteger() const { return (_buffer == NULL ? 0 : atoi(_buffer)); }
  double toNumber() const { return (_buffer == NULL ? 0 : atof(_buffer)); }
  void   replaceAll(char a, char b);
  String rightOf(char ch) const;
  String substring(int beginIndex) const;
  String substring(int beginIndex, int endIndex) const;
  void   trim();

private:
  char *_buffer;
};

//--List------------------------------------------------------------------------

#define List_each(type, itr, v) \
  for (strlib::List<type>::TP itr = (v).begin(); itr != (v).end(); itr++)

template<typename T>
struct List {
  typedef T *TP;

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

  /**
   * Returns whether string exists in the list
   */
  bool exists(const char *s) {
    bool result = false;
    for (TP it = begin(); it != end(); it++) {
      T next = (*it);
      if (next->equals(s)) {
        result = true;
        break;
      }
    }    
    return result;
  }

  void sort(int (*compareFunc)(const void *p1, const void *p2)) {
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
  T peek() { return !this->_count ? (T)NULL : this->_head[this->_count - 1]; }
  T pop() { return !this->_count ? (T)NULL : this->_head[--this->_count]; }
  void push(T o) { this->add(o); }
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

typedef strlib::List<strlib::String *> StringList;

#endif

