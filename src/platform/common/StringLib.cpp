// This file is part of SmallBASIC
//
// Copyright(C) 2001-2013 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include "StringLib.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

// uncomment for unit testing and then run:
// g++ StringLib.cpp;./a.exe
// #define UNIT_TEST 1

using namespace strlib;

//--String----------------------------------------------------------------------

String::String(const char *s) : _buffer(0) {
  append(s);
}

String::String(const char *s, int len) : _buffer(0) {
  append(s, len);
}

String::String(const String &s) : _buffer(0) {
  append(s._buffer);
}

String::String() : _buffer(0) {
}

String::~String() {
  empty();
}

const String &String::operator=(const String &s) {
  if (this != &s) {
    empty();
    append(s._buffer);
  }
  return *this;
}

const String &String::operator=(const char *s) {
  empty();
  append(s);
  return *this;
}

const String &String::operator=(const char c) {
  empty();
  append(&c, 1);
  return *this;
}

const void String::operator+=(const String &s) {
  append(s._buffer);
}

const void String::operator+=(const char *s) {
  append(s);
}

const void String::operator+=(int i) {
  append(i);
}

const String String::operator+(const String &s) {
  String rs;
  rs.append(_buffer);
  rs.append(s._buffer);
  return rs;
}

const String String::operator+(int i) {
  String rs;
  rs.append(_buffer);
  rs.append(i);
  return rs;
}

const char String::operator[] (int index) {
  if (index < length()) {
    return _buffer[index];
  }
  return 0;
}

const String &String::operator=(int i) {
  empty();
  append(i);
  return *this;
}

String &String::append(const String &s) {
  append(s._buffer);
  return *this;
}

String &String::append(const String *s) {
  if (s && s->length()) {
    append(s->_buffer);
  }
  return *this;
}

String &String::append(int i) {
  char t[20];
  sprintf(t, "%i", i);
  append(t);
  return *this;
}

String &String::append(char c) {
  char t[2] = { c, 0 };
  append(t);
  return *this;
}

String &String::append(double d) {
  char t[20];
  sprintf(t, "%g", d);
  append(t);
  return *this;
}

String &String::append(int i, int padding) {
  char buf[20];
  char fmt[20];
  fmt[0] = '%';
  fmt[1] = '0';
  padding = min(20, padding);
  sprintf(fmt + 2, "%dd", padding);
  sprintf(buf, fmt, i);
  append(buf);
  return *this;
}

String &String::append(const char *s) {
  if (s != null && s[0]) {
    int len = length();
    _buffer = (char *)realloc(_buffer, len + strlen(s) + 1);
    strcpy(_buffer + len, s);
  }
  return *this;
}

String &String::append(const char *s, int numCopy) {
  if (s == null || numCopy < 1) {
    return *this;
  }
  int len = strlen(s);
  if (numCopy > len) {
    numCopy = len;
  }

  len = length();
  _buffer = (char *)realloc(_buffer, len + numCopy + 1);
  strncpy(_buffer + len, s, numCopy);
  _buffer[len + numCopy] = '\0';
  return *this;
}

String &String::append(FILE *fp, long filelen) {
  int len = length();
  _buffer = (char *)realloc(_buffer, len + filelen + 1);
  fread((void *)(len + _buffer), 1, filelen, fp);
  _buffer[len + filelen] = 0;
  return *this;
}

String String::substring(int beginIndex) const {
  String out;
  if (beginIndex < length()) {
    out.append(_buffer + beginIndex);
  }
  return out;
}

String String::substring(int beginIndex, int endIndex) const {
  String out;
  int len = length();
  if (endIndex > len) {
    endIndex = len;
  }
  if (beginIndex < length()) {
    out.append(_buffer + beginIndex, endIndex - beginIndex);
  }
  return out;
}

void String::replaceAll(char a, char b) {
  int len = length();
  for (int i = 0; i < len; i++) {
    if (_buffer[i] == a) {
      _buffer[i] = b;
    }
  }
}

