/**
 * -*- c-file-style: "java" -*-
 * SmallBASIC for eBookMan
 * Copyright(C) 2001-2002 Chris Warren-Smith. Gawler, South Australia
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

#define FS_MODULE

#include "ebjlib.h"
// #include "sys.h"
#include "device.h"

EXTERN_C_BEGIN

#include "match.h"

static File* ftab[OS_FILEHANDLES];
static Properties env;
extern int prog_error;

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
    handle--; // BASIC's handles starting from 1
    if (handle < 0 || handle >= OS_FILEHANDLES)  {
        rt_raise("FS: INVALID USER HANDLE");
        return null;
    }

    return ftab[handle];
}

int dev_initfs(void) {
    env.removeAll();
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

int dev_fopen(int handle, const char *name, int flags) {
    if (handle == -1) {
        // open() semantics
        for (int i = 0; i<OS_FILEHANDLES; i++) {
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
    switch(flags) {
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
    File *f = getFile(handle);
    if (f == null) {
        return 0;
    }
    delete f;
    ftab[handle-1] = null;
    return 1;
}

int dev_fwrite(int handle, byte *buffer, dword length) {
    File *f = getFile(handle);
    if (f == null || f->write(buffer, length) == false) {
        rt_raise("FS: FAILED TO UPDATE FILE");
    }
    return 0;
}

int dev_fread(int handle, byte *buffer, dword length) {
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
    File *f = getFile(handle);
    if (f == null) {
        return 1;
    }
    return (f->isEOF());
}

dword dev_flength(int handle) {
    File *f = getFile(handle);
    if (f == null) {
        return 0;
    }
    return f->getLength();
}

dword dev_fseek(int handle, dword offset) {
    File *f = getFile(handle);
    if (f == null) {
        return 0;
    }
    return (f->seek(offset));
}

/** 
 * Returns zero to indicate file is NOT already open 
 */
int dev_fstatus(int handle) {
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
    buf->st_size = f.getLength();
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

int putenv(const char *s) {
    Properties p;
    p.load(s);
    String* key = p.getKey(0);
    if (key == null) {
        return 0;
    }

    String* value = env.get(key->toString());
    if (value != null) {
        // property already exists
        String* newValue = p.get(key->toString());
        value->empty();
        value->append(newValue->toString());
    } else {
        // new property
        env.load(s);
    }
    return 1;
}

char* getenv(const char *s) {
    String* str = env.get(s);
    return (str ? (char*)str->toString() : null);
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
