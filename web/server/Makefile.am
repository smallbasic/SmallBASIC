# SmallBASIC
# Copyright(C) 2001-2016 Chris Warren-Smith.
#
# This program is distributed under the terms of the GPL v2.0 or later
# Download the GNU Public License (GPL) from www.gnu.org
# 

SUBDIRS = src/common .

AM_CFLAGS = -I"${JAVA_HOME}/include" -I"${JAVA_HOME}/include/linux"
ACLOCAL_AMFLAGS= -I m4
lib_LTLIBRARIES = libsmallbasic.la
libsmallbasic_la_SOURCES = main.c
libsmallbasic_la_LIBADD = -L$(top_srcdir)/src/common -lsb_common
libsmallbasic_la_DEPENDENCIES = $(top_srcdir)/src/common/libsb_common.a

all-am:
	mvn package
