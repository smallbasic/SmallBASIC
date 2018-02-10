# SmallBASIC
# Copyright(C) 2001-2008 Chris Warren-Smith. [http://tinyurl.com/ja2ss]
#
# This program is distributed under the terms of the GPL v2.0 or later
# Download the GNU Public License (GPL) from www.gnu.org
# 

AM_CPPFLAGS = -I../../include
lib_LTLIBRARIES = libmysql.la
libmysql_la_SOURCES = mysql.c mod_utils.c
libmysql_la_LDFLAGS = -module -rpath '$(libdir)' -lmysqlclient
