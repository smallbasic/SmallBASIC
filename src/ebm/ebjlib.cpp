/*
 * eBookman java-ish libraries
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

#include "ebjlib.h"
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>

////////////////////////////////////////////////////////////////////////////////
// class String

String::String(const char* s)  {
    owner=false;
    buffer=(char*)s;
}

String::String(const String& s)  {
    init();
    append(s.buffer);
}

const String& String::operator=(const String& s) {
    if (this != &s) {
        empty();
        append(s.buffer);
    }
    return *this;
}

const String& String::operator=(const char* s) {
    empty();
    append(s);
    return *this;
}

const void String::operator+=(const String& s) {
    append(s.buffer);
}

const String& String::operator+(const String& s) {
    append(s.buffer);
    return *this;
}

void String::append(const String& s) {
    append(s.buffer);
}

void String::append(int i) {
    char t[20];
    sprintf(t, "%i", i);
    append(t);
}

void String::append(int i, int padding) {
    char buf[20];
    char fmt[20];
    fmt[0]='%';
    fmt[1]='0';
    padding = min(20,padding);
    sprintf(fmt+2, "%dd", padding);
    sprintf(buf, fmt, i);
    append(buf);
}

void String::append(const char *s) {
    if (s != null && owner) {
        int len = length();
        buffer = (char*)realloc(buffer, len + strlen(s) + 1);
        strcpy(buffer+len, s);
    }
}

void String::append(const char *s, int numCopy) {
    if (!owner || s == null || numCopy < 1) {
        return;
    }
    int len = strlen(s);
    if (numCopy > len) {
        numCopy = len;
    }
    
    len = length();
    buffer = (char*)realloc(buffer, len + numCopy + 1);
    strncpy(buffer+len, s, numCopy);
    buffer[len + numCopy] = '\0';
}

const char* String::toString() const {
    return buffer;
}

int String::length() const {
    return (buffer == 0 ? 0 : strlen(buffer));
}

String String::substring(int beginIndex) const {
    String out;
    if (beginIndex < length()) {
        out.append(buffer+beginIndex);
    }
    return out;
}

String String::substring(int beginIndex, int endIndex) const {
    String out;
    int len = length();
    if (endIndex > len)
        endIndex = len;
    if (beginIndex < length()) {
        out.append(buffer+beginIndex, endIndex-beginIndex);
    }
    return out;
}

String String::replace(char a, char b) const {
    String s(buffer);
    int len = s.length();
    for (int i=0; i<len; i++) {
        if (s.buffer[i]==a) {
            s.buffer[i]=b;
        }
    }
    return s;
}

String String::toUpperCase() const {
    String s(buffer);
    int len = s.length();
    for (int i=0; i<len; i++) {
        s.buffer[i] = toupper(s.buffer[i]);
    }
    return s;
}

String String::toLowerCase() const {
    String s(buffer);
    int len = s.length();
    for (int i=0; i<len; i++) {
        s.buffer[i] = tolower(s.buffer[i]);
    }
    return s;
}

int String::toInteger() const {
    return (buffer == 0 ? 0 : atoi(buffer));
}

bool String::equals(const String &s) const {
    return (strcmp(buffer, s.buffer) == 0);
}

bool String::equals(const char* s) const {
    return (strcmp(buffer, s) == 0);
}

bool String::startsWith(const String &s) const {
    return (strncmp(buffer, s.buffer, s.length()) == 0);
}

int String::indexOf(const String &s, int fromIndex) const {
    int len = length();
    if (fromIndex>=len) {
        return -1;
    }
    if (s.length()==1) {
        char* c = strchr(buffer+fromIndex, s.buffer[0]);
        return (c==NULL ? -1 : (c-buffer));
    } else {
        char* c = strstr(buffer+fromIndex, s.buffer);
        return (c==NULL ? -1 : (c-buffer));
    }
}

char String::charAt(int i) const {
    if (i<length())
        return buffer[i];
    return -1;
}

int String::indexOf(char chr, int fromIndex) const {
    int len = length();
    if (fromIndex>=len) {
        return -1;
    }
    char* c = strchr(buffer+fromIndex, chr);
    return (c==NULL ? -1 : (c-buffer));
}

void String::empty() {
    if (buffer != null && owner)
        free(buffer);
    buffer=0;
}

String String::trim() const {
    String s(buffer);
    int len = s.length();
    bool whiteSp = false;
    for (int i=0; i<len; i++) {
        if (s.buffer[i] == '\r' ||
            s.buffer[i] == '\n' ||
            s.buffer[i] == '\t') {
            s.buffer[i] = ' ';
            whiteSp = true;
        } else if (i<len-1 &&
            s.buffer[i] == ' ' && 
            s.buffer[i+1] == ' ') {
            whiteSp = true;
        }
    }
    String b(s.buffer);
    if (whiteSp) {
        int iAppend=0;
        for (int i=0; i<len; i++) {
            if (i<len-1 &&
                s.buffer[i] == ' ' && 
                s.buffer[i+1] == ' ') {
                continue;
            }
            b.buffer[iAppend++] = s.buffer[i];
        }
        b.buffer[iAppend] = '\0';
    }
    return b;
}

////////////////////////////////////////////////////////////////////////////////
// class Date

Date::Date(const char* s) {
    init();
    parse(s);
}

Date::Date(String& s) {
    init();
    parse(s);
}

Date::Date() {
    init();
    time_t ttTimeNow;
    time(&ttTimeNow);
    tm* ptm = localtime(&ttTimeNow);
    tmTimeNow.tm_mday = ptm->tm_mday;
    tmTimeNow.tm_mon = ptm->tm_mon;
    tmTimeNow.tm_year = ptm->tm_year;
}

/** 
 * Parses a date like 28 Nov 1973
 */
