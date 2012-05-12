// This file was part of EBjLib
//
// Copyright(C) 2001-2008 Chris Warren-Smith. [http://tinyurl.com/ja2ss]
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

//--Object----------------------------------------------------------------------

Object::Object() {
}
Object::~Object() {
}

//--String----------------------------------------------------------------------

String::String(const char *s) {
  owner = false;
  buffer = (char *)s;
}

String::String(const char *s, int len) {
  init();
  append(s, len);
}

String::String(const String & s) {
  init();
  append(s.buffer);
}

String::String() {
  init();
}

String::~String() {
  empty();
}

const String & String::operator=(const String & s) {
  if (this != &s) {
    empty();
    append(s.buffer);
  }
  return *this;
}

const String & String::operator=(const char *s) {
  empty();
  append(s);
  return *this;
}

const String & String::operator=(const char c) {
  empty();
  append(&c, 1);
  return *this;
}

const void String::operator+=(const String & s) {
  append(s.buffer);
}

const void String::operator+=(const char *s) {
  append(s);
}

const void String::operator+=(int i) {
  append(i);
}

const String String::operator+(const String & s) {
  String rs;
  rs.append(buffer);
  rs.append(s.buffer);
  return rs;
}

const String String::operator+(int i) {
  String rs;
  rs.append(buffer);
  rs.append(i);
  return rs;
}

const char String::operator[] (int index) {
  if (index < length()) {
    return buffer[index];
  }
  return 0;
}

const String & String::operator=(int i) {
  empty();
  append(i);
  return *this;
}

String & String::append(const String & s) {
  append(s.buffer);
  return *this;
}

String & String::append(const String *s) {
  if (s && s->length()) {
    append(s->buffer);
  }
  return *this;
}

String & String::append(int i) {
  char t[20];
  sprintf(t, "%i", i);
  append(t);
  return *this;
}

String & String::append(char c) {
  char t[2] = { c, 0 };
  append(t);
  return *this;
}

String & String::append(double d) {
  char t[20];
  sprintf(t, "%g", d);
  append(t);
  return *this;
}

