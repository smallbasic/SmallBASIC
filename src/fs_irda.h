/*
*	IrDA I/O, driver
*
*	Nicholas Christopoulos
*/

#if !defined(_sbfs_irda_h)
#define _sbfs_irda_h

#include "sys.h"
#include "device.h"

int		irda_open(dev_file_t *f)                            SEC(BIO);
int		irda_close(dev_file_t *f)                           SEC(BIO);
int		irda_write(dev_file_t *f, byte *data, dword size)   SEC(BIO);
int		irda_read(dev_file_t *f, byte *data, dword size)    SEC(BIO);
dword	irda_length(dev_file_t *f)                          SEC(BIO);
dword	irda_eof(dev_file_t *f)                             SEC(BIO);

#endif

