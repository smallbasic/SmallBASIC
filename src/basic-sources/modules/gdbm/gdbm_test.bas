#!/usr/bin/sbasic -m -q

import gdbm

' Parameters to gdbm_open for READERS, WRITERS, and WRITERS who
' can create the database.
const GDBM_READER = 0		' A reader
const GDBM_WRITER = 1		' A writer
const GDBM_WRCREAT = 2		' A writer.  Create the db if needed.
const GDBM_NEWDB = 3		' A writer.  Always create a new db.
const GDBM_FAST = 0x10		' Write fast! => No fsyncs.  OBSOLETE.
const GDBM_SYNC = 0x20		' Sync operations to the disk.
const GDBM_NOLOCK = 0x40    ' Don't do file locking operations.

' Parameters to gdbm_store for simple insertion or replacement in the
' case that the key is already in the database.
const GDBM_INSERT = 0		' Never replace old data with new.
const GDBM_REPLACE = 1		' Always replace old data with new.

' Parameters to gdbm_setopt, specifing the type of operation to perform.
const GDBM_CACHESIZE = 1	' Set the cache size.
const GDBM_FASTMODE = 2		' Toggle fast mode.  OBSOLETE.
const GDBM_SYNCMODE = 3		' Turn on or off sync operations.
const GDBM_CENTFREE = 4		' Keep all free blocks in the header.
const GDBM_COALESCEBLKS = 5	' Attempt to coalesce free blocks.

' TEST
h = gdbm.open("dbtest.db", 512, GDBM_WRCREAT, 0o666)
? "Handle = "; h
? "Store returns = "; gdbm.store(h, "key1", "data1....")
? "Store returns = "; gdbm.store(h, "key2", "data2....")
? "Fetch returns = "; gdbm.fetch(h, "key1")
gdbm.close h






