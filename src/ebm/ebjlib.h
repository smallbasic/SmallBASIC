/*
 * eBookman java-ish libraries
 * version 1.1
 *
 * Copyright(C) 2001 Chris Warren-Smith. Gawler, South Australia
 * cwarrens@twpo.com.au
 *
 *                  _.-_:\
 *                 /      \
 *                 \_.--*_/
 *                       v
 *
 * This program is distributed under the terms of the GPL v2.0 or later
 * Download the GNU Public License (GPL) from www.gnu.org
 * 
 */

#ifndef EBJLIB_HH
#define EBJLIB_HH

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <ctype.h>
#include <gui.h>
#include <gui_pkg.h>
#include <sys.h>

extern "C" {
#include <ebm_object.h>
#include <ereader_hostio.h>
#include <lcd.h>
}

#ifndef null
#define null 0
#endif

#ifndef max
#define max(a,b) ((a<b) ? (b) : (a))
#endif
#ifndef min
#define min(a,b) ((a>b) ? (b) : (a))
#endif

#define IDOK 1
#define IDCANCEL 2

////////////////////////////////////////////////////////////////////////////////
// class Object

struct Object {
    Object() {}
    virtual ~Object() {}
    virtual const char* className() {return "Object";}
};

////////////////////////////////////////////////////////////////////////////////
// class String

class String : public Object {
    public:
    String() {
        init();
    }

    ~String() {
        empty();
    }

    String(const char* s);
    String(const String& s);
    const String& operator=(const String& s);
    const String& operator=(const char* s);
    const void operator+=(const String& s);
    const String& operator+(const String& s);
    void append(const String& s);
    void append(int i);
    void append(int i, int padding);
    void append(const char *s);
    void append(const char *s, int numCopy);

    operator const char* () const {
        return buffer;
    }

    const char* toString() const;
    inline int length() const;
    String substring(int beginIndex) const;
    String substring(int beginIndex, int endIndex) const;
    String replace(char a, char b) const;
    String toUpperCase() const;
    String toLowerCase() const;
    String trim() const;
    int toInteger() const;
    bool equals(const String &s) const;
    bool equals(const char* s) const;
    bool startsWith(const String &s) const;
    int indexOf(const String &s, int fromIndex) const;
    void empty();
    char charAt(int i) const;
    int indexOf(char chr, int fromIndex) const;
    
    protected:
    void init() {buffer=0;owner=true;}
    char* buffer;
    bool owner;
};

////////////////////////////////////////////////////////////////////////////////
// class StringTokenizer

class StringTokenizer : String {
    public:
    StringTokenizer(const char* s) : 
        String(s), seps(" ") {
        token = strtok(buffer, seps);
    }

    StringTokenizer(const char* s, const char* delim) : 
        String(s), seps(delim) {
        token = strtok(buffer, seps);
    }

    bool hasMoreTokens() const {
        return (token != null);
    }

    String nextToken() {
        String out(token);
        token = strtok(null, seps);
        return out;
    }

    private:
    char *token;
    String seps;
};

////////////////////////////////////////////////////////////////////////////////
// class Date

class Date : public Object {
    public: 

    Date(const char* s);
    Date(String& s);
    Date();
    
    void parse(const char* s);

    int getFullYear() {
        return 1900+tmTimeNow.tm_year;
    }
    int getMonth() {
        return tmTimeNow.tm_mon;
    }
    int getTime() {
        return mktime(&tmTimeNow);
    }
    void setSeconds(int i) {
        tmTimeNow.tm_sec = i;
    }
    void setMinutes(int i) {
        tmTimeNow.tm_min = i;
    }
    void setHours(int i) {
        tmTimeNow.tm_hour = i;
    }
    void setYear(int i) {
        tmTimeNow.tm_year = i-1900;
    }

    String toString() const;

    private:
    tm tmTimeNow;
    char* dateFormat;
    void init();
};

////////////////////////////////////////////////////////////////////////////////
// class File

class File : public Object {
    public:
    File();
    ~File();

    typedef enum {readMode,writeMode,appendMode,closedMode} mode;
    bool open(const char* fileName, mode openMode);
    bool open(const char* fileName, bool readWrite);
    bool open(const char* fileName);
    void close();
    String readLine();
    bool ready();
    bool isEOF() {return eof;}
    size_t getLength();
    bool write(void* buffer, size_t length);
    bool read(void* buffer, size_t length);
    unsigned ftell() {return index;}
    bool seek(unsigned offs);
    static ebo_name_t getName(const char* fileName);
    static bool exists(const char *file);
    char* getPtr(void) {return &pMem[index];}
    bool remove();
    
    private:
    void init();
    char* pMem;
    unsigned index;
    size_t fileLength;
    ebo_name_t eboName;
    mode openMode;
    bool eof;
    bool isMMC;
    int objIndex;
};

////////////////////////////////////////////////////////////////////////////////
// class List

struct List {
    private:
    struct ListNode {
        Object* object;
        ListNode* next;
    
        ListNode(Object* object) {
            this->object = object;
            this->next  = null;
        }
    
        virtual ~ListNode() {
            if (object != null) {
                delete object;
            }
        }
    };

    void emptyAll(ListNode* head) {
        if (head == null)
            return;
        if (head->next != null) {
            emptyAll(head->next);
        }
        head->object = null;
        delete head;
    }

    int count;
    ListNode *head;
    ListNode *tail;
    ListNode *iterator;

    public:    
    List() {
        head = null;
        count=0;
    }

    virtual ~List() {
        removeAll(head);
    }

    void removeAll() {
        removeAll(head);
        head = null;
        count=0;
    }

    void removeAll(ListNode* head) {
        if (head == null)
            return;
        if (head->next != null)
            removeAll(head->next);
        delete head;
    }

    // Empties the list without deleteing the list objects
    void emptyList() {
        emptyAll(head);
        head = null;
    }

    int length() const {
        return count;
    }

    Object* operator[](const int index) const {
        ListNode *n = head;
        int i = 0;
        while (n != null && i < index) {
            n = n->next;
            i++;
        }
        return (n == null ? null : n->object);
    }

    void iterateInit() {
        iterator = head;
    }

    bool hasMoreElements() const {
        return (iterator != null && iterator->object != null);
    }

    Object* nextElement() {
        Object* o = iterator->object;
        iterator = iterator->next;
        return o;
    }

    void append(Object* object) {
        count++;
        if (head == null) {
            head = new ListNode(object);
            tail = head;
        } else {
            ListNode *next = new ListNode(object);
            tail->next = next;
            tail = next;
        }
    }
};

////////////////////////////////////////////////////////////////////////////////
// class Properties

struct Properties {
    Properties() {}
    ~Properties() {}

    void load(const char* s);
    String* get(const char* key);
    String* getKey(int i) const {return (String*)list[i*2];}
    void put(String& key, String& value);
    void removeAll() {list.removeAll();}
    private:
    List list;
};

#endif