void Date::parse(const char* s) {
    init();
    StringTokenizer st(s);
    int day = st.nextToken().toInteger();
    String month = st.nextToken().toLowerCase();
    int year = st.nextToken().toInteger();
    tmTimeNow.tm_mday = day;
    tmTimeNow.tm_year = year-1900;
    char months[12][4] = {"jan", "feb", "mar", "apr", "may", "jun", 
                          "jul", "aug", "sep", "oct", "nov", "dec"};
    for (int i=0; i<12; i++) {
        if (month.equals(months[i])) {
            tmTimeNow.tm_mon = i;
            break;
        }
    }
}

void Date::init() {
    dateFormat = sysGetDateFormat();
    tmTimeNow.tm_sec=0;
    tmTimeNow.tm_min=0;
    tmTimeNow.tm_hour=0;
    tmTimeNow.tm_mday=0;
    tmTimeNow.tm_mon=0;
    tmTimeNow.tm_year=0;
    tmTimeNow.tm_wday=0;
    tmTimeNow.tm_yday=0;
    tmTimeNow.tm_isdst=0;
}

String Date::toString() const {
    char buffer[40];
    strftime(&buffer[0], sizeof(buffer), dateFormat, &tmTimeNow);
    String s(buffer);
    return s;
}

////////////////////////////////////////////////////////////////////////////////
// class File

File::File() {
    init();
}

File::~File() {
    close();
}

void File::init() {
    pMem = 0;
    index = 0;
    fileLength = 0;
    eof = false;
    isMMC = false;
    objIndex = 0;
    openMode = closedMode;
}

void File::close() {
    if (openMode == closedMode) {
        return;
    }

    if (pMem) {
        ebo_unmap(pMem, fileLength);
        pMem = 0;
    }

#if 0
    // MMC write still not supported in EBM-OS-2.01 ;-()
    if (isMMC && openMode != readMode) {
        // Copy the saved RAM file to MMC

        // Copy the temp file to the mmc file
        int i = strlen(eboName.extension);
        if (i>=4) {
            eboName.extension[i-4] = 0;
        }
        int i;
        char fname[128];
        strcpy(fname, "!t!txt");
        ebo_name_t ebon = getName(fname);

        i = ebo_xdir_copy_memobj_to_file(-1, &ebon, 
                                         EBO_XDIRNUM_VIRTUAL,
                                         fname, 128);
        f.remove();
    }
        // write to a temporary RAM file - copied to mmc in close()
        // strcat(eboName.extension, "_tmp");

#endif

    openMode = closedMode;
}

