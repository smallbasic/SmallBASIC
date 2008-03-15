/*
*	PalmOS PDOC driver
*
*	Nicholas Christopoulos
*
*	Notes:
*	On non-PalmOS systems, names are created with prefix "pdoc_".
*	These files are non PDOC compatible... I think it will work if I add the PDOC/PDB headers.
*
*	This code is copied from txt2pdbdoc.c
*	The txt2pdbdoc is an excellent utility to convert from/to PDOC to/from text
*	In the same package you can also find html<->pdoc convertor. Very useful, try it.
*	>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
*	Text to Doc converter for Palm Pilots
*	txt2pdbdoc.c
*
*	Copyright (C) 1998  Paul J. Lucas (license GPL2)
*	>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
*/

#include "sys.h"
#include "device.h"
#include "pproc.h"
#include "match.h"
#if defined(_PalmOS)
#define	MIN(a,b)	( ((a)<(b))?(a):(b) )
#elif defined(_VTOS)
typedef FILE *FileHand;
#else
#include <errno.h>

#if defined(_UnixOS)
#include <sys/time.h>
#include <unistd.h>
#endif
#include <dirent.h>             // POSIX standard (Note: Borland C++ compiler
                                // supports it; VC no)

typedef int FileHand;

#include "pdb.h"
#endif
#include "fs_stream.h"
#include "fs_pdoc.h"

#define	ID_READ	0x52454164      // 'REAd'
#define	ID_TEXT	0x54455874      // 'TEXt'
#define	SID_READ	"REAd"
#define	SID_TEXT	"TEXt"

#define	PDOC_BUFFER_SIZE	6144

#define DISP_BITS 			11
#define COUNT_BITS 			3

#if !defined(F_OK)
#define F_OK  	0
#define	R_OK	4
#define	W_OK	6
#endif

/*
*	=== PDOC LIB:
*	
*	pdoc_lopen/pdoc_loadpage, pdoc_lclose
*
*	=== FS API:
*
*	pdoc_open:
*		>>> uncompress the whole file <<< to a filestream
*
*	pdoc_close:
*		>>> compress the whole file <<< from temporary filestream
*
*	Anything else is using fs_stream
*/

// some declarations
void pdoc_compress(pdoc_buf_t *) SEC(BIO);
void pdoc_uncompress(pdoc_buf_t *) SEC(BIO);
byte *pdoc_mem_find(byte * t, int t_len, byte * m, int m_len) SEC(BIO);
void pdoc_put_byte(pdoc_buf_t * b, byte c, int *space) SEC(BIO);
void pdoc_remove_binary(pdoc_buf_t * b) SEC(BIO);

/*
*	uncompress a DOC text
*	returns the new length
*/
void pdoc_uncompress(pdoc_buf_t * b)
{
  byte *const new_data = tmp_alloc(PDOC_BUFFER_SIZE);
  int i, j, di, n;
  word c;

  for (i = j = 0; i < b->len;) {
    c = b->ptr[i++];

    if (c >= 1 && c <= 8) {
      while (c--)               /* copy 'c' bytes */
        new_data[j++] = b->ptr[i++];
    }
    else if (c <= 0x7F)         /* 0,09-7F = self */
      new_data[j++] = c;

    else if (c >= 0xC0)         /* space + ASCII char */
      (new_data[j++] = ' ', new_data[j++] = c ^ 0x80);

    else {                      /* 80-BF = sequences */
      c = (c << 8) + b->ptr[i++];
      di = (c & 0x3FFF) >> COUNT_BITS;
      for (n = (c & ((1 << COUNT_BITS) - 1)) + 3; n--; ++j)
        new_data[j] = new_data[j - di];
    }
  }
  tmp_free(b->ptr);
  b->ptr = new_data;
  b->len = j;
}

/*
*/
byte *pdoc_mem_find(byte * t, int t_len, byte * m, int m_len)
{
  int i;

  for (i = t_len - m_len + 1; i > 0; --i, ++t) {
    if (*t == *m && !memcmp(t, m, m_len))
      return t;
  }
  return 0;
}

