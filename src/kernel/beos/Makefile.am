AUTOMAKE_OPTIONS = 1.7

DEFAULT_INCLUDES = -I$(top_builddir)

INCLUDES =	$(ETK_THREAD_CFLAGS) 			\
		$(ETK_EXTRA_CFLAGS)			\
		-DETK_COMPILATION 			\
		-DNEED_MAIN				\
		@ETK_DEBUG_FLAGS@

top_builddir_full=`cd \$(top_builddir); pwd`

noinst_LTLIBRARIES = libkernel-beos.la

libkernel_beos_la_SOURCES =			\
			etk-os.cpp		\
			etk-timefuncs.cpp	\
			etk-area.cpp		\
			etk-image.cpp

libkernel_beos_la_LIBADD =
libkernel_beos_la_DEPENDENCIES =

DISTCLEANFILES =	\
	Makefile.in

