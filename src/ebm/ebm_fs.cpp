// -*- c-file-style: "java" -*-
// $Id: ebm_fs.cpp,v 1.2 2004-04-12 00:21:41 zeeb90au Exp $
// This file is part of SmallBASIC
//
// Copyright(C) 2001-2004 Chris Warren-Smith. Gawler, South Australia
// cwarrens@twpo.com.au
//
/*                  _.-_:\
//                 /      \
//                 \_.--*_/
//                       v
*/
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#define FS_MODULE

#include "ebjlib.h"
#include "device.h"
#include "sberr.h"
#include <rs232.h>

EXTERN_C_BEGIN

#include "match.h"

static File* ftab[OS_FILEHANDLES];

struct ComportFile {
    ComportFile();

    int open(int handle, const char* name);
    void close();
    void send(byte *buffer, dword length);
    void read(byte *buffer, dword length);
    dword length();

    int handle;
    dword bufferLen;
    bool eof;
    byte buffer[256];
} comport;

ComportFile::ComportFile() {
    handle = -2;
    eof = true;
    bufferLen = 0;
    buffer[0] = 0;
}

void ComportFile::close() {
    if (handle != -2) {
        handle = -2;
        eof = true;
        bufferLen = 0;
        RS232_Close(RS232_PORTID_0);
    }
}

int ComportFile::open(int handle, const char* name) {
    if (this->handle != -2) {
        rt_raise("FS: RS232 PORT ALREADY OPEN");
        return 0;
    }
        
    RS232_BAUDRATE baud = RS232_BAUD_9600;
    if (strlen(name) > 5) {
        switch (atoi(name+5)) {
        case 9600: 
            baud = RS232_BAUD_9600;
            break;
        case 19200:
            baud = RS232_BAUD_19200;
            break;
        case 38400:
            baud = RS232_BAUD_38400;
            break;
        case 57600:
            baud = RS232_BAUD_57600;
            break;
        case 67187:
            baud = RS232_BAUD_67187_5;
            break;
        case 115200:
            baud = RS232_BAUD_115200;
            break;
        case 230400:
            baud = RS232_BAUD_230400;
            break;
        case 460800:
            baud = RS232_BAUD_460800;
        }
    }
    if (RS232_Open(RS232_PORTID_0, baud, 4096) == RS232_NO_ERROR) {
        this->handle = handle;
        eof = false;
        bufferLen = 0;
        return handle;
    } else {
        rt_raise("FS: RS232 PORT NOT READY");
        return 0;
    }
}

void ComportFile::send(byte *buffer, dword length) {
    if (RS232_SendBytes(RS232_PORTID_0, (U8*)buffer, length) != 
        RS232_NO_ERROR) {
        eof = true;
    }
}

void ComportFile::read(byte *buffer, dword length) {
    int maxCopy = min(length, bufferLen);
    if (bufferLen > 0) {
        memcpy(buffer, this->buffer, maxCopy);
        buffer[maxCopy] = 0;
        bufferLen -= maxCopy;
        return;
    }
    
    U16 bytesRead;
    RS232_ERRORCODE err = 
        RS232_GetBytes(RS232_PORTID_0, (U8*)buffer, length, 
                       &bytesRead, 5000);
    if (err == RS232_RX_BUFF_EMPTY) {
        buffer[0] = 0;
        return;
    }
    if (err == RS232_NO_ERROR) {
        buffer[bytesRead]= 0;
    } else {
        eof = true;
    }
}

dword ComportFile::length() {
    if (eof) {
        return 0;
    }
    
    if (bufferLen > 0) {
        return bufferLen;
    }
    
    U16 bytesRead;
    RS232_ERRORCODE err = 
        RS232_GetBytes(RS232_PORTID_0, (U8*)buffer, sizeof(buffer),
                       &bytesRead, 0);
    if (err == RS232_RX_BUFF_EMPTY) {
        bufferLen = 0;
    } else {
        bufferLen = bytesRead;
        buffer[bufferLen] = 0;
    }
    return bufferLen;
}

struct FileSystem {
    FileSystem() {
        for (int i=0; i<OS_FILEHANDLES; i++) {
            ftab[i] = null;
        }
    }
    ~FileSystem() {
        dev_closefs();
    }
} fileSystem;

File* getFile(int handle) {
    if (handle == comport.handle) {
        return null;
    }
    
    handle--; // BASIC's handles start from 1
    if (handle < 0 || handle >= OS_FILEHANDLES)  {
        rt_raise("FS: INVALID USER HANDLE");
        return null;
    }

    return ftab[handle];
}

int dev_initfs(void) {
    //env.removeAll();
    // this is called in brun after we have started using 
    // the file system methods
    return 1;
}

void dev_closefs(void) {
    for (int i = 0; i < OS_FILEHANDLES; i ++) {
        if (ftab[i] != null) {
            dev_fclose(i+1);
        }
    }

    comport.close();
}

void dev_chdir(const char *dir) {
    err_unsup();
}

void dev_mkdir(const char *dir) {
    err_unsup();
}

