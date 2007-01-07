// -*- c-file-style: "java" -*-
// $Id: file.c,v 1.7 2007-01-07 03:05:50 zeeb90au Exp $
// This file is part of SmallBASIC
//
// SmallBASIC - file.c - Low-level file system support
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#include "sys.h"
#include "device.h"
#include "pproc.h"
#include "match.h"
#include "extlib.h"
#include "messages.h"
#if defined(_PalmOS)
#include <FileStream.h>
#include <SerialMgrOld.h>
#define strncasecmp(a,b,n)              StrNCaselessCompare((a),(b),(n))
#elif defined(_VTOS)
typedef FILE *FileHand;
#else
#include <errno.h>

#if defined(_UnixOS) && !defined(__MINGW32__)
#include <sys/time.h>
#include <termios.h>
#include <unistd.h>
#endif
#include <dirent.h>             // POSIX standard (Note: Borland C++
                                // compiler supports it; VC no)

typedef int FileHand;
#endif

#if defined(_WinBCB)
#include <dir.h>
#endif

// drivers
#include "fs_stream.h"
#include "fs_serial.h"
#include "fs_irda.h"
#include "fs_memo.h"
#include "fs_pdoc.h"
#include "fs_socket_client.h"

// FILE TABLE
static dev_file_t file_table[OS_FILEHANDLES];

/*
 *       Basic wild-cards 
 */
int wc_match(const char *mask, char *name)
{
    if (mask == NULL)
        return 1;
    if (*mask == '\0')
        return 1;
    if (strcmp(mask, "*") == 0)
        return 1;

    return (reg_match(mask, name) == 0);
}

/*
 *       initialize file system
 */
int dev_initfs()
{
    int i;

    for (i = 0; i < OS_FILEHANDLES; i++)
        file_table[i].handle = -1;

    // drivers initialization
    if (memo_mount() == 0) {
        rt_raise("MEMOFS DRIVER FAILED");
        return 0;
    }
    return 1;
}

/*
 *       cleanup file system
 */
void dev_closefs()
{
    int i;

    for (i = 0; i < OS_FILEHANDLES; i++) {
        if (file_table[i].handle != -1)
            dev_fclose(i + 1);
    }

    // drivers deinit
    memo_umount();
}

/*
 *       returns a free file handle for user's commands
 */
int dev_freefilehandle()
{
    int i;

    for (i = 0; i < OS_FILEHANDLES; i++) {
        if (file_table[i].handle == -1)
            return i + 1;       // Warning: BASIC's handles starting from
                                // 1
    }

    rt_raise(FSERR_TOO_MANY_FILES);
    return -1;
}

/*
 */
dev_file_t *dev_getfileptr(int handle)
{
    handle--;                   // Warning: BASIC's handles starting from
                                // 1

    if (handle < 0 || handle >= OS_FILEHANDLES) {
        rt_raise(FSERR_HANDLE);
        return NULL;
    }

    return &file_table[handle];
}

/*
 *       returns true if the file is opened
 */
int dev_fstatus(int handle)
{
    dev_file_t *f;

    if ((f = dev_getfileptr(handle)) == NULL)
        return 0;

    return (f->handle != -1);
}

#if defined(_UnixOS) && !defined(_Win32) && !defined(__MINGW32__)
/*
 *       terminal speed
 *       select the correct system constant
 */
int select_unix_serial_speed(int n)
{
    switch (n) {
    case 300:
        return B300;
    case 600:
        return B600;
    case 1200:
        return B1200;
    case 2400:
        return B2400;
    case 4800:
        return B4800;
    case 9600:
        return B9600;
    case 19200:
        return B19200;
    case 38400:
        return B38400;
    }
    return B9600;
}
#endif

/*
 *       opens a file
 *
 *       returns true on success
 */
