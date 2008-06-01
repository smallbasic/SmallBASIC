# SmallBASIC
# Copyright(C) 2001-2008 Chris Warren-Smith. [http://tinyurl.com/ja2ss]
#
# This program is distributed under the terms of the GPL v2.0 or later
# Download the GNU Public License (GPL) from www.gnu.org
# 

# see: http://www.cygwin.com/ml/cygwin/2004-03/msg00289.html
AM_CFLAGS = @PACKAGE_CFLAGS@

INCLUDES = -DPACKAGE_LIB_DIR=\""$(pkglibdir)"\"

EXTRA_DIST =					\
    Changes					\
    keywords.c					\
    languages/chars.el.h			\
    languages/chars.en.h			\
    languages/keywords.el.c			\
    languages/keywords.en.c			\
    languages/messages.el.h			\
    languages/messages.en.h			\
    modules/configure.in			\
    modules/example1.c				\
    modules/Makefile.am				\
    modules/mod_utils.c				\
    modules/mod_utils.h				\
    modules/example1.c				\
    modules/flite/flite.c			\
    modules/flite/Makefile			\
    modules/flite/README.TXT			\
    modules/flite/test.bas			\
    modules/gdbm/gdbm.c				\
    modules/gdbm/Makefile			\
    modules/mysql/Makefile.am			\
    modules/mysql/mysql.c

noinst_LIBRARIES = libsb_common.a

libsb_common_a_SOURCES =			\
    bc.c bc.h					\
    blib.c blib.h				\
    blib_db.c					\
    blib_func.c					\
    blib_graph.c				\
    blib_math.c					\
    blib_ui.c blib_ui.h				\
    blib_math.h blib_ui.h			\
    blib_sound.c				\
    brun.c					\
    ceval.c					\
    circle.c					\
    decomp.c					\
    device.c device.h				\
    dev_genfb.h dev_term.h			\
    drvmouse.h drvsound.h			\
    eval.c					\
    extlib.c extlib.h				\
    file.c					\
    ffill.c					\
    fmt.c fmt.h					\
    fs_irda.c fs_irda.h				\
    fs_memo.c fs_memo.h				\
    fs_pdoc.c fs_pdoc.h				\
    fs_serial.c fs_serial.h			\
    fs_socket_client.c fs_socket_client.h	\
    fs_stream.c fs_stream.h			\
    g_line.c					\
    geom.c geom.h g_bmp.h			\
    inet.c inet.h				\
    kw.c kw.h					\
    match.c match.h				\
    mem.c					\
    panic.c panic.h				\
    pfill.c					\
    plot.c					\
    proc.c pproc.h				\
    sberr.c sberr.h				\
    scan.c scan.h				\
    str.c str.h					\
    tasks.c tasks.h				\
    var_uds.c var_uds.h				\
    var_hash.c var_hash.h		        \
    units.c units.h				\
    var.c var.h					\
    vmt.c vmt.h					\
    interactive_mode.h				\
    keys.h					\
    lopen_bridge.h				\
    messages.h					\
    osd.h					\
    pdb.h					\
    pmem.h					\
    sbapp.h					\
    smbas.h					\
    sys.h					\
    unx_memmgr.h

