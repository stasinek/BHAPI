AUTOMAKE_OPTIONS = 1.7

DEFAULT_INCLUDES = -I$(top_builddir)

INCLUDES =	$(ETK_THREAD_CFLAGS) 			\
		$(ETK_EXTRA_CFLAGS)			\
		-DETK_COMPILATION 			\
		@ETK_DEBUG_FLAGS@

top_builddir_full=`cd \$(top_builddir); pwd`

noinst_LTLIBRARIES = libkernel-unix.la

libkernel_unix_la_SOURCES =			\
			BHAPI_wrapper_os.cpp		\
			BHAPI_wrapper_timefuncs.cpp	\
			BHAPI_wrapper_area.cpp		\
			BHAPI_wrapper_image.cpp

libkernel_unix_la_LIBADD =
libkernel_unix_la_DEPENDENCIES =

DISTCLEANFILES =	\
	Makefile.in