int dev_fopen(int sb_handle, const char *name, int flags)
{
    dev_file_t *f;
    int i;

    if ((f = dev_getfileptr(sb_handle)) == NULL)
        return 0;

    memset(f, 0, sizeof(dev_file_t));

    f->handle = -1;
    f->open_flags = flags;
    f->drv_data = NULL;
    strcpy(f->name, name);

    f->type = ft_stream;

    // 
    // special devices
    // 
    if (strlen(f->name) > 4) {
        if (f->name[4] == ':') {
            for (i = 0; i < 5; i++)
                f->name[i] = to_upper(f->name[i]);

            if (strncmp(f->name, "MEMO:", 5) == 0)
                f->type = ft_memo;
            else if (strncmp(f->name, "PDOC:", 5) == 0)
                f->type = ft_pdoc;
            else if (strncmp(f->name, "COM", 3) == 0) {
                f->type = ft_serial_port;
                f->port = f->name[3] - '1';
                if (f->port < 0)
                    f->port = 10;

                if (strlen(f->name) > 5)
                    f->devspeed = xstrtol(f->name + 5);
                else
                    f->devspeed = 9600;

#if defined(_UnixOS) && !defined(_Win32) && !defined(__MINGW32__)
                f->devspeed = select_unix_serial_speed(f->devspeed);
#endif
            } else if (strncmp(f->name, "IRD", 3) == 0) {
                f->type = ft_irda_port;
                f->port = f->name[3] - '1';
                if (strlen(f->name) > 5)
                    f->devspeed = xstrtol(name + 5);
                else
                    f->devspeed = 9600;
            } else if (strncmp(f->name, "SOCL:", 5) == 0)
                f->type = ft_socket_client;
            else if (strncasecmp(f->name, "HTTP:", 5) == 0)
                f->type = ft_http_client;
            else if (strncmp(f->name, "SOUT:", 5) == 0 ||
                     strncmp(f->name, "SDIN:", 5) == 0 ||
                     strncmp(f->name, "SERR:", 5) == 0)
                f->type = ft_stream;
            else {
                // external-lib
                f->vfslib = sblmgr_getvfs(f->name);
                if (f->vfslib == -1)
                    // no such driver
                    rt_raise(FSERR_WRONG_DRIVER);
                else
                    f->type = ft_vfslib;
                return 0;
            }

        } else if (f->name[3] == ':') {
            for (i = 0; i < 4; i++)
                f->name[i] = to_upper(f->name[i]);

            if (strncmp(f->name, "CON:", 4) == 0) {
                strcpy(f->name, "SOUT:");
                f->type = ft_stream;
            } else if (strncmp(f->name, "KBD:", 4) == 0) {
                strcpy(f->name, "SDIN:");
                f->type = ft_stream;
            }
        }
    }                           // device

    // 
    // open
    // 
    switch (f->type) {
    case ft_memo:
        return memo_open(f);
    case ft_stream:
        return stream_open(f);
    case ft_pdoc:
        return pdoc_open(f);
    case ft_socket_client:
        return sockcl_open(f);
    case ft_http_client:
        return http_open(f);
    case ft_serial_port:
        return serial_open(f);
    case ft_irda_port:
        return irda_open(f);
    case ft_vfslib:
        return sblmgr_vfsexec(lib_vfs_open, f);
    default:
        err_unsup();
    };

    return 0;
}

/*
 *       returns true on success
 */
int dev_fclose(int sb_handle)
{
    dev_file_t *f;

    if ((f = dev_getfileptr(sb_handle)) == NULL)
        return 0;

    switch (f->type) {
    case ft_stream:
        return stream_close(f);
    case ft_memo:
        return memo_close(f);
    case ft_serial_port:
        return serial_close(f);
    case ft_irda_port:
        return irda_close(f);
    case ft_pdoc:
        return pdoc_close(f);
    case ft_socket_client:
    case ft_http_client:
        return sockcl_close(f);
    case ft_vfslib:
        return sblmgr_vfsexec(lib_vfs_close, f);
    default:
        err_unsup();
    }
    return 0;
}