/*
*/
void pdoc_put_byte(pdoc_buf_t * b, byte c, int *space)
{
  if (*space) {
    *space = 0;
    /*
     ** There is an outstanding space char: see if we can squeeze it
     ** in with an ASCII char.
     */
    if (c >= 0x40 && c <= 0x7F) {
      b->ptr[b->len++] = c ^ 0x80;
      return;
    }
    b->ptr[b->len++] = ' ';     /* couldn't squeeze it in */
  }
  else if (c == ' ') {
    *space = 1;
    return;
  }

  if ((c >= 1 && c <= 8) || c >= 0x80)
    b->ptr[b->len++] = '\1';

  b->ptr[b->len++] = c;
}
/*
*/
void pdoc_compress(pdoc_buf_t * b)
{
  int i, j, space = 0;
  byte *buf_orig, *p, *p_prev;
  byte *head, *tail, *end;

  p = p_prev = head = buf_orig = b->ptr;
  tail = head + 1;
  end = b->ptr + b->len;

  b->ptr = tmp_alloc(PDOC_BUFFER_SIZE);
  b->len = 0;

  /*
   * loop, absorbing one more char from the input buffer on each pass 
   */
  while (head != end) {
    /*
     * establish where the scan can begin 
     */
    if (head - p_prev > ((1 << DISP_BITS) - 1))
      p_prev = head - ((1 << DISP_BITS) - 1);

    /*
     * scan in the previous data for a match 
     */
    p = pdoc_mem_find(p_prev, tail - p_prev, head, tail - head);

    /*
     * on a mismatch or end of buffer, issued codes 
     */
    if (!p || p == head || tail - head > (1 << COUNT_BITS) + 2 || tail == end) {
      /*
       * issued the codes 
       */
      /*
       * first, check for short runs 
       */
      if (tail - head < 4)
        pdoc_put_byte(b, *head++, &space);
      else {
        unsigned dist = head - p_prev;
        unsigned compound = (dist << COUNT_BITS) + tail - head - 4;

/*
				if ( dist >= ( 1 << DISP_BITS ) ||
					tail - head - 4 > 7
				)
					fprintf( stderr,
						"%s: error: dist overflow\n", me
					);
*/
        /*
         * for longer runs, issue a run-code 
         */
        /*
         * issue space char if required 
         */
        if (space) {
          b->ptr[b->len++] = ' ';
          space = 0;
        }

        b->ptr[b->len++] = 0x80 + (compound >> 8);
        b->ptr[b->len++] = compound & 0xFF;
        head = tail - 1;        /* and start again */
      }
      p_prev = buf_orig;        /* start search again */
    }
    else
      p_prev = p;               /* got a match */

    /*
     * when we get to the end of the buffer, don't inc past the 
     */
    /*
     * end; this forces the residue chars out one at a time 
     */
    if (tail != end)
      ++tail;
  }
  tmp_free(buf_orig);

  if (space)
    b->ptr[b->len++] = ' ';     /* add left-over space */

  /*
   * final scan to merge consecutive high chars together 
   */
  for (i = j = 0; i < b->len; ++i, ++j) {
    b->ptr[j] = b->ptr[i];

    /*
     * skip run-length codes 
     */
    if (b->ptr[j] >= 0x80 && b->ptr[j] < 0xC0)
      b->ptr[++j] = b->ptr[++i];

    /*
     * if we hit a high char marker, look ahead for another 
     */
    else if (b->ptr[j] == '\1') {
      b->ptr[j + 1] = b->ptr[i + 1];
      while (i + 2 < b->len && b->ptr[i + 2] == 1 && b->ptr[j] < 8) {
        b->ptr[j]++;
        b->ptr[j + b->ptr[j]] = b->ptr[i + 3];
        i += 2;
      }
      j += b->ptr[j];
      ++i;
    }
  }
  b->len = j;
}

