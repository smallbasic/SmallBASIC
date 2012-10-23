#!/usr/bin/ruby

#
#./workfile.rb CONFIG="debug" USE_NEWLIB=1 PACK="Android/Android 2.x"
#

require File.expand_path(ENV['MOSYNCDIR']+'/rules/mosync_exe.rb')

work = PipeExeWork.new
work.instance_eval do
	@SOURCES = ["src/platform/mosync"]
	@EXTRA_CPPFLAGS = " -Wno-shadow -Wno-missing-format-attribute -DHAVE_CONFIG_H"
	@EXTRA_INCLUDES = [".", "src", "src/platform/mosync/common", "#{mosyncdir}/include/newlib"]
	@LIBRARIES = ["mautil", "libsb_common.a"]
	@EXTRA_LINKFLAGS = " -datasize=2048576 -heapsize=786432 -stacksize=65536 "
	@NAME = "SmallBASIC"
end

work.invoke