String String::replaceAll(const char *srch, const char *repl) {
  String out;
  int begin = 0;
  int numCopy = 0;
  int numMatch = 0;
  int len = length();
  int srchLen = strlen(srch);

  for (int i = 0; i < len; i++) {
    numMatch = (_buffer[i] == srch[numMatch]) ? numMatch + 1 : 0;
    if (numMatch == srchLen) {
      numCopy = 1 + i - begin - srchLen;
      if (numCopy > 0) {
        out.append(_buffer + begin, numCopy);
      }
      out.append(repl);
      numMatch = 0;
      begin = i + 1;
    }
  }

  if (begin < len) {
    out.append(_buffer + begin);
  }
  return out;
}

void String::toUpperCase() {
  int len = length();
  for (int i = 0; i < len; i++) {
    _buffer[i] = toupper(_buffer[i]);
  }
}

void String::toLowerCase() {
  int len = length();
  for (int i = 0; i < len; i++) {
    _buffer[i] = tolower(_buffer[i]);
  }
}

bool String::equals(const String &s, bool ignoreCase) const {
  return (_buffer == 0 ? s._buffer == 0 : ignoreCase ?
          strcasecmp(_buffer, s._buffer) == 0 : strcmp(_buffer, s._buffer) == 0);
}

bool String::equals(const char *s, bool ignoreCase) const {
  return (_buffer == 0 ? s == 0 : ignoreCase ? strcasecmp(_buffer, s) == 0 : strcmp(_buffer, s) == 0);
}

bool String::startsWith(const char *s, bool ignoreCase) const {
  if (s == 0 || s[0] == 0) {
    return (_buffer == 0 || _buffer[0] == 0);
  }
  return (ignoreCase ? strncasecmp(_buffer, s, strlen(s)) == 0 : strncmp(_buffer, s, strlen(s)) == 0);
}

int String::indexOf(const String &s, int fromIndex) const {
  int len = length();
  if (fromIndex >= len) {
    return -1;
  }
  if (s.length() == 1) {
    char *c = strchr(_buffer + fromIndex, s._buffer[0]);
    return (c == NULL ? -1 : (c - _buffer));
  } else {
    char *c = strstr(_buffer + fromIndex, s._buffer);
    return (c == NULL ? -1 : (c - _buffer));
  }
}

int String::indexOf(char chr, int fromIndex) const {
  int len = length();
  if (fromIndex >= len) {
    return -1;
  }
  char *c = strchr(_buffer + fromIndex, chr);
  return (c == NULL ? -1 : (c - _buffer));
}

int String::lastIndexOf(char chr, int untilIndex) const {
  int len = length();
  if (untilIndex >= len || untilIndex < 0) {
    return -1;
  }
  char *c = strrchr(_buffer + untilIndex, chr);
  return (c == NULL ? -1 : (c - _buffer));
}

char String::charAt(int i) const {
  if (i < length()) {
    return _buffer[i];
  }
  return 0;
}

void String::empty() {
  if (_buffer != null) {
    free(_buffer);
  }
  _buffer = 0;
}

void String::trim() {
  int len = length();
  if (len == 0) {
    return;
  }
  int ibegin = 0;
  while (isWhite(_buffer[ibegin])) {
    ibegin++;
  }
  int iend = len;
  while (isWhite(_buffer[iend - 1])) {
    iend--;
  }
  String s = substring(ibegin, iend);
  empty();
  append(s);
}

String String::lvalue() {
  int endIndex = indexOf('=', 0);
  if (endIndex == -1) {
    return *this;
  }
  return substring(0, endIndex);
}

String String::rvalue() {
  int endIndex = indexOf('=', 0);
  if (endIndex == -1) {
    return *this;
  }
  return substring(endIndex + 1, length());
}

//--Properties------------------------------------------------------------------

void Properties::load(const char *s) {
  if (s && s[0]) {
    load(s, strlen(s));
  }
}