/*
*	Read standard DOC record (4096KB compressed or not)
*/
void pdoc_loadpage(pdoc_t * doc, int index)
{
#if defined(_VTOS)
  // do nothing
#elif defined(_PalmOS)
  byte *rec_p = NULL;
  mem_t rec_h = NULL;
  int len;

  if (doc->info.page_count < index)
    panic("PDOCFS: WRONG PAGE");

  doc->page_no = index;

  rec_h = DmQueryRecord(doc->db_ref, index);
  rec_p = MemHandleLock(rec_h);

  len = MemPtrSize(rec_p);

  if (doc->page.ptr)
    tmp_free(doc->page.ptr);

  doc->page.ptr = tmp_alloc(len + 1);
  memcpy(doc->page.ptr, rec_p, len);
  doc->page.len = len;
  doc->page.ptr[len] = '\0';

  if (doc->info.version == 2)   // 2 = compressed
    pdoc_uncompress(&(doc->page));
  doc->page.ptr[doc->page.len] = '\0';

  MemHandleUnlock(rec_h);
  DmReleaseRecord(doc->db_ref, doc->page_no, 0);

  doc->length = doc->page.len;
  doc->text = doc->page.ptr;
#else
  // // UNIX /////////////////////////////////////////////////////////////
  int len;
  dword offset, next_offset;

  if (doc->info.page_count < index)
    panic("PDOCFS: WRONG PAGE");

  doc->page_no = index;

  lseek(doc->handle, PDB_DATABASE_HDR_SIZE + PDB_RECORD_ENTRY_SIZE * index,
        SEEK_SET);
  read(doc->handle, &offset, 4);
  offset = BS32(offset);

  if (index < doc->info.page_count) {
    lseek(doc->handle, PDB_DATABASE_HDR_SIZE + PDB_RECORD_ENTRY_SIZE * (index + 1),
          SEEK_SET);
    read(doc->handle, &next_offset, 4);
    next_offset = BS32(next_offset);
    len = next_offset - offset;
  }
  else {
    next_offset = lseek(doc->handle, 0L, SEEK_END);
    len = next_offset - offset; // the last record can carry garbages
  }


  lseek(doc->handle, offset, SEEK_SET);
  if (doc->page.ptr)
    tmp_free(doc->page.ptr);
  doc->page.ptr = tmp_alloc(len + 1);
  read(doc->handle, doc->page.ptr, len);
  doc->page.len = len;
  doc->page.ptr[len] = '\0';

  if (doc->info.version == 2)   // 2 = compressed
    pdoc_uncompress(&(doc->page));
  doc->page.ptr[doc->page.len] = '\0';

  doc->length = doc->page.len;
  doc->text = doc->page.ptr;
#endif
}

/*
*	opens a PDOC file
*	returns 0 on success
*			-1 file not found
*			-2 cannot open file
*			-3 out of memory
*			-4 generic read error
*			-5 signature error
*/
int pdoc_lopen(const char *filename, pdoc_t * doc)
{
#if defined(_PalmOS)
  LocalID LID;
  word maxrec;
  MemHandle rec_h;
  MemPtr rec_p;

  memset(&doc->info, 0, sizeof(pdoc_info_t));
  LID = DmFindDatabase(0, (char *)filename);
  if (LID) {
    doc->db_ref = DmOpenDatabase(0, LID, dmModeReadWrite);
    if (doc->db_ref) {
      maxrec = DmNumRecords(doc->db_ref);
      rec_h = DmQueryRecord(doc->db_ref, 0);
      if (rec_h) {
        rec_p = MemHandleLock(rec_h);
        memcpy(&doc->info, rec_p, sizeof(pdoc_info_t));
        MemPtrUnlock(rec_p);

        DmReleaseRecord(doc->db_ref, 0, 0);

        if (maxrec <= doc->info.page_count)
          doc->info.page_count = maxrec - 1;

        doc->page_count = doc->info.page_count;

        doc->page.ptr = NULL;
        doc->text = NULL;
        doc->length = 0;
        doc->page_no = 0xFFFF;  // there is nothing
      }
      else {
        DmCloseDatabase(doc->db_ref);
        return -4;
      }
    }
    else
      return -2;
  }
  else
    return -1;

#elif defined(_VTOS)
  return -1;
#else
  // // UNIX /////////////////////////////////////////////////////////////
  dword offset;
  pdb_database_hdr_t header;
//      int                     num_records;
  int h;

  memset(&doc->info, 0, sizeof(pdoc_info_t));
  h = open(filename, O_RDWR);
  if (h == -1)
    return -2;
  read(h, &header, PDB_DATABASE_HDR_SIZE);
  if ((strncmp(header.type, SID_TEXT, 4) != 0) ||
      (strncmp(header.creator, SID_READ, 4) != 0)) {
    close(h);
    return -5;
  }

//      num_records = BS32(header.record_list.num_records) - 1;
  lseek(h, PDB_DATABASE_HDR_SIZE + PDB_RECORD_ENTRY_SIZE * 0, SEEK_SET);  // RECORD 
                                                                          // 
  // 0
  read(h, &offset, 4);
  offset = BS32(offset);
  lseek(h, offset, SEEK_SET);
  read(h, &doc->info, sizeof(pdoc_info_t));

  doc->info.version = BS16(doc->info.version);
  doc->info.doc_size = BS32(doc->info.doc_size);
  doc->info.page_count = BS16(doc->info.page_count);
  doc->info.rec_size = BS16(doc->info.rec_size);

  doc->handle = h;
  doc->page.ptr = NULL;
  doc->page_count = doc->info.page_count;
  doc->text = NULL;
  doc->length = 0;
  doc->page_no = 0xFFFF;        // there is nothing
#endif
  return 0;
}