void dev_rmdir(const char *dir) {
    err_unsup();
}

char *dev_getcwd(void) {
    static char retbuf[OS_PATHNAME_SIZE+1];
    *retbuf = '\0';
    return retbuf;
}

int wc_match(const char *mask, char *name) {
    if  (mask == 0 || *mask == 0) {
        return 1;
    }
    if (strcmp(mask, "*") == 0) 
        return 1;

    return (reg_match(mask, name) == 0);
}

char_p_t *dev_create_file_list(const char *wc, int *count) {
    int maxSize = 256; // max list size
    char_p_t *list = (char_p_t*)tmp_alloc(sizeof(char_p_t) * maxSize);
    char name[EBO_LEN_NAME+EBO_LEN_EXT+5];
    ebo_enumerator_t ebo_enum;
    *count= 0;

    int i = ebo_first_object(&ebo_enum);
    while (*count < maxSize && i == EBO_OK) {
        strcpy(name, ebo_enum.name.name);
        strcat(name, ".");
        strcat(name, ebo_enum.name.extension);
        if (wc_match(wc, name)) {
            list[*count] = (char*)tmp_alloc(strlen(name)+1);
            strcpy(list[*count], name);
            (*count)++;
        }
        i = ebo_next_object(&ebo_enum);
    }

    // scan for MMC files
    i = ebo_first_xobject(&ebo_enum);
    while (*count < maxSize && i == EBO_OK) {
        strcpy(name, "mmc:");
        strcat(name, ebo_enum.name.name);
        strcat(name, ".");
        strcat(name, ebo_enum.name.extension);
        if (wc_match(wc, name+4)) {
            list[*count] = (char*)tmp_alloc(strlen(name)+1);
            strcpy(list[*count], name);
            (*count)++;
        }
        i = ebo_next_xobject(&ebo_enum);
    }

    if (*count == 0) {
        if (list) {
            tmp_free(list);
        }
        list = 0;
    }
    
    return list;
}

void dev_destroy_file_list(char_p_t *list, int count) {
    for (int i=0; i<count; i++) {
        tmp_free(list[i]);
    }
    tmp_free(list); 
}

// replacement for open() - this is used to open .bas files for
// reading or .sbx files for writing
int brun_fopen(const char *name, int flags) {
    int handle = -1;
    for (int i = OS_FILEHANDLES-1; i>-1; i--) {
        if (ftab[i] == null) {
            handle = i;
            break;
        }
    }

    if (handle == -1) {
        return -1;
    }

    File *f = ftab[handle] = new File;
    File::mode fileMode = File::readMode;
    if  (flags & O_CREAT) {
        fileMode = File::writeMode; // create
    }

    if (f->open(name, fileMode)) {
        return handle + 1;
    } else {
        return -1;
    }

}

int dev_fopen(int handle, const char *name, int flags) {
    if (strncmpi(name, "com1:", 5) == 0) {
        return comport.open(handle, name);
    }

    if (handle == -1) {
        // open() semantics
        for (int i = OS_FILEHANDLES-1; i>-1; i--) {
            if (ftab[i] == null) {
                handle = i;
                break;
            }
        }
        if (handle == -1) {
            rt_raise("FS: NO FREE HANDLES");
            return 0;
        }
    } else {
        handle--; 
        if (handle < 0 || handle >= OS_FILEHANDLES || ftab[handle] != null)  {
            rt_raise("FS: INVALID USER HANDLE");
            return 0;
        }
    }
    File *f = ftab[handle] = new File;
    File::mode fileMode = File::readMode;
    switch (flags) {
    case DEV_FILE_OUTPUT:
        fileMode = File::writeMode; // create
        break;
    case DEV_FILE_APPEND:  // if not exists first create; open for append
        fileMode = File::appendMode;
    }

    if (f->open(name, fileMode)) {
        return handle + 1;
    } else {
        rt_raise("FS: FAILED TO OPEN FILE");
        return -1;
    }
}

int dev_fclose(int handle) {
    if (handle == comport.handle) {
        comport.close();
        return 1;
    }
    File *f = getFile(handle);
    if (f == null) {
        return 0;
    }
    delete f;
    ftab[handle-1] = null;
    return 1;
}

int dev_fwrite(int handle, byte *buffer, dword length) {
    if (length == 0) {
        return 0;
    }
    
    if (handle == comport.handle) {
        comport.send(buffer, length);
        return 0;
    }

    File *f = getFile(handle);
    if (f == null || f->write(buffer, length) == false) {
        rt_raise("FS: FAILED TO UPDATE FILE");
    }
    return 0;
}

int dev_fread(int handle, byte *buffer, dword length) {
    if (handle == comport.handle) {
        comport.read(buffer, length);
        return 0;
    }

    File *f = getFile(handle);
    if (f == null || f->read(buffer, length) == false) {
        rt_raise("FS: FAILED TO READ FILE");
    }
    return 0;
}

void* dev_get_record(int handle, int index) {
    File *f = getFile(handle);
    if (f == null) {
        return 0;
    }
    return (f->getPtr());
}