/*
 *       returns true on success
 */
int dev_fwrite(int sb_handle, byte * data, dword size)
{
    dev_file_t *f;

    if ((f = dev_getfileptr(sb_handle)) == NULL)
        return 0;

    switch (f->type) {
    case ft_memo:
        return memo_write(f, data, size);
    case ft_stream:
    case ft_pdoc:
        return stream_write(f, data, size);
    case ft_serial_port:
        return serial_write(f, data, size);
    case ft_irda_port:
        return irda_write(f, data, size);
    case ft_socket_client:
    case ft_http_client:
        return sockcl_write(f, data, size);
    case ft_vfslib:
        return sblmgr_vfsexec(lib_vfs_write, f, data, size);
    default:
        err_unsup();
    };
    return 0;
}

/*
 *       returns true on success
 */
int dev_fread(int sb_handle, byte * data, dword size)
{
    dev_file_t *f;

    if ((f = dev_getfileptr(sb_handle)) == NULL)
        return 0;

    switch (f->type) {
    case ft_memo:
        return memo_read(f, data, size);
    case ft_stream:
    case ft_pdoc:
        return stream_read(f, data, size);
    case ft_serial_port:
        return serial_read(f, data, size);
    case ft_irda_port:
        return irda_read(f, data, size);
    case ft_socket_client:
    case ft_http_client:
        return sockcl_read(f, data, size);
    case ft_vfslib:
        return sblmgr_vfsexec(lib_vfs_read, f, data, size);
    default:
        err_unsup();
    }
    return 0;
}

/*
 */
dword dev_ftell(int sb_handle)
{
    dev_file_t *f;

    if ((f = dev_getfileptr(sb_handle)) == NULL)
        return 0;

    switch (f->type) {
    case ft_stream:
        return stream_tell(f);
    case ft_vfslib:
        return sblmgr_vfsexec(lib_vfs_tell, f);
    default:
        err_unsup();
    };
    return 0;
}

/*
 */
dword dev_flength(int sb_handle)
{
    dev_file_t *f;

    if ((f = dev_getfileptr(sb_handle)) == NULL)
        return 0;

    switch (f->type) {
    case ft_stream:
    case ft_pdoc:
        return stream_length(f);
    case ft_serial_port:
        return serial_length(f);
    case ft_irda_port:
        return irda_length(f);
    case ft_memo:
        return memo_length(f);
    case ft_socket_client:
    case ft_http_client:
        return sockcl_length(f);
    case ft_vfslib:
        return sblmgr_vfsexec(lib_vfs_length, f);
    default:
        err_unsup();
    };
    return 0;
}

/*
 */
dword dev_fseek(int sb_handle, dword offset)
{
    dev_file_t *f;

    if ((f = dev_getfileptr(sb_handle)) == NULL)
        return 0;

    switch (f->type) {
    case ft_memo:
        return memo_seek(f, offset);
    case ft_stream:
    case ft_pdoc:
        return stream_seek(f, offset);
    case ft_vfslib:
        return sblmgr_vfsexec(lib_vfs_seek, f, offset);
    default:
        err_unsup();
    };
    return -1;
}

/*
 */
int dev_feof(int sb_handle)
{
    dev_file_t *f;

    if ((f = dev_getfileptr(sb_handle)) == NULL)
        return 0;

    switch (f->type) {
    case ft_stream:
    case ft_pdoc:
        return stream_eof(f);
    case ft_memo:
        return memo_eof(f);
    case ft_serial_port:
        return serial_eof(f);
    case ft_irda_port:
        return irda_eof(f);
    case ft_socket_client:
    case ft_http_client:
        return sockcl_eof(f);
    case ft_vfslib:
        return sblmgr_vfsexec(lib_vfs_eof, f);
    default:
        err_unsup();
    };

    return 0;
}

/*
 *       deletes a file
 *       returns true on success
 */
