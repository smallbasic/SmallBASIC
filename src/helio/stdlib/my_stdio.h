#ifndef __MY_STDIO_H__

#define __MY_STDIO_H__

#undef ferror
#undef feof

#if !defined(NOREDEFINE)
#define FILE  MYFILE
#define fopen my_fopen
#define fseek my_fseek
#define ftell my_ftell
#define fread my_fread
#define fwrite my_fwrite
#define fgetc my_fgetc
#define fputc my_fputc
#define feof my_feof
#define fclose my_fclose
#define fsize my_fsize
#define fsize_true my_fsize_true
#define filetype my_filetype
#define unlink my_unlink
#define fgets my_fgets
#define ferror(x) FALSE
#define ungetc my_ungetc
#define fexists my_fexists
#define clock my_clock
#define time my_time
#undef CLOCKS_PER_SEC
#define CLOCKS_PER_SEC 1000
#endif

#ifndef SEEK_SET
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#endif

// Helper Functions
#define time_t unsigned long
#define __time_t_defined
unsigned long my_time(unsigned long *ptr);
#ifndef EOF
#define EOF (-1)
#endif

// Make a file list of all type==owner
int ListAllDBs(unsigned char *owner, int list_id);
// Do it but only in memory
int listalldbs(unsigned char *owner, unsigned char *listary, int maxentries);


//------- Rest is private stuff

typedef struct {
  char name[32];
  char type[5];
  int size;
  int curptr;
  int write;
  int maxwrite;
  int readonly;
  int append;
  unsigned char *bank;
  int bankoff;
  int zdoc;
  int hasungetc;
  unsigned char ungetc;
} MYFILE;

#ifdef MY_STDIO_C
#define PTR unsigned char
#else
#define PTR void
#endif

void *my_fopen(const char *name, const char *mode);
int my_fclose(MYFILE *file);
int my_fseek(MYFILE *file, int off, int origin);
int my_ftell(MYFILE *file);
int my_feof(MYFILE *fp);
int my_fread(PTR *buffer, int size, int count, MYFILE *file);
int my_fwrite(const PTR *buffer, int size, int count, MYFILE *fp);
int my_fgetc(MYFILE *file);
int my_fputc(int c, MYFILE *stream );
int my_fsize(MYFILE *file); // Uncompressed size (normal)
int my_fsize_true(MYFILE *file); // Compressed size (how much RAM does it really take?
int my_unlink(const char *filename);
int my_filetype(const char *type);
int my_ungetc(unsigned char c, MYFILE *file);
unsigned long my_clock(void);
void usleep(int us);
int my_fexists(const char *filename);
char *my_fgets(char *s, int n, MYFILE *file);

void my_cleanup();   // Close any open files/etc.

#endif
