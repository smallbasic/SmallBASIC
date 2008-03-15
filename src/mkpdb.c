/*
*	mkpdb.c	- create PDB file from binaries
*
*	Nicholas Christopoulos - 28/02/2001
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#if defined(_UnixOS)
#include <unistd.h>
#else
#if defined(__GNUC__)
#include <unistd.h>
#else
#include <stdlib.h>
#include <io.h>
#endif
#endif
#include <time.h>
#include <sys/stat.h>

#if !defined(O_BINARY)
#define	O_BINARY	0
#endif

typedef unsigned char byte;
typedef int int32;
typedef int short int16;
typedef char *char_p;

void write_i16(int handle, int16 x)
{
  byte y;

  y = x >> 8;
  write(handle, &y, 1);
  y = x & 0xFF;
  write(handle, &y, 1);
}

void write_i32(int handle, int32 x)
{
  write_i16(handle, x >> 16);
  write_i16(handle, x & 0xFFFF);
}

void change_file_ext(char *file, char *ext)
{
  char *p = strrchr(file, '.');
  char *e = ext;

  if (p)
    p++;
  if (*e == '.')
    e++;

  if (p) {
    *p = '\0';
    strcat(file, e);
  }
  else {
    strcat(file, ".");
    strcat(file, e);
  }
}

/*
*	PDB FILE HEADER
*/
struct pdb_header {
  char name[32];
  int16 attr;                   // 0x2 RO, 0x4 dirty, 0x8 backup, 0x10 ok to
  // install newer,
  // 0x20 reset after install, 0x40 dont allow copy/beam
  int16 ver;                    // app
  int32 dt_created;
  int32 dt_modified;
  int32 dt_backup;
  int32 mod_num;                // always 0 ?
  int32 app_info_pos;           // 0 = no appinf
  int32 sort_info_pos;          // 0 = no ?
  byte type[4];
  byte creator[4];
  byte uniq[4];                 // 0 ??
  int32 next;                   // 0
  int16 counter;                // record counter
};

/*
*	PDB RECORD HEADER
*/
struct pdb_rec_hdr {
  int32 offset;
  int32 index;
};


/*
*/
struct pdb_data {
  byte *data;
  int len;
};

static struct pdb_data data[256];
static int data_count;

/*
*/
int pdb_save(const char *fname, const char *creator, const char *type)
{
  int handle, offset, i, err_code = 0;
  char file_name[256], *src;
  int16 filler = 0;
  struct pdb_header head;
  struct pdb_rec_hdr rec;

  // fill header with defaults
  memset(head.name, 0, 32);
  head.attr = 0;
  head.ver = 0;
  strncpy((char *)&head.dt_created, "\x06\xD1\x44\xAE", 4);
  strncpy((char *)&head.dt_modified, "\x06\xD1\x44\xAE", 4);
  head.dt_backup = 0;
  head.mod_num = 0;
  head.app_info_pos = head.sort_info_pos = 0;
  memcpy(head.type, type, 4);
  memcpy(head.creator, creator, 4);
  memset(head.uniq, 0, 4);
  head.next = 0;
  head.counter = 0;

  // the filename
#if !defined(_UnixOS) && !defined(__GNUC__)
  src = strrchr((char *)fname, '\\');
#else
  src = strrchr((char *)fname, '/');
#endif
  if (src)
    src++;
  else
    src = (char *)fname;

  memset(file_name, 0, 32);     // debug pdb
  strcpy(file_name, src);
  src = strrchr(file_name, '.');
  if (src)
    *src = '\0';
  file_name[31] = '\0';
  strcpy(head.name, file_name);

  // create
  remove(fname);
  handle = open(fname, O_CREAT | O_BINARY | O_RDWR, S_IREAD | S_IWRITE);
  if (handle >= 0) {
    head.counter = data_count;

    // write header
    write(handle, &head.name, 32);
    write_i16(handle, head.attr);
    write_i16(handle, head.ver);
    write(handle, &head.dt_created, 4);
    write(handle, &head.dt_modified, 4);
    write_i32(handle, head.dt_backup);
    write_i32(handle, head.mod_num);
    write_i32(handle, head.app_info_pos);
    write_i32(handle, head.sort_info_pos);
    write(handle, &head.type, 4);
    write(handle, &head.creator, 4);
    write(handle, &head.uniq, 4);
    write_i32(handle, head.next);
    write_i16(handle, head.counter);

    // write record-index
    offset = sizeof(struct pdb_header) + sizeof(struct pdb_rec_hdr) * head.counter;
    for (i = 0; i < data_count; i++) {
      rec.index = ((0x40 << 24) | 0x59A000) + (i + 1);
      rec.offset = offset;

      write_i32(handle, rec.offset);
      write_i32(handle, rec.index);

      offset += data[i].len;
    }
    write_i16(handle, filler);

    // write records
    for (i = 0; i < data_count; i++)
      write(handle, data[i].data, data[i].len);

    // bye, bye
    close(handle);
  }
  else
    err_code = -1;

  // cleanup
  for (i = 0; i < data_count; i++)
    free(data[i].data);
  data_count = 0;

  return err_code;
}

/*
*/
int pdb_addfile(const char *filename)
{
  int handle;
  struct stat st;

  stat(filename, &st);
  data[data_count].len = st.st_size;
  handle = open(filename, O_BINARY | O_RDWR, S_IREAD | S_IWRITE);
  if (handle >= 0) {
    data[data_count].data = (byte *) malloc(data[data_count].len);
    read(handle, data[data_count].data, data[data_count].len);
    data_count++;
    return 0;
  }
  return -1;
}

/*
*/
void usage()
{
  fprintf(stderr, "usage: mkpdb -c CREA -t TYPE -o output.pdb files\n");
  fprintf(stderr, "-c		creator\n");
  fprintf(stderr, "-t		type\n");
  fprintf(stderr, "-o		output file\n");
}

/*
*/
int main(int argc, char *argv[])
{
  char output[1024];
  int i;
  char creator[64], type[64];

  printf("mkpdb version 0.9\n");

  strcpy(creator, "none");
  strcpy(type, "DATA");
  strcpy(output, "new.pdb");

  if (argc == 1) {
    usage();
    return 1;
  }


  data_count = 0;               // reset
  i = 1;
  while (i < argc) {
    if (argv[i][0] == '-') {
      if (argv[i][1] == 't') {
        strcpy(type, argv[i + 1]);
        i++;
      }
      else if (argv[i][1] == 'c') {
        strcpy(creator, argv[i + 1]);
        i++;
      }
      else if (argv[i][1] == 'o') {
        strcpy(output, argv[i + 1]);
        i++;
      }
      else if (argv[i][1] == 'h') {
        usage();
        return 1;
      }
    }
    else {
      if (pdb_addfile(argv[i]) != 0)
        fprintf(stderr, "error on file %s\n", argv[i]);
      else
        fprintf(stderr, "file '%s' added\n", argv[i]);
    }

    i++;
  }

  // 
  if (pdb_save(output, creator, type) != 0) {
    fprintf(stderr, "can't create file %s\n", output);
    return 1;
  }

  fprintf(stderr, "* DONE *\n");
  return 0;
}