int dev_fremove(const char *file)
{
    int success, vfslib;

    /*
     *       common for all, execute driver's function
     */
    if (strncasecmp(file, "memo:", 5) == 0)
        success = memo_delete(file);
    else if (strncasecmp(file, "pdoc:", 5) == 0)
        success = pdoc_remove(file);
    else if ((vfslib = sblmgr_getvfs(file)) != -1)
        success = sblmgr_vfsdirexec(lib_vfs_remove, vfslib, file + 5);
    else

#if defined(_PalmOS)
        success = (FileDelete(0, (char *)file) == 0);
#elif defined(_VTOS)
        success = (unlink((char *)file) != 0);
#else
        success = (remove(file) == 0);
#endif
    if (!success)
        rt_raise(FSERR_ACCESS);
    return success;
}

/*
 *       returns true if the file exists
 */
int dev_fexists(const char *file)
{
    int vfslib;

    /*
     *       common for all, execute driver's function
     */
    if (strncasecmp(file, "memo:", 5) == 0)
        return memo_exist(file);
    else if (strncasecmp(file, "pdoc:", 5) == 0)
        return pdoc_exist(file);
    else if ((vfslib = sblmgr_getvfs(file)) != -1)
        return sblmgr_vfsdirexec(lib_vfs_exist, vfslib, file + 5);

#if defined(_PalmOS)
    return (DmFindDatabase(0, (char *)file) != 0);
#elif defined(_VTOS)
    return fexists(file);
#else
    return (access(file, 0) == 0);
#endif
}

/*
 *       copy file
 *       returns true on success
 */
int dev_fcopy(const char *file, const char *newfile)
{
    int src, dst;
    byte *buf;
    dword i, block_size, block_num, remain, file_len;

    if (dev_fexists(file)) {
        if (dev_fexists(newfile)) {
            if (!dev_fremove(newfile)) {
                return 0;       // cannot delete target-file
            }
        }

        src = dev_freefilehandle();
        if (prog_error)
            return 0;
        dev_fopen(src, file, DEV_FILE_INPUT);
        if (prog_error)
            return 0;
        dst = dev_freefilehandle();
        if (prog_error)
            return 0;
        dev_fopen(dst, newfile, DEV_FILE_OUTPUT);
        if (prog_error)
            return 0;

        file_len = dev_flength(src);
        if (file_len != -1 && file_len > 0) {
            block_size = 1024;
            block_num = file_len / block_size;
            remain = file_len - (block_num * block_size);
            buf = tmp_alloc(block_size);

            for (i = 0; i < block_num; i++) {
                dev_fread(src, buf, block_size);
                if (prog_error) {
                    tmp_free(buf);
                    return 0;
                }
                dev_fwrite(dst, buf, block_size);
                if (prog_error) {
                    tmp_free(buf);
                    return 0;
                }
            }

            if (remain) {
                dev_fread(src, buf, remain);
                if (prog_error) {
                    tmp_free(buf);
                    return 0;
                }
                dev_fwrite(dst, buf, remain);
                if (prog_error) {
                    tmp_free(buf);
                    return 0;
                }
            }
            tmp_free(buf);
        }

        dev_fclose(src);
        if (prog_error)
            return 0;
        dev_fclose(dst);
        if (prog_error)
            return 0;
        return 1;
    }
    return 0;                   // source file does not exists
}

/*
 *       rename file
 *       returns true on success
 */
int dev_frename(const char *file, const char *newname)
{
    if (dev_fcopy(file, newname))
        return dev_fremove(file);
    return 0;
}

/*
 *       create a directory
 *       BUG: no drivers supported
 */
void dev_mkdir(const char *dir)
{
#if defined(_PalmOS) || defined(_VTOS)
    err_unsup();
#elif defined(_Win32) || defined(__MINGW32__)
    if (mkdir(dir) != 0)
        err_file(errno);
#else
    if (mkdir(dir, 0777) != 0)
        err_file(errno);
#endif
}