String & String::append(int i, int padding) {
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

String & String::append(const char *s) {
  if (s != null && s[0] && owner) {
    int len = length();
    buffer = (char *)realloc(buffer, len + strlen(s) + 1);
    strcpy(buffer + len, s);
  }
  return *this;
}

String & String::append(const char *s, int numCopy) {
  if (!owner || s == null || numCopy < 1) {
    return *this;
  }
  int len = strlen(s);
  if (numCopy > len) {
    numCopy = len;
  }

  len = length();
  buffer = (char *)realloc(buffer, len + numCopy + 1);
  strncpy(buffer + len, s, numCopy);
  buffer[len + numCopy] = '\0';
  return *this;
}

String & String::append(FILE *fp, long filelen) {
  int len = length();
  buffer = (char *)realloc(buffer, len + filelen + 1);
  fread((void *)(len + buffer), 1, filelen, fp);
  buffer[len + filelen] = 0;
  return *this;
}

const char *String::toString() const const {
  return buffer;
}

int String::length() const const {
  return (buffer == 0 ? 0 : strlen(buffer));
}

String String::substring(int beginIndex) constconst {
  String out;
  if (beginIndex < length()) {
    out.append(buffer + beginIndex);
  }
  return out;
}

String String::substring(int beginIndex, int endIndex) constconst {
  String out;
  int len = length();
  if (endIndex > len) {
    endIndex = len;
  }
  if (beginIndex < length()) {
    out.append(buffer + beginIndex, endIndex - beginIndex);
  }
  return out;
}

void String::replaceAll(char a, char b) {
  int len = length();
  for (int i = 0; i < len; i++) {
    if (buffer[i] == a) {
      buffer[i] = b;
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
    numMatch = (buffer[i] == srch[numMatch]) ? numMatch + 1 : 0;
    if (numMatch == srchLen) {
      numCopy = 1 + i - begin - srchLen;
      if (numCopy > 0) {
        out.append(buffer + begin, numCopy);
      }
      out.append(repl);
      numMatch = 0;
      begin = i + 1;
    }
  }

  if (begin < len) {
    out.append(buffer + begin);
  }
  return out;
}

void String::toUpperCase() {
  int len = length();
  for (int i = 0; i < len; i++) {
    buffer[i] = toupper(buffer[i]);
  }
}

void String::toLowerCase() {
  int len = length();
  for (int i = 0; i < len; i++) {
    buffer[i] = tolower(buffer[i]);
  }
}

int String::toInteger() const const {
  return (buffer == 0 ? 0 : atoi(buffer));
}

double String::toNumber() const const {
  return (buffer == 0 ? 0 : atof(buffer));
}

bool String::equals(const String & s, bool ignoreCase) constconst {
  return (buffer == 0 ? s.buffer == 0 : ignoreCase ?
          strcasecmp(buffer, s.buffer) == 0 : strcmp(buffer, s.buffer) == 0);
}

bool String::equals(const char *s, bool ignoreCase) constconst {
  return (buffer == 0 ? s == 0 : ignoreCase ? strcasecmp(buffer, s) == 0 : strcmp(buffer, s) == 0);
}

bool String::startsWith(const char *s, bool ignoreCase) constconst {
  if (s == 0 || s[0] == 0) {
    return (buffer == 0 || buffer[0] == 0);
  }
  return (ignoreCase ? strncasecmp(buffer, s, strlen(s)) == 0 : strncmp(buffer, s, strlen(s)) == 0);
}

int String::indexOf(const String & s, int fromIndex) const const {
  int len = length();
  if (fromIndex >= len) {
    return -1;
  }
  if (s.length() == 1) {
    char *c = strchr(buffer + fromIndex, s.buffer[0]);
    return (c == NULL ? -1 : (c - buffer));
  } else {
    char *c = strstr(buffer + fromIndex, s.buffer);
    return (c == NULL ? -1 : (c - buffer));
  }
}

int String::indexOf(char chr, int fromIndex) const const {
  int len = length();
  if (fromIndex >= len) {
    return -1;
  }
  char *c = strchr(buffer + fromIndex, chr);
  return (c == NULL ? -1 : (c - buffer));
}

int String::lastIndexOf(char chr, int untilIndex) const const {
  int len = length();
  if (untilIndex >= len || untilIndex < 0) {
    return -1;
  }
  char *c = strrchr(buffer + untilIndex, chr);
  return (c == NULL ? -1 : (c - buffer));
}

char String::charAt(int i) const const {
  if (i < length()) {
    return buffer[i];
  }
  return 0;
}

void String::empty() {
  if (buffer != null && owner) {
    free(buffer);
  }
  buffer = 0;
  owner = true;
}

void String::trim() {
  int len = length();
  if (len == 0) {
    return;
  }
  int ibegin = 0;
  while (isWhite(buffer[ibegin])) {
    ibegin++;
  }
  int iend = len;
  while (isWhite(buffer[iend - 1])) {
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

/**
 * extract the directory path from this string and append fileName
 */
String String::getPath(const char *fileName) {
  String path;
  int i = lastIndexOf('/', 0);
  if (i != -1) {
    path = substring(0, i + 1);
    path.append(fileName);
    if (access(path.toString(), 0) == 0) {
      return path;
    }
  }
  path.empty();
  path.append(fileName);
  return path;
}

//--List------------------------------------------------------------------------

List::List(int growSize) {
  this->growSize = growSize;
  init();
}

List::~List() {
  for (int i = 0; i < count; i++) {
    delete head[i];
  }
  free(head);
  head = 0;
  count = 0;
}

void List::init() {
  count = 0;
  size = growSize;
  head = (Object **) malloc(sizeof(Object *) *size);
}

void List::removeAll() {
  for (int i = 0; i < count; i++) {
    delete head[i];
  }
  emptyList();
}

void List::emptyList() {
  free(head);
  init();
}

Object *List::operator[] (const int index)
const {
  return index < count ? head[index] : 0;
} Object *List::get(const int index) constconst {
  return index < count ? head[index] : 0;
}

void List::add(Object *object) {
  if (++count > size) {
    size += growSize;
    head = (Object **) realloc(head, sizeof(Object *) * size);
  }
  head[count - 1] = object;
}

// append unique strings
void List::addSet(String *s) {
  if (s == 0 || s->length() == 0) {
    return;
  }
  for (int i = 0; i < count; i++) {
    String *item = (String *) head[i];
    if (item->equals(s->toString())) {
      return;
    }
  }
  add(new String(*s));
}

void List::iterateInit(int ibegin /*=0*/ ) {
  iterator = ibegin;
}

bool List::hasNext() const const {
  return (iterator < count);
}

Object *List::next() {
  return head[iterator++];
}

const char **List::toArray() {
  if (length()) {
    int i = 0;
    const char **array = new const char *[length()];
    iterateInit();
    while (hasNext()) {
      array[i++] = ((String *) next())->toString();
    }
    return array;
  }
  return 0;
}

int List::compare(const void *a, const void *b) {
  String *s1 = ((String **) a)[0];
  String *s2 = ((String **) b)[0];
  return strcasecmp(s1->toString(), s2->toString());
}

void List::sort(bool desc) {
  if (count > 0) {
    qsort(head, count, sizeof(Object), compare);
  }
}

//--Stack-----------------------------------------------------------------------

Stack::Stack() : List() {
}

Stack::Stack(int growSize) : List(growSize) {
}

Object *Stack::peek() {
  if (count == 0) {
    return 0;
  }
  return head[count - 1];
}

Object *Stack::pop() {
  if (count == 0) {
    return 0;
  }
  return head[--count];
}

void Stack::push(Object *o) {
  add(o);
}

//--Properties------------------------------------------------------------------

Properties::Properties(int growSize) : List(growSize) {
}

Properties::Properties() {
}

Properties::~Properties() {
}

void Properties::load(const char *s) {
  if (s == 0 || s[0] == 0) {
    return;
  }
  load(s, strlen(s));
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
  for (int i = 0; i < count; i++) {
    String *nextKey = (String *) head[i++];
    if (nextKey == null || i == count) {
      return null;
    }
    String *nextValue = (String *) head[i];
    if (nextValue == null) {
      return null;
    }
    if (nextKey->equals(key)) {
      return nextValue;
    }
  }
  return null;
}

String *Properties::get(int i) constconst {
  int index = (i * 2) + 1;
  return index < count ? (String *) head[index] : 0;
}

String *Properties::getKey(int i) constconst {
  int index = i * 2;
  return index < count ? (String *) head[index] : 0;
}

void Properties::get(const char *key, List *arrayValues) {
  for (int i = 0; i < count; i++) {
    String *nextKey = (String *) head[i++];
    if (nextKey == null || i == count) {
      break;
    }
    String *nextValue = (String *) head[i];
    if (nextValue == null) {
      break;
    }
    if (nextKey->equals(key)) {
      arrayValues->add(new String(*nextValue));
    }
  }
}

void Properties::put(String & key, String & value) {
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
  for (int i = 0; i < count; i++) {
    String *nextKey = (String *) head[i++];
    if (nextKey == null || nextKey->length() == 0 || i == count) {
      break;
    }
    String *nextValue = (String *) head[i];
    if (nextValue != null && nextValue->length() > 0) {
      s.append(nextKey->toString());
      s.append("='");
      s.append(nextValue->toString());
      s.append("'\n");
    }
  }
  return s;
}

void Properties::operator=(Properties & p) {
  removeAll();
  for (int i = 0; i < p.count; i++) {
    add(p.head[i]);
  }
  p.emptyList();
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
}
#endif
