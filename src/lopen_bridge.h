/**
 * lopen_to_fopen_bridge.c
 *
 * support open/read/write/close command set,
 * and other ANSI/POSIX commands for files.
 *
 * Nicholas Christopoulos
 */

#if !defined(_lopen_bridge_h)
#define _lopen_bridge_h

#include "sys.h"

// seek
#if !defined(SEEK_SET)
#define SEEK_SET  0
#define SEEK_CUR  1
#define SEEK_END  2
#endif

//
#if !defined(O_BINARY)
#define O_BINARY  0
#endif

// flags
#define O_RDONLY  0x1
#define O_WRONLY  0x2
#define O_RDWR    0x4
#define O_APPEND  0x8
#define O_CREAT   0x10
#define O_TRUNC   0x20
#define O_EXCL    0x40
#define O_NONBLOCK  0x80
#define O_NDELAY  0x80
#define O_NOFOLLOW  0x100
#define O_DIRECTORY 0x200
#define O_LARGEFILE 0x400

// modes
#define S_IEXEC   0100          //
#define S_IWRITE  0200          //
#define S_IREAD   0400          //

#define S_IRWXU   0700          // user (file owner) has read, write and
                                // execute permission
#define S_IRUSR   (S_IREAD)     // 0400 user has read permission
#define S_IWUSR   (S_IWRITE)    // 0200 user has write permission
#define S_IXUSR   (S_IEXEC)     // 0100 user has execute permission
#define S_IRWXG   070           // group has read, write and execute permission
#define S_IRGRP   040           // group has read permission
#define S_IWGRP   020           // group has write permission
#define S_IXGRP   010           // group has execute permission
#define S_IRWXO   07            // others have read, write and execute
                                // permission
#define S_IROTH   04            // others have read permission
#define S_IWOTH   02            // others have write permisson
#define S_IXOTH   01            // others have execute permission

int open(const char *pathname, int flags, ...) SEC(PALMFS);
int creat(const char *pathname, int mode) SEC(PALMFS);
int read(int fd, void *buf, int count) SEC(PALMFS);
int write(int fd, const void *buf, int count) SEC(PALMFS);
int close(int fd) SEC(PALMFS);
long lseek(int fildes, long offset, int whence) SEC(PALMFS);

#if defined(_PalmOS)
extern int errno;

int remove(const char *pathname) SEC(PALMFS);
int rename(const char *oldname, const char *newname) SEC(PALMFS);
#endif

#endif