/*
 *       removes a directory
 *       BUG: no drivers supported
 */
void dev_rmdir(const char *dir)
{
#if defined(_PalmOS) || defined(_VTOS)
    err_unsup();
#else
    if (rmdir(dir) != 0)
        err_file(errno);
#endif
}

/*
 *       changes the current directory
 *       BUG: no drivers supported
 */
void dev_chdir(const char *dir)
{
#if defined(_PalmOS) || defined(_VTOS)
    err_unsup();
#else
    if (chdir(dir) != 0)
        err_file(errno);
    setsysvar_str(SYSVAR_PWD, dev_getcwd());
#endif
}

/*
 *       create a file-list using wildcards
 */
char_p_t *dev_create_file_list(const char *wc, int *count)
{
    /*
     *       common for all, execute driver's function
     */
    if (wc) {
        int vfslib;

        if (strncasecmp(wc, "MEMO:", 5) == 0)
            return memo_create_file_list(wc + 5, count);
        else if (strncasecmp(wc, "PDOC:", 5) == 0)
            return pdoc_create_file_list(wc + 5, count);
        else if ((vfslib = sblmgr_getvfs(wc)) != -1)
            return (char_p_t *) sblmgr_vfsdirexec(lib_vfs_list, vfslib, wc + 5,
                                                  count);
    }

    /*
     *       do it for "local disk"
     */
    {
#if defined(_PalmOS)
        int db_count, i;
        dword type, creator;
        LocalID LID;
        char temp[65];
        char_p_t *list;

        // ////////////
        db_count = DmNumDatabases(0);
        list = tmp_alloc(db_count * sizeof(char_p_t));
        *count = 0;

        for (i = 0; i < db_count; i++) {
            LID = DmGetDatabase(0, i);

            if (LID) {
                temp[0] = '\0';
                if (DmDatabaseInfo(0, LID,
                                   temp,
                                   NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                                   NULL, &type, &creator) == 0) {

                    if (creator == ID_SmBa && type == ID_UFST) {
                        if (wc_match(wc, temp)) {
                            list[*count] = (char *)tmp_alloc(strlen(temp) + 1);
                            strcpy(list[*count], temp);
                            *count = *count + 1;
                        }
                    }           // type
                }               // DmDatabaseInfo
            }                   // LID
        }                       // for

#elif defined(_VTOS)
        int db_count, i;
        char_p_t *list;
        unsigned char *mem, *ptr;

        ptr = mem = qmalloc(256 * 32);
        db_count = listalldbs(NULL, mem, 255);

        list = tmp_alloc(db_count * sizeof(char_p_t));
        *count = 0;

        for (i = 0; i < db_count; i++) {
            if (wc_match(wc, ptr)) {
                list[*count] = (char *)tmp_alloc(strlen(ptr) + 1);
                strcpy(list[*count], ptr);
                *count = *count + 1;
            }
            ptr += 32;
        }                       // type
        qfree(mem);
#else

        DIR *dp;
        struct dirent *e;
        char *p, wc2[OS_FILENAME_SIZE + 1], *name;
        char path[OS_PATHNAME_SIZE + 1];
        int l, size;
        char_p_t *list;

        if (wc) {
            strcpy(path, wc);
            if ((p = strrchr(path, OS_DIRSEP)) == NULL) {
                getcwd(path, OS_PATHNAME_SIZE);
                if (path[(l = strlen(path))] != OS_DIRSEP) {
                    path[l] = OS_DIRSEP;
                    path[l + 1] = '\0';
                }
                strcpy(wc2, wc);
            } else {
                strcpy(wc2, p + 1);
                *(p + 1) = '\0';
                if (strlen(wc2) == 0)
                    strcpy(wc2, "*");
            }
        } else {
            getcwd(path, OS_PATHNAME_SIZE);
            if (path[(l = strlen(path))] != OS_DIRSEP) {
                path[l] = OS_DIRSEP;
                path[l + 1] = '\0';
            }
            wc2[0] = '\0';
        }

        *count = 0;
        size = 256;
        list = tmp_alloc(sizeof(char_p_t) * size);

        if ((dp = opendir(path)) == NULL)
            return list;

        while ((e = readdir(dp)) != NULL) {
            name = e->d_name;
            if ((strcmp(name, ".") == 0) || (strcmp(name, "..") == 0))
                continue;
            if (wc_match(wc2, name)) {
                if ((*count + 1) == size) {
                    size += 256;
                    list = tmp_realloc(list, sizeof(char_p_t) * size);
                }
                list[*count] = (char *)tmp_alloc(strlen(name) + 1);
                strcpy(list[*count], name);
                *count = *count + 1;
            }
        }

        closedir(dp);
#endif

        /*
         *       common for all, if there are no files, return NULL
         */
        if (*count == 0) {
            if (list)
                tmp_free(list);
            list = NULL;
        }
        return list;
    }

    return NULL;
}