bool File::open(const char* fileName) {
    return open(fileName, readMode);
}

bool File::open(const char* fileName, bool readWrite) {
    return open(fileName, readWrite ? writeMode : readMode);
}

bool File::open(const char* fileName, mode openMode) {
    ebo_enumerator_t ee;
    this->eof = false;
    this->openMode = closedMode;

    // open mmc file
    if (strncasecmp("mmc:", fileName, 4) == 0) {
        eboName = getName(fileName+4);
        isMMC = true;
        if (openMode == readMode) {
            if (ebo_locate_xobject(&eboName, &ee) == EBO_OK) {
                this->fileLength = ee.size;
                this->objIndex = ee.index;
                this->openMode = readMode;
                return true;
            } else {
                return false;
            }
        }

        // write not yet supported (by the ebm-os)
        return false;

    } else {
        // open ram file
        isMMC = false;
        eboName = getName(fileName);
    }

    if (openMode == writeMode || openMode == appendMode) {
        fileLength = EBO_BLK_SIZE;
        if (openMode == writeMode || // append mode and doesn't exist
            ebo_locate_object(&eboName, &ee) != EBO_OK) {
            if (ebo_new(&eboName, fileLength, 1) <0) {
                return false; // failed to create 1 block file
            }
        }
        pMem = (char*)OS_availaddr;
        OS_availaddr += 131072; // max filesize is 32 blocks := 128KB
        if (ebo_mapin(&eboName, 0, pMem, &fileLength, 1) >=0) {
            this->openMode = openMode;
            return true;
        }
        return false;
    } else {
        // open readonly
        if (ebo_locate_object(&eboName, &ee) != EBO_OK) {
            return false;
        }

        pMem = (char*)ebo_roundup(OS_availaddr);
        fileLength = EBO_MAX_SIZE; 
        int i = ebo_mapin(&eboName, 0, pMem, &fileLength, 0);
        assert(i != EBO_UNALIGNED);
        assert(i != EBO_UNIMPLEMENTED);
        OS_availaddr += fileLength;
        this->openMode = readMode;
        return true;
    }
}

bool File::read(void* buffer, size_t length) {
    if (openMode != readMode || eof == true) {
        return false;
    }

    if (length > fileLength-index) {
        length = fileLength-index; // trim to remaining
    }

    if (isMMC) {
        long iread = ebo_iread(objIndex, buffer, index, length);
        if (iread < EBO_OK) {
            eof = true;
            return false;
        }
        index += iread;
        eof = (index >= fileLength);
        return true;
    }

    if (pMem == null) {
        eof = true;
        return false;
    }

    memcpy(buffer, pMem+index, length);
    index += length;
    eof = (index >= fileLength);
    return true;
}

bool File::write(void* buffer, size_t length) {
    if (openMode != writeMode && openMode != appendMode) {
        return false;
    }

    if (index+length > fileLength) {
        // grow file 
        fileLength += ebo_roundup(index+length - fileLength);
        if (ebo_extend(&eboName, fileLength) < 0 ||
            ebo_mapin(&eboName, 0, pMem, &fileLength, 1) < 0) {
            return false;
        }
    }
    memcpy(pMem+index, buffer, length);
    index += length;
    return true;
}

bool File::remove() {
    close();
    return (EBO_OK == ebo_destroy(&eboName));
}

String File::readLine() {
    assert(openMode == readMode);
    String out;
    if (pMem == null || fileLength == 0) {
        return out;
    }
    
    unsigned iBegin = index;
    while (pMem[index] != '\0') {
        if (pMem[index] == '\n') {
            if (iBegin == index) {
                return out;
            }
            int i = (index>0 && pMem[index-1] == '\r' ? index-1:index);
            out.append(pMem+iBegin, i-iBegin);
            while (pMem[index] != '\0' && pMem[index] == '\n') {
                index++;
            }
            return out;
        }
        index++;
    }
    return out;
}

