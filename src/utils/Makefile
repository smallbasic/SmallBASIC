#
#	Ready for the real script
#
cc=g++
scc=gcc
t_cx_flags=-x c++ -DSTRLWR
t_sc_flags=-DSTRLWR
c_output=-o
c_compile_only=-c
c_debug_flags=-g
c_nodebug_flags=-O2 -s
#c_libs=-static
c_libs=
sc_libs=
exe_ext=
obj_ext=.o
cmd_del=rm
cmd_echo=echo
cmd_cp=cp
cmd_install=install
bin_dir=../../bin

# unix
#cx_flags = $(t_cx_flags) -D_UnixOS
#sc_flags = $(t_sc_flags) -D_UnixOS
cx_flags = $(t_cx_flags)
sc_flags = $(t_sc_flags)

install_dir=/usr/local/bin

all: $(bin_dir)/d2u$(exe_ext) $(bin_dir)/u2d$(exe_ext) $(bin_dir)/mksmall$(exe_ext) $(bin_dir)/chext$(exe_ext)

clean:
	$(cmd_del) $(bin_dir)/d2u$(exe_ext) $(bin_dir)/u2d$(exe_ext) $(bin_dir)/mksmall$(exe_ext)

$(bin_dir)/d2u$(exe_ext): d2u.cpp
	$(cc) $(cx_flags) d2u.cpp $(c_output) $(bin_dir)/d2u$(exe_ext) $(c_libs)

$(bin_dir)/u2d$(exe_ext): u2d.cpp
	$(cc) $(cx_flags) u2d.cpp $(c_output) $(bin_dir)/u2d$(exe_ext) $(c_libs)

$(bin_dir)/mksmall$(exe_ext): mksmall.cpp
	$(cc) $(cx_flags) mksmall.cpp $(c_output) $(bin_dir)/mksmall$(exe_ext) $(c_libs)

$(bin_dir)/chext$(exe_ext): chext.c
	$(scc) $(sc_flags) chext.c $(c_output) $(bin_dir)/chext$(exe_ext) $(sc_libs)

install:
	$(cmd_install) $(bin_dir)/d2u $(install_dir)
	$(cmd_install) $(bin_dir)/u2d $(install_dir)
	$(cmd_install) $(bin_dir)/mksmall $(install_dir)
	$(cmd_install) $(bin_dir)/chext $(install_dir)

