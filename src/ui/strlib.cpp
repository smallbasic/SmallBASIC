// This file is part of SmallBASIC
//
// Copyright(C) 2001-2019 Chris Warren-Smith.
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

String::String() : _buffer(nullptr) {
}

String::String(const char *s) {
  _buffer = (s == nullptr ? nullptr : strdup(s));
}

String::String(const String &s) {
  _buffer = s._buffer == nullptr ? nullptr : strdup(s._buffer);
}

String::String(const char *s, int len) : _buffer(nullptr) {
  append(s, len);
}

String::~String() {
  free(_buffer);
  _buffer = nullptr;
}

const String &String::operator=(const String &s) {
  clear();
  if (_buffer != s._buffer && s._buffer != nullptr) {
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

void String::operator+=(const String &s) {
  append(s._buffer);
}

void String::operator+=(const char *s) {
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
  if (s != nullptr && s[0]) {
    int len = length();
    _buffer = (char *)realloc(_buffer, len + strlen(s) + 1);
    strcpy(_buffer + len, s);
  }
  return *this;
}

String &String::append(const char *s, int numCopy) {
  if (s != nullptr && numCopy) {
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
  filelen = fread((void *)(len + _buffer), 1, filelen, fp);
  _buffer[len + filelen] = 0;
  return *this;
}

void String::clear() {
  free(_buffer);
  _buffer = nullptr;
}

bool String::equals(const String &s, bool ignoreCase) const {
  bool result;
  if (_buffer == s._buffer) {
    result = true;
  } else if (_buffer == nullptr || s._buffer == nullptr) {
    result = _buffer == s._buffer;
  } else if (ignoreCase) {
    result = strcasecmp(_buffer, s._buffer) == 0;
  } else {
    result = strcmp(_buffer, s._buffer) == 0;
  }
  return result;
}

bool String::equals(const char *s, bool ignoreCase) const {
  return (_buffer == nullptr ? s == nullptr :
          s == nullptr ? _buffer == nullptr : ignoreCase ?
          strcasecmp(_buffer, s) == 0 : strcmp(_buffer, s) == 0);
}

bool String::endsWith(const String &needle) const {
  bool result;
  int len1 = _buffer == nullptr ? 0 : strlen(_buffer);
  int len2 = needle._buffer == nullptr ? 0 : strlen(needle._buffer);
  if ((len1 == 0 || len2 == 0) || len2 > len1) {
    // "cat" -> "cats"
    result = false;
  } else {
    // "needle" -> "dle"
    int fromIndex = len1 - len2;
    result = (strcmp(_buffer + fromIndex, needle._buffer) == 0);
  }
  return result;
}

int String::indexOf(const char *s, int fromIndex) const {
  int result;
  int len = length();
  if (fromIndex >= len || _buffer == nullptr) {
    result = -1;
  } else if (strlen(s) == 1) {
    char *c = strchr(_buffer + fromIndex, s[0]);
    result = (c == nullptr ? -1 : (c - _buffer));
  } else {
    char *c = strstr(_buffer + fromIndex, s);
    result = (c == nullptr ? -1 : (c - _buffer));
  }
  return result;
}

int String::indexOf(char chr, int fromIndex) const {
  int len = length();
  if (fromIndex >= len) {
    return -1;
  }
  char *c = strchr(_buffer + fromIndex, chr);
  return (c == nullptr ? -1 : (c - _buffer));
}

int String::lastIndexOf(char chr, int untilIndex) const {
  int len = length();
  if (untilIndex >= len || untilIndex < 0) {
    return -1;
  }
  char *c = strrchr(_buffer + untilIndex, chr);
  return (c == nullptr ? -1 : (c - _buffer));
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

//--List------------------------------------------------------------------

template<> void List<String *>::add(const char *s) {
  add(new String(s, strlen(s)));
}

template<> bool List<String *>::contains(const char *s) {
  bool result = false;
  for (auto next : *this) {
    if (next->equals(s)) {
      result = true;
      break;
    }
  }
  return result;
}

//--Properties------------------------------------------------------------------

template<> void Properties<String *>::load(const char *s) {
  if (s && s[0]) {
    load(s, strlen(s));
  }
}

template<> void Properties<String *>::load(const char *s, int slen) {
  if (s == nullptr || s[0] == 0 || slen == 0) {
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

template<> void Properties<String *>::put(const char *key, const char *value) {
  String *prev = get(key);
  if (prev) {
    prev->clear();
    prev->append(value);
  } else {
    add(new String(key));
    add(new String(value));
  }
}

template<> void Properties<String *>::get(const char *key, List<String *> *arrayValues) {
  for (int i = 0; i < _count; i++) {
    auto *nextKey = (String *)_head[i++];
    if (nextKey == nullptr || i == _count) {
      break;
    }
    auto *nextValue = (String *)_head[i];
    if (nextValue == nullptr) {
      break;
    }
    if (nextKey->equals(key)) {
      arrayValues->add(new String(*nextValue));
    }
  }
}

// g++ -DUNIT_TESTS=1 -I. ui/strlib.cpp && valgrind ./a.out
#if defined(UNIT_TESTS)
#include <stdio.h>
void assertEq(int a, int b) {
  if (a != b) {
    fprintf(stderr, "FAIL: %d != %d\n", a, b);
  }
}
int main() {
  String s1 = "test string is here x";
  String s2;
  String s3 = "cats";
  assertEq(0, s1.indexOf("t", 0));
  assertEq(20, s1.indexOf("x", 20));
  assertEq(5, s1.indexOf("string", 4));
  assertEq(-1, s1.indexOf("not", 10));
  assertEq(-1, s2.indexOf("not", 10));
  assertEq(0, s3.equals(nullptr, true));
  assertEq(1, s3.equals("CATS", true));
  assertEq(0, s3.equals("CATS", false));
  assertEq(1, s3.equals("cats", false));
  assertEq(1, s3.endsWith("ats"));
  assertEq(1, s3.endsWith("cats"));
  assertEq(0, s3.endsWith("morecats"));
  assertEq(1, s1.endsWith("x"));
  assertEq(1, s1.endsWith(" is here x"));
  assertEq(0, s1.endsWith(nullptr));
  assertEq('x', s1.lastChar());
  assertEq('\0', s2.lastChar());
  assertEq('s', s3.lastChar());
  return 0;
}
#endif
