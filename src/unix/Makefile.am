# SmallBASIC command line version
# Copyright(C) 2001-2006 Chris Warren-Smith. Gawler, South Australia
# cwarrens@twpo.com.au
#
# This program is distributed under the terms of the GPL v2.0 or later
# Download the GNU Public License (GPL) from www.gnu.org
# 

INCLUDES = -I../ -I. @PACKAGE_CFLAGS@

bin_PROGRAMS = sbasic

sbasic_SOURCES = 								\
  ../dev_ndcfb.c 								\
  ../dev_genfb.c 								\
  ../device.c 									\
  ../dev_oss.c									\
  ../unx_memmgr.c 								\
  ../dev_term.c 								\
  ../g_line.c 									\
  ../decomp.c 									\
  ../console_main.c								\
  ../interactive_mode.c							\
  ../help_subsys.c

sbasic_LDADD = @PACKAGE_LIBS@ -L$(top_srcdir)/sync -lsb_common

