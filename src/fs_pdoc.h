/*
 * PalmOS PDOC driver
 *
 * Nicholas Christopoulos
 *
 * This code is copied from txt2pdbdoc.c (Paul J. Lucas)
 *   (more info on fs_pdoc.c)
 *
 * Notes:
 * On non-PalmOS systems, names are created with prefix "pdoc_".
 * These files are non PDOC compatible... I think it will work if I add the PDOC/PDB headers.
 */

#if !defined(_fs_pdoc_h)
#define _fs_pdoc_h

#include "sys.h"
#include "device.h"

typedef struct {
  word version;                 // 2 1:plain, 2:compressed, 3:ext
  word reserved1;               // 2
  dword doc_size;               // 4 in bytes, when uncompressed
  word page_count;              // 2 numRecords-1
  word rec_size;                // 2 /* usually MAXTEXTSIZE */
  dword reserved2;              // 4
} pdoc_info_t;

/* DOC page stored inside */
typedef struct {
  byte *ptr;
  int len;
} pdoc_buf_t;

/* finaly the pdoc_t structure */
typedef struct {
  pdoc_info_t info;             // also, the header of DOCs
  pdoc_buf_t page;

#if defined(_PalmOS)
  DmOpenRef db_ref;             // file handle
#else
  int handle;
#endif
  word page_no;                 // current page index (loaded into 'page')
  word page_count;              // the number of the pages

  char *text;                   // current page text
  dword length;                 // current page text length
} pdoc_t;

//
int pdoc_lopen(const char *filename, pdoc_t * doc) SEC(BIO);
int pdoc_create_empty(const char *filename) SEC(BIO);
void pdoc_loadpage(pdoc_t * doc, int index) SEC(BIO);
void pdoc_lclose(pdoc_t * doc) SEC(BIO);
mem_t pdoc_loadtomem(const char *filename) SEC(BIO);
int pdoc_create_pdoc_from_file(const char *textfile, const char *pdocfile) SEC(BIO);
char_p_t *pdoc_create_file_list(const char *wc, int *count) SEC(BIO);

//
int pdoc_open(dev_file_t * f) SEC(BIO);
int pdoc_close(dev_file_t * f) SEC(BIO);
int pdoc_remove(const char *name) SEC(BIO);
int pdoc_exist(const char *name) SEC(BIO);
int pdoc_access(const char *name) SEC(BIO);
int pdoc_fattr(const char *name) SEC(BIO);

#endif