void Properties::load(const char *s, int slen) {
  if (s == 0 || s[0] == 0 || slen == 0) {
    return;
  }

  String attr;
  String value;

  int i = 0;
  while (i < slen) {
    attr.empty();
    value.empty();

    // remove w/s before attribute
    while (i < slen && isWhite(s[i])) {
      i++;
    }
    if (i == slen) {
      break;
    }
    int iBegin = i;

    // find end of attribute
    while (i < slen && s[i] != '=' && !isWhite(s[i])) {
      i++;
    }
    if (i == slen) {
      break;
    }

    attr.append(s + iBegin, i - iBegin);

    // scan for equals
    while (i < slen && isWhite(s[i])) {
      i++;
    }
    if (i == slen) {
      break;
    }

    if (s[i] != '=') {
      break;
    }
    i++;                        // skip equals

    // scan value
    while (i < slen && isWhite(s[i])) {
      i++;
    }
    if (i == slen) {
      break;
    }

    if (s[i] == '\"' || s[i] == '\'') {
      // scan quoted value
      char quote = s[i];
      iBegin = ++i;
      while (i < slen && s[i] != quote) {
        i++;
      }
    } else {
      // non quoted value
      iBegin = i;
      while (i < slen && !isWhite(s[i])) {
        i++;
      }
    }

    value.append(s + iBegin, i - iBegin);
    // append (put) to list
    add(new String(attr));
    add(new String(value));
    i++;
  }
}

String *Properties::get(const char *key) {
  for (int i = 0; i < this->_count; i++) {
    String *nextKey = (String *) _head[i++];
    if (nextKey == null || i == _count) {
      return null;
    }
    String *nextValue = (String *) _head[i];
    if (nextValue == null) {
      return null;
    }
    if (nextKey->equals(key)) {
      return nextValue;
    }
  }
  return null;
}

String *Properties::get(int i) const {
  int index = (i * 2) + 1;
  return index < _count ? (String *) _head[index] : 0;
}

String *Properties::getKey(int i) const {
  int index = i * 2;
  return index < _count ? (String *) _head[index] : 0;
}

void Properties::get(const char *key, List<String *> *arrayValues) {
  for (int i = 0; i < _count; i++) {
    String *nextKey = (String *) _head[i++];
    if (nextKey == null || i == _count) {
      break;
    }
    String *nextValue = (String *) _head[i];
    if (nextValue == null) {
      break;
    }
    if (nextKey->equals(key)) {
      arrayValues->add(new String(*nextValue));
    }
  }
}

void Properties::put(String &key, String &value) {
  String *prev = get(key.toString());
  if (prev) {
    prev->empty();
    prev->append(value);
  } else {
    add(new String(key));
    add(new String(value));
  }
}

void Properties::put(const char *key, const char *value) {
  String *prev = get(key);
  if (prev) {
    prev->empty();
    prev->append(value);
  } else {
    String *k = new String();
    String *v = new String();
    k->append(key);
    v->append(value);
    add(k);
    add(v);
  }
}

String Properties::toString() {
  String s;
  for (int i = 0; i < _count; i++) {
    String *nextKey = (String *) _head[i++];
    if (nextKey == null || nextKey->length() == 0 || i == _count) {
      break;
    }
    String *nextValue = (String *) _head[i];
    if (nextValue != null && nextValue->length() > 0) {
      s.append(nextKey->toString());
      s.append("='");
      s.append(nextValue->toString());
      s.append("'\n");
    }
  }
  return s;
}

void Properties::operator=(Properties &p) {
  removeAll();
  for (int i = 0; i < p._count; i++) {
    add(p._head[i]);
  }
  p.emptyList();
}

void List::addSet(String *s) {
  if (s && s->length()) {
    List_each(String*, it, this) {
      String *item = (*it);
      if (item->equals(s->toString())) {
        return;
      }
    }
    add(new String(*s));
  }
}

#ifdef UNIT_TEST
#include <stdio.h>
int main(int argc, char **argv) {
  String s = " test string ";
  printf("'%s'\n", s.substring(2, 6).toString());
  s.trim();
  printf("'%s'\n", s.toString());
  String r = s.replaceAll("s", "S");
  printf("'%s'\n", r.toString());
  String x = "http://blah";
  printf("starts=%d\n", x.startsWith("http://"));
  printf("starts=%d\n", x.startsWith("sshttp://"));
  List<String *> list;
  for (int j = 0; j < 5; j++) {
    for (int i = 0; i < 5; i++) {
      String *next = new String();
      next->append("hello_").append(i).append("_").append(j);
      list.add(next);
    }
    List_each(String *, it, list) {
      String *next = (*it);
      printf("next item: '%s'\n", next->toString());
    }
    list.removeAll();
  }
}
#endif
