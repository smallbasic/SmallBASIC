// This file is part of SmallBASIC
//
// Copyright(C) 2001-2014 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include "ui/strlib.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace strlib;

//--String----------------------------------------------------------------------

String::String() : _buffer(NULL) {
}

String::String(const char *s) {
  _buffer = (s == NULL ? NULL : strdup(s));
}

String::String(const String &s) {
  _buffer = strdup(s._buffer);
}

String::String(const char *s, int len) : _buffer(NULL) {
  append(s, len);
}

String::~String() {
  free(_buffer);
  _buffer = NULL;
}

const String &String::operator=(const String &s) {
  clear();
  if (_buffer != s._buffer && s._buffer != NULL) {
    _buffer = strdup(s._buffer);
  }
  return *this;
}

const String &String::operator=(const char *s) {
  clear();
  if (_buffer != s) {
    _buffer = strdup(s);
  }
  return *this;
}

const void String::operator+=(const String &s) {
  append(s._buffer);
}

const void String::operator+=(const char *s) {
  append(s);
}

String &String::append(const String &s) {
  append(s._buffer);
  return *this;
}

String &String::append(const String *s) {
  if (s && !s->empty()) {
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

String &String::append(const char *s) {
  if (s != NULL && s[0]) {
    int len = length();
    _buffer = (char *)realloc(_buffer, len + strlen(s) + 1);
    strcpy(_buffer + len, s);
  }
  return *this;
}

String &String::append(const char *s, int numCopy) {
  if (s != NULL && numCopy) {
    int len = strlen(s);
    if (numCopy > len) {
      numCopy = len;
    }
    len = length();
    _buffer = (char *)realloc(_buffer, len + numCopy + 1);
    memcpy(_buffer + len, s, numCopy);
    _buffer[len + numCopy] = '\0';
  }
  return *this;
}

String &String::append(FILE *fp, long filelen) {
  int len = length();
  _buffer = (char *)realloc(_buffer, len + filelen + 1);
  fread((void *)(len + _buffer), 1, filelen, fp);
  _buffer[len + filelen] = 0;
  return *this;
}

void String::clear() {
  free(_buffer);
  _buffer = NULL;
}

bool String::equals(const String &s, bool ignoreCase) const {
  bool result;
  if (_buffer == s._buffer) {
    result = true;
  } else if (_buffer == NULL || s._buffer == NULL) {
    result = _buffer == s._buffer;
  } else if (ignoreCase) {
    result = strcasecmp(_buffer, s._buffer) == 0;
  } else {
    result = strcmp(_buffer, s._buffer) == 0;
  }
  return result;
}

bool String::equals(const char *s, bool ignoreCase) const {
  return (_buffer == 0 ? s == 0 : ignoreCase ? 
          strcasecmp(_buffer, s) == 0 : strcmp(_buffer, s) == 0);
}

int String::indexOf(const char *s, int fromIndex) const {
  int len = length();
  if (fromIndex >= len) {
    return -1;
  }
  if (strlen(s) == 1) {
    char *c = strchr(_buffer + fromIndex, s[0]);
    return (c == NULL ? -1 : (c - _buffer));
  } else {
    char *c = strstr(_buffer + fromIndex, s);
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

String String::leftOf(char ch) const {
  int endIndex = indexOf(ch, 0);
  if (endIndex == -1) {
    return *this;
  }
  return substring(0, endIndex);
}

void String::replaceAll(char a, char b) {
  int len = length();
  for (int i = 0; i < len; i++) {
    if (_buffer[i] == a) {
      _buffer[i] = b;
    }
  }
}

String String::rightOf(char ch) const {
  int endIndex = indexOf(ch, 0);
  if (endIndex == -1) {
    return *this;
  }
  return substring(endIndex + 1, length());
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

void String::trim() {
  int len = length();
  if (len == 0) {
    return;
  }
  int ibegin = 0;
  while (IS_WHITE(_buffer[ibegin])) {
    ibegin++;
  }
  int iend = len;
  while (IS_WHITE(_buffer[iend - 1])) {
    iend--;
  }
  String s = substring(ibegin, iend);
  clear();
  append(s);
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
    attr.clear();
    value.clear();

    // remove w/s before attribute
    while (i < slen && IS_WHITE(s[i])) {
      i++;
    }
    if (i == slen) {
      break;
    }
    int iBegin = i;

    // find end of attribute
    while (i < slen && s[i] != '=' && !IS_WHITE(s[i])) {
      i++;
    }
    if (i == slen) {
      break;
    }

    attr.append(s + iBegin, i - iBegin);

    // scan for equals
    while (i < slen && IS_WHITE(s[i])) {
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
    while (i < slen && IS_WHITE(s[i])) {
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
      while (i < slen && !IS_WHITE(s[i])) {
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
    if (nextKey == NULL || i == _count) {
      return NULL;
    }
    String *nextValue = (String *) _head[i];
    if (nextValue == NULL) {
      return NULL;
    }
    if (nextKey->equals(key)) {
      return nextValue;
    }
  }
  return NULL;
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
    if (nextKey == NULL || i == _count) {
      break;
    }
    String *nextValue = (String *) _head[i];
    if (nextValue == NULL) {
      break;
    }
    if (nextKey->equals(key)) {
      arrayValues->add(new String(*nextValue));
    }
  }
}

void Properties::put(String &key, String &value) {
  String *prev = get(key.c_str());
  if (prev) {
    prev->clear();
    prev->append(value);
  } else {
    add(new String(key));
    add(new String(value));
  }
}

void Properties::put(const char *key, const char *value) {
  String *prev = get(key);
  if (prev) {
    prev->clear();
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
    if (nextKey == NULL || nextKey->empty() || i == _count) {
      break;
    }
    String *nextValue = (String *) _head[i];
    if (nextValue != NULL && !nextValue->empty()) {
      s.append(nextKey->c_str());
      s.append("='");
      s.append(nextValue->c_str());
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
  p.clear();
}