/*
*/
int pdoc_create_empty(const char *filename)
{
#if defined(_PalmOS)
  LocalID LID;
  MemHandle rec_h;
  MemPtr rec_p;
  word index;
  pdoc_t doc;

  memset(&doc.info, 0, sizeof(pdoc_info_t));
  LID = DmFindDatabase(0, (char *)filename);
  if (LID)
    DmDeleteDatabase(0, LID);

  DmCreateDatabase(0, filename, ID_READ, ID_TEXT, 0);
  LID = DmFindDatabase(0, (char *)filename);
  doc.db_ref = DmOpenDatabase(0, LID, dmModeReadWrite);

  if (!doc.db_ref)
    return -2;

  doc.info.version = 1;
  doc.info.doc_size = 0;
  doc.info.page_count = 0;
  doc.info.rec_size = 4096;

  index = dmMaxRecordIndex;
  rec_h = DmNewRecord(doc.db_ref, &index, sizeof(pdoc_info_t));
  rec_p = mem_lock(rec_h);
  DmWrite(rec_p, 0, &doc.info, sizeof(pdoc_info_t));
  mem_unlock(rec_h);
  DmReleaseRecord(doc.db_ref, index, 1);
  DmCloseDatabase(doc.db_ref);
  // /////////////////////////////////////////////////////////////////////////////////
#elif defined(_VTOS)
  return -1;
#else
  // // UNIX /////////////////////////////////////////////////////////////
  dword offset, index, tmp, date;
  pdb_database_hdr_t header;
  pdoc_t doc;
  char docname[OS_FILENAME_SIZE + 1];

  memset(&doc.info, 0, sizeof(pdoc_info_t));
  memset(header.name, 0, sizeof(header.name));

  xbasename(docname, filename);
  strncpy(header.name, docname, sizeof(header.name) - 1);
  if (strlen(docname) > sizeof(header.name) - 1)
    strncpy(header.name + sizeof(header.name) - 4, "...", 3);

  header.attributes = 0;
  header.version = 0;

  date = time(NULL) + 2082844800ul;
  date = BS32(date);
  memcpy(&header.creation_date, &date, 4);
  memcpy(&header.modification_date, &date, 4);
  header.last_backup_date = 0;
  header.modification_number = 0;
  header.app_info_id = 0;
  header.sort_info_id = 0;
  strncpy(header.type, SID_TEXT, sizeof(header.type));
  strncpy(header.creator, SID_READ, sizeof(header.creator));
  header.unique_id_seed = 0;
  header.record_list.next_record_list_id = 0;
  tmp = 1;
  header.record_list.num_records = BS32(tmp);

  if (access(filename, F_OK | W_OK) == 0)
    remove(filename);
  doc.handle = open(filename, O_CREAT | O_RDWR, 0660);
  if (doc.handle == -1)
    return -2;
  write(doc.handle, &header, PDB_DATABASE_HDR_SIZE);

        /********** write record offsets *************************************/

  offset = PDB_DATABASE_HDR_SIZE + PDB_RECORD_ENTRY_SIZE * 1 /* num of recs */ ;
  index = (0x40 << 24) | 0x6F8000;  /* dirty + unique ID */

  offset = BS32(offset);
  write(doc.handle, &offset, 4);
  tmp = BS32(index);
  write(doc.handle, &index, 4);

        /********** write record 0 *******************************************/

  doc.info.version = BS16(1);
  doc.info.doc_size = 0;
  doc.info.page_count = 0;
  doc.info.rec_size = BS16(4096);

  write(doc.handle, &doc.info, sizeof(pdoc_info_t));

  // 
  close(doc.handle);
#endif
  return 0;
}

