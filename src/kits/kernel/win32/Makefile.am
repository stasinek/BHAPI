AUTOMAKE_OPTIONS = 1.7

DEFAULT_INCLUDES = -I$(top_builddir)

INCLUDES =	$(ETK_THREAD_CFLAGS) 			\
		$(ETK_EXTRA_CFLAGS)			\
		-DETK_COMPILATION 			\
		@ETK_DEBUG_FLAGS@

top_builddir_full=`cd \$(top_builddir); pwd`

noinst_LTLIBRARIES = libkernel-win32.la

libkernel_win32_la_SOURCES =			\
			etk-os.cpp		\
			etk-timefuncs.cpp	\
			etk-area.cpp		\
			etk-image.cpp

libkernel_win32_la_LIBADD =
libkernel_win32_la_DEPENDENCIES =

DISTCLEANFILES =	\
	Makefile.in

