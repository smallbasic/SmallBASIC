# SmallBASIC command line version
# Copyright(C) 2001-2012 Chris Warren-Smith.
#
# This program is distributed under the terms of the GPL v2.0 or later
# Download the GNU Public License (GPL) from www.gnu.org
# 

AM_CPPFLAGS = -I$(top_builddir)/src -I. @PACKAGE_CFLAGS@

EXTRA_DIST =																		\
    chr2c.c																			\
    el-928.c																		\
    lnx928.fnt																	\
    mgr928.fnt																	\
    mod928.fnt																	\
    mugr928.fnt																	\
    nar928-2.fnt																\
    nar928.fnt																	\
    ndc928.c																		\
    ndc928.fnt																	\
    rlgr928.fnt																	\
    rom16.c																			\
    inet.c

bin_PROGRAMS = sbasic

sbasic_SOURCES =																\
		dev_ndcfb.c																	\
	  dev_genfb.c																	\
		dev_oss.c																		\
		dev_term.c																	\
	  console_main.c															\
		../../common/device.c												\
    ../../common/g_line.c												\
	  ../../common/decomp.c

sbasic_LDADD = -L$(top_srcdir)/src/common -lsb_common @PACKAGE_LIBS@

if !WITH_WIN32
sbasic_LDADD += -ldl
endif

sbasic_DEPENDENCIES = $(top_srcdir)/src/common/libsb_common.a

TEST_DIR=../../../samples/distro-examples/tests
UNIT_TESTS=array break byref eval-test iifs matrices metaa ongoto uds

test: ${bin_PROGRAMS}
	@for utest in $(UNIT_TESTS); do                             \
    ${bin_PROGRAMS} ${TEST_DIR}/$${utest}.bas > test.out;     \
    if cmp -s test.out ${TEST_DIR}/output/$${utest}.out; then \
      echo $${utest} ✓;                                      \
    else                                                      \
      echo $${utest} ✘;                                       \
    fi ;                                                      \
  done;