/*
*	close PDOC file
*/
void pdoc_lclose(pdoc_t * doc)
{
#if defined(_PalmOS)
  DmCloseDatabase(doc->db_ref);
#elif defined(_VTOS)
  // No-op
#else
  close(doc->handle);
#endif
  if (doc->page.ptr)
    tmp_free(doc->page.ptr);

  memset(&doc->info, 0, sizeof(pdoc_info_t));
  doc->page_no = 0xFFFF;        // there is nothing
}

/*
*	Loads the whole file into a memory handle (returns the handle)
*/
mem_t pdoc_loadtomem(const char *filename)
{
  pdoc_t pdoc;
  int i, size = 1;
  mem_t mem;
  byte *ptr;

  if (pdoc_lopen(filename, &pdoc) != 0)
    return (mem_t) NULL;

  mem = mem_alloc(PDOC_BUFFER_SIZE);
  for (i = 1; i <= pdoc.page_count; i++) {
    pdoc_loadpage(&pdoc, i);
    size += pdoc.length;
    mem = mem_realloc(mem, size);
    ptr = mem_lock(mem);
    if (i == 0)
      strcpy(ptr, pdoc.text);
    else
      strcat(ptr, pdoc.text);
    mem_unlock(mem);
  }
  pdoc_lclose(&pdoc);
  return mem;
}

/*
*/
void pdoc_remove_binary(pdoc_buf_t * b)
{
  byte *const new_data = tmp_alloc(b->len);
  int i, j;

  for (i = j = 0; i < b->len; ++i) {
    if (b->ptr[i] < 9)          /* discard really low ASCII */
      continue;
    switch (b->ptr[i]) {

    case '\r':
      if (i < b->len - 1 && b->ptr[i + 1] == '\n')
        continue;               /* CR+LF -> LF */
      /*
       * no break; 
       */

    case '\f':
      new_data[j] = '\n';
      break;

    default:
      new_data[j] = b->ptr[i];
    }
    ++j;
  }
  tmp_free(b->ptr);
  b->ptr = new_data;
  b->len = j;
}

