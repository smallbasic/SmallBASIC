#!/usr/bin/sbasic -q

const GDBM_READER = 0		' A reader
const GDBM_WRITER = 1		' A writer
const GDBM_WRCREAT = 2		' A writer.  Create the db if needed.
const GDBM_NEWDB = 3		' A writer.  Always create a new db.
const GDBM_FAST = 0x10		' Write fast! => No fsyncs.  OBSOLETE.
const GDBM_SYNC = 0x20		' Sync operations to the disk.
const GDBM_NOLOCK = 0x40    ' Don't do file locking operations.

h = loadlib("gdbm")
f = call(h, "gdbm_open", "dbtest", 512, GDBM_WRCREAT, 0o666, 0)
r = call(h, "gdbm_store", "key1", "data....", 0)
call h, "gdbm_close", f
unloadlib h




