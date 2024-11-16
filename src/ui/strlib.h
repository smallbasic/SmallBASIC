// This file is part of SmallBASIC
//
// Copyright(C) 2001-2017 Chris Warren-Smith.
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
  void operator+=(const String &s);
  void operator+=(const char *s);
  String &append(const String &s);
  String &append(const String *s);
  String &append(int i);
  String &append(char c);
  String &append(const char *s);
  String &append(const char *s, int numCopy);
  String &append(FILE *fp, long len);
  operator const char *() const { return _buffer; }
  const char *c_str() const { return _buffer; };
  void   clear();
  bool   equals(const String &s, bool ignoreCase = true) const;
  bool   equals(const char *s, bool ignoreCase = true) const;
  bool   endsWith(const String &s) const;
  int    indexOf(char chr, int fromIndex) const;
  int    indexOf(const char *s, int fromIndex) const;
  bool   empty() const { return _buffer == nullptr || _buffer[0] == '\0'; };
  char   lastChar() const { return (_buffer == nullptr || !_buffer[0] ? '\0' : _buffer[strlen(_buffer) - 1]); }
  int    lastIndexOf(char chr, int untilIndex) const;
  int    length() const { return (_buffer == nullptr ? 0 : strlen(_buffer)); }
  String leftOf(char ch) const;
  int    toInteger() const { return (_buffer == nullptr ? 0 : atoi(_buffer)); }
  double toNumber() const { return (_buffer == nullptr ? 0 : atof(_buffer)); }
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
  for (strlib::List<type>::TP itr = (v).begin(); itr < (v).end(); itr++)

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
    clear();
  }

  /**
   * Empties the list without deleteing the list objects
   */
  void clear() {
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
  T operator[] (const int index) const { return index < _count ? _head[index] : nullptr; }

  /**
   * Returns T at the given index
   */
  T get(const int index) const { return index < _count ? _head[index] : nullptr; }

  /**
   * Adds T to the list
   */
  void add(T object) {
    if (++_count > _size) {
      _size += _growSize;
      _head = (TP) realloc(_head, sizeof(T) * _size);
    }
    _head[_count - 1] = object;
  }

  /**
   * Removes the element pointed to by TP i.
   */
  void remove(TP i) {
    TP e = end();
    while (i != (e-1)) {
      *i = *(i+1);
      i++;
    }
    _count--;
  }

  /**
   * Returns an TP pointing to the first element of the List.
   */
  TP begin() const { return _head; }

  /**
   * Returns an TP pointing beyond the last element of the List.
   */
  TP end() const { return _head + _count; }

  /**
   * String specialisation - Add a String to the list of strings
   */
  void add(const char *s);

  /**
   * returns whether the list is empty
   */
  bool empty() const {
    return !_count;
  }

  /**
   * Returns whether list of strings constains the string
   */
  bool contains(const char *s);

  /**
   * Returns whether the list contains t
   */
  bool contains(T t) {
    bool result = false;
    for (T *it = begin(); it < end(); it++) {
      T next = (*it);
      if (next == t) {
        result = true;
        break;
      }
    }
    return result;
  }

  void sort(int (*compareFunc)(const void *p1, const void *p2)) {
    if (_size > 1) {
      qsort(_head, _count, sizeof(T), compareFunc);
    }
  }

protected:
  void init() {
    _count = 0;
    _size = _growSize;
    _head = (TP) malloc(sizeof(T) * _size);
  }

  TP _head;
  int _growSize;
  int _count;
  int _size;
};

// specialisations for String List
template<> void List<String *>::add(const char *s);
template<> bool List<String *>::contains(const char *s);

//--Stack-----------------------------------------------------------------------

template<typename T>
struct Stack : public List<T> {
  Stack() : List<T>() {}
  Stack(int growSize) : List<T>(growSize) {}
  T peek() { return !this->_count ? (T)nullptr : this->_head[this->_count - 1]; }
  T pop() { return !this->_count ? (T)nullptr : this->_head[--this->_count]; }
  void push(T o) { this->add(o); }
};

//--Queue-----------------------------------------------------------------------

template<typename T>
struct Queue : public List<T> {
  Queue() : List<T>() {}
  Queue(int growSize) : List<T>(growSize) {}
  T front() { return !this->_count ? (T)nullptr : this->_head[0]; }
  void pop(bool free=true) {
    if (this->_count) {
      if (free) {
        delete this->_head[0];
      }
      memmove(this->_head, this->_head + 1, (--this->_count) * sizeof(T));
    }
  }
  void push(T o) { this->add(o); }
};

//--Properties------------------------------------------------------------------

template<typename T>
struct Properties : public List<T> {
  Properties() : List<T>() {}
  Properties(int growSize) : List<T>(growSize) {}
  virtual ~Properties() {}

  /**
   * find the position of the key in the list
   */
  int find(const char *key) {
    int result = -1;
    for (int i = 0; i < this->_count; i++) {
      String *nextKey = (String *)this->_head[i++];
      if (nextKey == nullptr || i == this->_count) {
        break;
      }
      T nextValue = this->_head[i];
      if (nextValue == nullptr) {
        break;
      }
      if (nextKey->equals(key)) {
        result = i;
        break;
      }
    }
    return result;
  }

  T get(const char *key) {
    int index = find(key);
    return index == -1 ? nullptr : this->_head[index];
  }

  int length() const {
    return this->_count / 2;
  }

  // for Properties<String *>
  void get(const char *key, List<String *> *arrayValues);
  void load(const char *s);
  void load(const char *s, int len);
  void put(const char *key, const char *value);

  void put(const char *key, T value) {
    int index = find(key);
    if (index != -1) {
      this->_head[index] = value;
    } else {
      this->add((T)new String(key));
      this->add(value);
    }
  }
};

// specialisations for String properties
template<> void Properties<String *>::get(const char *key, List<String *> *arrayValues);
template<> void Properties<String *>::load(const char *);
template<> void Properties<String *>::load(const char *s, int slen);
template<> void Properties<String *>::put(const char *key, const char *value);

}

typedef strlib::List<strlib::String *> StringList;

#endif