/*
 *       destroy the file-list
 */
void dev_destroy_file_list(char_p_t * list, int count)
{
    int i;

    for (i = 0; i < count; i++)
        tmp_free(list[i]);
    tmp_free(list);
}

/*
 *       returns the current directory
 *       BUG: no drivers supported
 */
char *dev_getcwd()
{
    static char retbuf[OS_PATHNAME_SIZE + 1];

#if defined(_PalmOS) || defined(_VTOS)
    *retbuf = '\0';
#else
    int l;

    getcwd(retbuf, OS_PATHNAME_SIZE);
    l = strlen(retbuf);
    if (retbuf[l - 1] != OS_DIRSEP) {
        retbuf[l] = OS_DIRSEP;
        retbuf[l + 1] = '\0';
    }
#endif
    return retbuf;
}

/*
 *       returns the file attributes
 *       1-1-1 = link - directory - regular file
 */
int dev_fattr(const char *file)
{
    int vfslib;

    /*
     *       common for all, execute driver's function
     */
    if (strncasecmp(file, "memo:", 5) == 0)
        return memo_fattr(file);
    else if (strncasecmp(file, "pdoc:", 5) == 0)
        return pdoc_fattr(file);
    else if ((vfslib = sblmgr_getvfs(file)) != -1)
        return sblmgr_vfsdirexec(lib_vfs_attr, vfslib, file + 5);

#if defined(_PalmOS) || defined(_VTOS)
    if (dev_fexists(file))
        return VFS_ATTR_FILE;
    return 0;
#else
    {
        struct stat st;
        int r = 0;

        if (stat(file, &st) == 0) {
            r |= ((S_ISREG(st.st_mode)) ? VFS_ATTR_FILE : 0);
            r |= ((S_ISDIR(st.st_mode)) ? VFS_ATTR_DIR : 0);
#if defined(_UnixOS) && !defined(__MINGW32__)
            r |= ((S_ISLNK(st.st_mode)) ? VFS_ATTR_LINK : 0);
#endif
        }
        return r;
    }
#endif
}

/*
 *       returns the access rights of the file
 */
int dev_faccess(const char *file)
{
    int vfslib;

    /*
     *       common for all, execute driver's function
     */
    if (strncasecmp(file, "memo:", 5) == 0)
        return memo_access(file);
    else if (strncasecmp(file, "pdoc:", 5) == 0)
        return pdoc_access(file);
    else if ((vfslib = sblmgr_getvfs(file)) != -1)
        return sblmgr_vfsdirexec(lib_vfs_access, vfslib, file + 5);

#if defined(_PalmOS) || defined(_VTOS)
    // TODO: if file's type is 'appl' return 0777
    return 0666;
#else
    {
        struct stat st;

        if (stat(file, &st) == 0)
            return st.st_mode;
    }
    return 0;
#endif
}