bool File::seek(unsigned pos) {
    if (openMode == closedMode) {
        return false;
    }
    if (pos < 0 || pos > fileLength) {
        return false;
    }
    index = pos;
    return true;
}

bool File::ready() {
    return (pMem[index] != '\0');
}

ebo_name_t File::getName(const char* fileName) {
    ebo_name_t name;
    name.secure_type[0] = 0;
    name.publisher[0] = 0;
    char *pos = strchr(fileName, '.');
    if (pos != null) {
        strcpy(name.extension, pos+1);
        strncpy(name.name, fileName, pos-fileName);
        name.name[pos-fileName] = '\0';
    } else {
        name.extension[0] = 0;
        strcpy(name.name, fileName);
    }
    if (hostIO_is_simulator()) {
        // for initial.mom
        strcpy(name.publisher, "Developer");
    }
    return name;
}

size_t File::getLength() {
    if (openMode == closedMode) {
        return 0;
    }
    if (hostIO_is_simulator()) {
        String fileName;
        fileName.append(eboName.name);
        fileName.append(".");
        fileName.append(eboName.extension);
        FILE* f = fopen(fileName, "r");
        if (f != null) {
            fseek(f, 0, SEEK_END);
            fileLength = ::ftell(f);
            fclose(f);
        } else {
            fileLength = 0;
        }
    }
    return fileLength;
}

bool File::exists(const char *fileName) { 
    if (hostIO_is_simulator()) {
        FILE* f = fopen(fileName, "r");
        if (f != null) {
            fclose(f);
            return true;
        }
        return false;
    }

    ebo_enumerator_t ee;
    if (strncasecmp("mmc:", fileName, 4) == 0) {
        ebo_name_t name = getName(fileName+4);
        return (ebo_locate_xobject(&name, &ee) == EBO_OK);
    }

    ebo_name_t name = getName(fileName);
    return (ebo_locate_object(&name, &ee) == EBO_OK);
}

////////////////////////////////////////////////////////////////////////////////
// class Properties

#define isWhite(c) (c == ' '|| c == '\n' || c == '\r' || c == '\t')

void Properties::load(const char *s) {

    int slen = strlen(s);
    int i=0;
    while (i<slen) {
        String attr;
        String value;

        // remove w/s before attribute
        while (i<slen && isWhite(s[i])) {
            i++;
        }
        if (i == slen) {
            break;
        }
        int iBegin = i;

        // find end of attribute
        while (i<slen && s[i] != '=' && !isWhite(s[i])) {
            i++;
        }
        if (i == slen) {
            break;
        }

        attr.append(s+iBegin, i-iBegin);

        // scan for equals 
        while (i<slen && isWhite(s[i])) {
            i++;
        }
        if (i == slen) {
            break;
        }

        if (s[i] != '=') {
            break;
        }
        i++; // skip equals

        // scan value
        while (i<slen && isWhite(s[i])) {
            i++;
        }
        if (i == slen) {
            break;
        }

        if (s[i] == '\"' || s[i] == '\'') {
            // scan quoted value
            char quote = s[i];
            iBegin = ++i;
            while (i<slen && s[i] != quote) {
                i++;
            }
        } else {
            // non quoted value
            iBegin = i;
            while (i<slen && !isWhite(s[i])) {
                i++;
            }
        }

        value.append(s+iBegin, i-iBegin);
        put(attr, value);
        i++;
    }
}

String* Properties::get(const char* key) {
    list.iterateInit();
    while (list.hasMoreElements()) {
        String *nextKey = (String*)list.nextElement();
        if (nextKey == null || list.hasMoreElements()==false) {
            return null;
        }
        String *nextValue = (String*)list.nextElement();
        if (nextValue == null) {
            return null;
        }
        if (nextKey->equals(key)) {
            return nextValue;
        }
    }
    return null;
}

void Properties::put(String& key, String& value) {
    list.append(new String(key));
    list.append(new String(value));
}