int dev_feof(int handle) {
    if (handle == comport.handle) {
        return comport.eof;
    }

    File *f = getFile(handle);
    if (f == null) {
        return 1;
    }
    return (f->isEOF());
}

dword dev_flength(int handle) {
    if (handle == comport.handle) {
        return comport.length();
    }

    File *f = getFile(handle);
    if (f == null) {
        return 0;
    }
    return f->length();
}


dword dev_lseek(int handle, dword offset, int whence) {
    File *f = getFile(handle);
    if (f == null) {
        return 0;
    }

    dword pos = 0;
    switch (whence) {
    case SEEK_SET:
        // set file offset to offset bytes
        pos = offset;
        break;

    case SEEK_CUR:
        // set file offset to current plus offset
        pos = f->ftell() + offset;        
        break;

    case SEEK_END:
        // set file offset to EOF plus offset
        pos = f->length() + offset;
        break;
    }

    // set file offset to offset
    if (f->seek(pos) == false) {
        return 0;
    }
    return f->ftell();
}

dword dev_fseek(int handle, dword offset) {
    File *f = getFile(handle);
    if (f == null) {
        return 0;
    }
    if (f->seek(offset) == false) {
        return 0;
    }
    return f->ftell();
}

/** 
 * Returns zero to indicate file is NOT already open 
 */
int dev_fstatus(int handle) {
    if (handle == comport.handle) {
        return 1;
    }

    return (getFile(handle) == null ? 0 : 1);
}

dword dev_ftell(int handle) {
    File *f = getFile(handle);
    if (f == null) {
        return 0;
    }
    return (f->ftell());
}

int dev_stat(const char *path, struct stat *buf) {
    memset(buf, 0, sizeof(struct stat));
    File f;
    f.open(path);
    buf->st_size = f.length();
    return 0;
}

#define CHK_ERROR  if (prog_error) return 0
#define CHK_ERROR2 if (prog_error) {tmp_free(buf); return 0;}

int dev_fcopy(const char *file, const char *newfile) {
    if (dev_fexists(file)) {
        if (dev_fexists(newfile)) {
            if (!dev_fremove(newfile))
                return 0;   // cannot delete target-file
        }

        int src = dev_freefilehandle(); CHK_ERROR;
        dev_fopen(src, file, DEV_FILE_INPUT); CHK_ERROR;
        int dst = dev_freefilehandle(); CHK_ERROR;
        dev_fopen(dst, newfile, DEV_FILE_OUTPUT); CHK_ERROR;

        long file_len = dev_flength(src);
        if (file_len != -1 && file_len > 0 ) {
            dword block_size = 1024;
            dword block_num = file_len / block_size;
            dword remain = file_len - (block_num * block_size) - 1;
            byte *buf = (byte *)tmp_alloc(block_size);
            for (dword i = 0; i<block_num; i++ ) {
                dev_fread(src, buf, block_size);
                dev_fwrite(dst, buf, block_size); CHK_ERROR2;
            }
            if (remain) {
                dev_fread(src, buf, remain);  CHK_ERROR2;
                dev_fwrite(dst, buf, remain); CHK_ERROR2;
            }
            tmp_free(buf);
        }

        dev_fclose(src); CHK_ERROR;
        dev_fclose(dst); CHK_ERROR;
        return 1;
    }
    return 0;   // source file does not exists
}

/**
 *  Returns true if the file exists
 */
int dev_fexists(const char *file) {
    return File::exists(file);
}

/**
 * If the requested access is permitted, access() succeeds  and
 * returns  0. Otherwise,  -1 is returned and errno is set to
 */
int dev_access(const char *path, int amode) {
    return (File::exists(path) ? 0 : -1);
}

int dev_faccess(const char *path) {
    return (File::exists(path) ? 0 : -1);
}

/**
 *  Deletes a file
 *  returns true on success
 */
int dev_fremove(const char *file) {
    ebo_name_t name = File::getName(file);
    return (ebo_destroy(&name) == EBO_OK);
}

int dev_frename(const char *file, const char *newname) {
    ebo_name_t oldFile = File::getName(file);
    ebo_name_t newFile = File::getName(newname);
    return (ebo_rename(&oldFile, &newFile, 1) == EBO_OK);
}

/**
 *  returns a free file handle for user's commands
 */
int dev_freefilehandle() {
    for (int i=0; i<OS_FILEHANDLES; i++)  {
        if (ftab[i] == null)
            return i+1; // BASIC's handles starting from 1
    }
    
    rt_raise("FS: TOO MANY OPEN FILES");
    return -1;
}

char* getcwd(char *s, size_t i) {
    return 0;
}

int chdir(const char *s) {
    return 0;
}

int chmod(const char *s, dword mode) {
    return 0;
}

int dev_fattr(const char *s) {
    return 0;
}

FILE *popen(const char *command, const char *mode) {
    err_unsup();
    return 0;
}

int pclose(FILE *stream) {
    return 0;
}


EXTERN_C_END