/*
*/
int pdoc_create_pdoc_from_file(const char *textfile, const char *pdocfile)
{
#if defined(_PalmOS)
  LocalID LID;
  MemHandle rec_h;
  MemPtr rec_p;
  word index;
  pdoc_t doc;
  int pages, rec_num;
  long file_size;
  FileHand h_in;
  Err last_error;

  h_in =
    FileOpen(0, (char *)textfile, ID_UFST, ID_SmBa,
             fileModeAnyTypeCreator | fileModeReadOnly, &last_error);
  FileTell(h_in, &file_size, &last_error);
  last_error = FileSeek(h_in, 0L, fileOriginBeginning);

  pages = file_size / 4096;
  if ((long)pages * 4096 < file_size)
    pages++;

  memset(&doc.info, 0, sizeof(pdoc_info_t));
  LID = DmFindDatabase(0, (char *)pdocfile);
  if (LID)
    DmDeleteDatabase(0, LID);

  DmCreateDatabase(0, pdocfile, ID_READ, ID_TEXT, 0);
  LID = DmFindDatabase(0, (char *)pdocfile);
  doc.db_ref = DmOpenDatabase(0, LID, dmModeReadWrite);

  if (!doc.db_ref)
    return -2;

  doc.info.version = 2;
  doc.info.doc_size = file_size;
  doc.info.page_count = pages;
  doc.info.rec_size = 4096;

  index = dmMaxRecordIndex;
  rec_h = DmNewRecord(doc.db_ref, &index, sizeof(pdoc_info_t));
  rec_p = mem_lock(rec_h);
  DmWrite(rec_p, 0, &doc.info, sizeof(pdoc_info_t));
  mem_unlock(rec_h);
  DmReleaseRecord(doc.db_ref, index, 1);

  doc.page.ptr = tmp_alloc(PDOC_BUFFER_SIZE);
  doc.page.len = PDOC_BUFFER_SIZE;

  for (rec_num = 1; rec_num <= pages; rec_num++) {
    int bytes_read;

    if (!(bytes_read = FileRead(h_in, doc.page.ptr, 1, 4096, &last_error)))
      break;
    doc.page.len = bytes_read;

    pdoc_remove_binary(&doc.page);
    pdoc_compress(&doc.page);

    index = dmMaxRecordIndex;
    rec_h = DmNewRecord(doc.db_ref, &index, doc.page.len);
    rec_p = mem_lock(rec_h);
    DmWrite(rec_p, 0, doc.page.ptr, doc.page.len);
    mem_unlock(rec_h);
    DmReleaseRecord(doc.db_ref, index, 1);
  }

  // 
  tmp_free(doc.page.ptr);
  DmCloseDatabase(doc.db_ref);
  FileClose(h_in);
  // /////////////////////////////////////////////////////////////////////////////////
#elif defined(_VTOS)
  return -2;
#else
  dword offset, index, tmp, date, file_size;
  int pages, recs, h_in, rec_num;
  pdb_database_hdr_t header;
  pdoc_t doc;
  char docname[OS_FILENAME_SIZE + 1];

  h_in = open(textfile, O_RDWR);
  file_size = lseek(h_in, 0, SEEK_END);
  lseek(h_in, 0, SEEK_SET);
  pages = file_size / 4096;
  if (((dword) pages * 4096L) < file_size)
    pages++;
  recs = pages + 1;

  memset(&doc.info, 0, sizeof(pdoc_info_t));
  memset(header.name, 0, sizeof(header.name));

  // PDB filename
  xbasename(docname, pdocfile);
  strncpy(header.name, docname, sizeof(header.name) - 1);
  if (strlen(docname) > sizeof(header.name) - 1)
    strncpy(header.name + sizeof(header.name) - 4, "...", 3);

  // build PDB header
  header.attributes = 0;
  header.version = 0;

  date = time(NULL) + 2082844800ul;
  date = BS32(date);
  memcpy(&header.creation_date, &date, 4);
  memcpy(&header.modification_date, &date, 4);
  header.last_backup_date = 0;
  header.modification_number = 0;
  header.app_info_id = 0;
  header.sort_info_id = 0;
  strncpy(header.type, SID_TEXT, sizeof(header.type));
  strncpy(header.creator, SID_READ, sizeof(header.creator));
  header.unique_id_seed = 0;
  header.record_list.next_record_list_id = 0;
  header.record_list.num_records = BS32(recs);

  if (access(pdocfile, F_OK | W_OK) == 0)
    remove(pdocfile);
  doc.handle = open(pdocfile, O_CREAT | O_RDWR, 0660);
  if (doc.handle == -1)
    return -2;
  write(doc.handle, &header, PDB_DATABASE_HDR_SIZE);

        /********** write record offsets *************************************/

  offset = PDB_DATABASE_HDR_SIZE + PDB_RECORD_ENTRY_SIZE * recs;
  index = (0x40 << 24) | 0x6F8000;  /* dirty + unique ID */

  offset = BS32(offset);
  write(doc.handle, &offset, 4);
  tmp = BS32(index);
  write(doc.handle, &tmp, 4);
  index++;

  offset = 0;
  while (--recs) {
    write(doc.handle, &offset, 4);
    tmp = BS32(index);
    write(doc.handle, &tmp, 4);
    index++;
  }

        /********** write record 0 *******************************************/

  doc.info.version = BS16(2);
  doc.info.doc_size = BS32(file_size);
  doc.info.page_count = BS16(pages);
  doc.info.rec_size = BS16(4096);

  write(doc.handle, &doc.info, sizeof(pdoc_info_t));

        /********** write text ***********************************************/

  doc.page.ptr = tmp_alloc(PDOC_BUFFER_SIZE);
  doc.page.len = PDOC_BUFFER_SIZE;

  for (rec_num = 1; rec_num <= pages; rec_num++) {
    int bytes_read;

    offset = lseek(doc.handle, 0L, SEEK_CUR);
    lseek(doc.handle, PDB_DATABASE_HDR_SIZE + PDB_RECORD_ENTRY_SIZE * rec_num,
          SEEK_SET);
    offset = BS32(offset);
    write(doc.handle, &offset, 4);
    offset = BS32(offset);

    if ((bytes_read = read(h_in, doc.page.ptr, 4096)) == 0)
      break;
    doc.page.len = bytes_read;

    pdoc_remove_binary(&doc.page);
    pdoc_compress(&doc.page);

    lseek(doc.handle, offset, SEEK_SET);
    write(doc.handle, doc.page.ptr, doc.page.len);
  }

  // 
  tmp_free(doc.page.ptr);
  close(doc.handle);
  close(h_in);
#endif
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// the filesystem API

/*
*/
int pdoc_open(dev_file_t * f)
{
#if defined(_VTOS)
  return 0;
#else
  pdoc_t doc;
  char newname[OS_PATHNAME_SIZE + 1];
  int i, file_len, rec_len;

#if !defined(_PalmOS)
  // Non-PalmOS
  strcpy(newname, f->name + 5);
#if	defined(_DOS)
  strcat(newname, ".pdc");
#else
  strcat(newname, ".pdoc");
#endif
  strcpy(f->name, newname);
  if ((access(f->name, F_OK) != 0) || (f->open_flags == DEV_FILE_OUTPUT))
    pdoc_create_empty(f->name);

#else
  // PalmOS
  memcpy(f->name, f->name + 5, strlen(f->name + 5) + 1);
  if ((DmFindDatabase(0, (char *)f->name) == 0) ||
      (f->open_flags == DEV_FILE_OUTPUT))
    pdoc_create_empty(f->name);
#endif

  switch (pdoc_lopen(f->name, &doc)) {
  case -1:
    rt_raise("PDOCFS: FILE NOT FOUND [%s]", f->name);
    return 0;
  case -2:
    rt_raise("PDOCFS: CANNOT OPEN FILE [%s]", f->name);
    return 0;
  case -4:
    rt_raise("PDOCFS: READ ERROR [%s]", f->name);
    return 0;
  case -5:
    rt_raise("PDOCFS: SIGNATURE ERROR [%s]", f->name);
    return 0;
  }

  f->drv_data = tmp_alloc(strlen(f->name) + 1);
  strcpy(f->drv_data, f->name);

#if defined(_PalmOS)
  strcpy(newname, f->name);
  if (strlen(newname) > 29) {
    newname[29] = '.';
    newname[30] = 'T';
    newname[31] = '\0';
  }
  else
    strcat(newname, ".T");
#elif defined(_UnixOS)
  chgfilename(newname, f->name, "/tmp/", NULL, NULL, ".sb.pdocfs.tmp");
#else
  chgfilename(newname, f->name, NULL, NULL, NULL, ".tmp");
#endif
  strcpy(f->name, newname);
  f->drv_dw[0] = f->open_flags;
  f->open_flags = DEV_FILE_INPUT | DEV_FILE_OUTPUT;
  if (stream_open(f) == 0) {
    tmp_free(f->drv_data);
    return 0;
  }

  // 
  file_len = 0;
  for (i = 1; i <= doc.page_count; i++) {
    pdoc_loadpage(&doc, i);

    if (i == doc.page_count) {  // remove garbages from the last page!
      rec_len = doc.info.doc_size - (doc.info.page_count - 1) * doc.info.rec_size;
      stream_write(f, doc.page.ptr, rec_len);
      file_len += rec_len;
    }
    else {
      rec_len = doc.page.len;
      stream_write(f, doc.page.ptr, rec_len);
      file_len += rec_len;
    }
  }

  pdoc_lclose(&doc);

  // set file-pointer
  if (f->drv_dw[0] & DEV_FILE_APPEND) {
    if (file_len)
      stream_seek(f, file_len);
  }
  else
    stream_seek(f, 0);
  return 1;
#endif
}

/*
*/
int pdoc_close(dev_file_t * f)
{
#if defined(_PalmOS)
  LocalID lid;
#endif

  stream_close(f);
  if ((f->drv_dw[0] & DEV_FILE_OUTPUT) || (f->drv_dw[0] & DEV_FILE_APPEND) ||
      (f->drv_dw[0] == 0))
    pdoc_create_pdoc_from_file(f->name, f->drv_data);
#if defined(_PalmOS)
  lid = DmFindDatabase(0, (char *)(f->name));
  DmDeleteDatabase(0, lid);

  // setup the backup-bit
  lid = DmFindDatabase(0, (char *)(f->drv_data));
  if (lid) {
    word attr;

    DmDatabaseInfo(0, lid, NULL, &attr, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                   NULL, NULL);
    attr |= dmHdrAttrBackup;
    DmSetDatabaseInfo(0, lid, NULL, &attr, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                      NULL, NULL);
  }

#elif defined(_VTOS)
  return 0;
#else
  remove(f->name);
#endif
  tmp_free(f->drv_data);
  f->drv_data = NULL;
  return 1;
}

/*
*/
char_p_t *pdoc_create_file_list(const char *wc, int *count)
{
#if defined(_PalmOS)
  int db_count, i;
  dword type, creator;
  LocalID LID;
  char temp[65];
  char_p_t *list;

  db_count = DmNumDatabases(0);
  list = tmp_alloc(db_count * sizeof(char_p_t));
  *count = 0;

  for (i = 0; i < db_count; i++) {
    LID = DmGetDatabase(0, i);

    if (LID) {
      temp[0] = '\0';
      if (DmDatabaseInfo(0, LID,
                         temp,
                         NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                         &type, &creator) == 0) {

        if (creator == ID_READ && type == ID_TEXT) {
          if (wc_match(wc, temp)) {
            list[*count] = (char *)tmp_alloc(strlen(temp) + 1);
            strcpy(list[*count], temp);
            *count = *count + 1;
          }
        }                       // type
      }                         // DmDatabaseInfo
    }                           // LID
  }                             // for

  return list;
  // //////////////////////////////////////////////////////////////////////////////////////
#else
  // //////////////////////////////////////////////////////////////////////////////////////
  char new_wc[OS_PATHNAME_SIZE + 1];
  char_p_t *list;
  int i;

  strcpy(new_wc, wc);
#if	defined(_DOS)
  strcat(new_wc, ".pdc");
#else
  strcat(new_wc, ".pdoc");
#endif
  list = dev_create_file_list(new_wc, count);

  // remove suffix (".pdoc")
  for (i = 0; i < *count; i++) {
    char *p;

    p = strrchr(list[i], '.');
    if (p)
      *p = '\0';
  }
  return list;
#endif
}

/*
*/
int pdoc_remove(const char *name)
{
#if defined(_PalmOS)
  LocalID lid;
  lid = DmFindDatabase(0, (char *)(name + 5));
  return (DmDeleteDatabase(0, lid) == 0);
#elif defined(_VTOS)
  return 0;
#else
  char tmp[OS_FILENAME_SIZE + 1];

  strcpy(tmp, name + 5);
#if	defined(_DOS)
  strcat(tmp, ".pdc");
#else
  strcat(tmp, ".pdoc");
#endif
  return (remove(name) == 0);
#endif
}

/*
*/
int pdoc_exist(const char *name)
{
#if defined(_PalmOS)
  return (DmFindDatabase(0, (char *)(name + 5)) != 0);
#elif defined(_VTOS)
  return 0;
#else
  char tmp[OS_FILENAME_SIZE + 1];

  strcpy(tmp, name + 5);
#if	defined(_DOS)
  strcat(tmp, ".pdc");
#else
  strcat(tmp, ".pdoc");
#endif
  return (access(name, F_OK) == 0);
#endif
}

/*
*	returns the access rights of the file
*/
int pdoc_access(const char *name)
{
#if defined(_PalmOS)
  return 0666;
#else
  char tmp[OS_FILENAME_SIZE + 1];

  strcpy(tmp, name + 5);
#if	defined(_DOS)
  strcat(tmp, ".pdc");
#else
  strcat(tmp, ".pdoc");
#endif
  return dev_faccess(tmp);
#endif
}

/*
*	returns the attributes of the file
*/
int pdoc_fattr(const char *name)
{
#if defined(_PalmOS)
  if (pdoc_exist(name))
    return 0666;
  return 0;
#elif defined(_VTOS)
  return 0;
#else
  char tmp[OS_FILENAME_SIZE + 1];

  strcpy(tmp, name + 5);
#if	defined(_DOS)
  strcat(tmp, ".pdc");
#else
  strcat(tmp, ".pdoc");
#endif
  return dev_fattr(tmp);
#endif
}
